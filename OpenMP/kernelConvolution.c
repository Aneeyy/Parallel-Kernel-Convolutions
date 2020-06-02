#include <stdio.h>

#include <stdlib.h>
#include <sys/syscall.h>
#include "cJSON.h"
#define MAXBUFLEN 1000000

void copyFile(char* s, char* d){
    FILE *source, *dest;
    int i;
    source = fopen(s, "rb");

    if( source == NULL ) {
        return;
    } //exit(EXIT_FAILURE);

    fseek(source, 0, SEEK_END);
    int length = ftell(source);

    fseek(source, 0, SEEK_SET);

    dest = fopen(d, "wb");

    if( dest == NULL ) {
        fclose(source);
        return;
    }

    for(i = 0; i < length; i++){
        fputc(fgetc(source), dest);
    }

    printf("File copied successfully.\n");
    fclose(source);
    fclose(dest);

}
cJSON* getConfig(){
    char source[MAXBUFLEN + 1];
    FILE *fp = fopen("../transferData/config.json", "r");
    if (fp == NULL) {
        return NULL;
    }
    size_t newLen = fread(source, sizeof(char), MAXBUFLEN, fp);
    if ( ferror( fp ) != 0 ) {
        fputs("Error reading file", stderr);
    } else {
        source[newLen++] = '\0';
    }
    fclose(fp);
    return cJSON_Parse(source);



}
void writeToTimingJSON(double timing, char* fileOutputLocation){
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "timing", timing);
    cJSON_AddStringToObject(obj, "fileOutputLocation", fileOutputLocation);

    FILE *fp = fopen("../transferData/OMPTiming.json", "w");
    if (fp != NULL){
        fputs(cJSON_Print(obj), fp);
        fclose(fp);
    }
}

double ** getKernel(cJSON* config, int *kernelSize){
    cJSON* kernelJSON = cJSON_GetObjectItem(config, "kernel");
    int ks = cJSON_GetArraySize(kernelJSON);
    *kernelSize = ks;



    double **kernel = (double **)malloc(ks * sizeof(double *));
    for (int r = 0; r < ks; r++){
        kernel[r] = (double *)malloc(ks * sizeof(double));
         cJSON* arrayRow = cJSON_GetArrayItem(kernelJSON, r);
        for(int c = 0; c < ks; c++){
           cJSON* arrayColItem = cJSON_GetArrayItem(arrayRow, c);
           kernel[r][c] = cJSON_GetNumberValue(arrayColItem);

        }


    }

    // Blurring kernel testing
    for (int i = 0; i < ks; i++){
        for (int j = 0; j < ks; j++){
            kernel[i][j] = 1.0/9.0;
        }
    }

    // kernel[0][0] = 0.0;
    // kernel[ks-1][ks-1] = 1.0;

    return kernel;


}

void performConv(char* fileInputLocation,char* fileOutputLocation,double** kernel,int kernelSize){
    FILE *source, *dest;
    source = fopen(fileInputLocation, "r");
    dest = fopen(fileOutputLocation, "w+");

    if( source == NULL ) {

        return;
    } 

    unsigned char byte[54];

    for(int i=0;i<54;i++){									
		byte[i] = getc(source);								
	}
    fwrite(byte,sizeof(unsigned char),54,dest);	

    int height = *(int*)&byte[18];
	int width = *(int*)&byte[22];

    int size = height*width;

    unsigned char (*buff)[3] = malloc(sizeof(unsigned char[size][3]));
	unsigned char (*out)[3] = malloc(sizeof(unsigned char[size][3]));

    for(int i=0;i<size;i++){
        buff[i][2]=getc(source);					
		buff[i][1]=getc(source);
		buff[i][0]=getc(source);
	}
    for(int x=1;x<height-1;x++){					
		for(int y=1;y<width-1;y++){
			float sum0= 0.0;
			float sum1= 0.0;
			float sum2= 0.0;
			for(int i=-1;i<=1;++i){
				for(int j=-1;j<=1;++j){	
					sum0=sum0+(float)kernel[i+1][j+1]*buff[(x+i)*width+(y+j)][0];
					sum1=sum1+(float)kernel[i+1][j+1]*buff[(x+i)*width+(y+j)][1];
					sum2=sum2+(float)kernel[i+1][j+1]*buff[(x+i)*width+(y+j)][2];
				}
			}
			out[(x)*width+(y)][0]=sum0;
			out[(x)*width+(y)][1]=sum1;
			out[(x)*width+(y)][2]=sum2;
		}
	}

    for(int i=0;i<size;i++){
		putc(out[i][2],dest);
		putc(out[i][1],dest);
		putc(out[i][0],dest);

	}
    fclose(source);
    fclose(dest);
    free(buff);
    free(out);
}

int main() {


    cJSON *configjson = getConfig();

    cJSON *fileInputJSON = cJSON_GetObjectItem(configjson, "fileInputLocation");
    cJSON *fileOutputJSON = cJSON_GetObjectItem(configjson, "openMPOutputLocation");

    char* fileInputLocation = fileInputJSON->valuestring;
    char* fileOutputLocation = fileOutputJSON->valuestring;

    int kernelSize = -1;
    int numThreads = (int)cJSON_GetNumberValue( cJSON_GetObjectItem(configjson,"numThreads"));
    double ** kernel = getKernel(configjson, &kernelSize);

    //print the kernel
    printf("kernel size: %d , num threads: %d\n",kernelSize,numThreads);
    for(int r = 0; r< kernelSize; r++){
        for(int c = 0; c< kernelSize; c++){
            printf("%f \t",kernel[r][c]);
        }
        printf("\n");
    }


    printf("Copying %s to %s \n",fileInputLocation,fileOutputLocation);


    //This copy file function is in place of the filter that has to be written
    //In the implementation, read image from the input file, and write image to output file
    //then replace the 100.02 with the seconds it took to perform the kernel convolution
    // copyFile(fileInputLocation,fileOutputLocation);
    

    performConv(fileInputLocation,fileOutputLocation,kernel,kernelSize);

//    double[][] kernel
//    double kSize = 3;//5

    //replace the .02 with the actual timing(its a double (seconds))
    writeToTimingJSON(100.02, fileOutputLocation);
    return 0;
}


