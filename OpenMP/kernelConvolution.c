#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include "cJSON.h"
#define MAXBUFLEN 1000000
#define DATA_OFFSET_OFFSET 0x000A
#define WIDTH_OFFSET 0x0012
#define HEIGHT_OFFSET 0x0016
#define BITS_PER_PIXEL_OFFSET 0x001C
#define HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define NO_COMPRESION 0
#define MAX_NUMBER_OF_COLORS 0
#define ALL_COLORS_REQUIRED 0

typedef unsigned int int32;
typedef short int16;
typedef unsigned char byte;

void WriteImage(const char *fileName, byte *pixels, int32 width, int32 height,int32 bytesPerPixel, int upsideDown);
void ReadImage(const char *fileName,byte **pixels, int32 *width, int32 *height, int32 *bytesPerPixel, int * upsideDown);
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
    // FILE *source, *dest;
    // source = fopen(fileInputLocation, "rb");
    // dest = fopen(fileOutputLocation, "wb+");
    byte **pixels;
    int32 width;
    int32 height;
    int32 bytesPerPixel;
    int upsideDown;
    printf("test %s\n",fileInputLocation);


    ReadImage(fileInputLocation, &pixels, &width, &height,&bytesPerPixel, &upsideDown);
    // if( source == NULL ) {

    //     return;
    // }

    // unsigned char byte[54],colorTable[1024];

    // for(int i=0;i<54;i++){
	// 	byte[i] = getc(source);
	// }
    // fwrite(byte,sizeof(unsigned char),54,dest);

    // int height = *(int*)&byte[18];
	// int width = *(int*)&byte[22];
    // int bitDepth = *(int*)&byte[28];

    // int row_padded = (width*3 + 3) & (~3);

    // if(bitDepth <= 8)										//if ColorTable present, extract it.
	// {
	// 	fread(colorTable,sizeof(unsigned char),1024,source);
	// 	fwrite(colorTable,sizeof(unsigned char),1024,dest);
	// }

    // int size = height*width;

    // unsigned long int (*buff)[3] = malloc(sizeof(unsigned long int[size][3]));
	// unsigned long int (*out)[3] = malloc(sizeof(unsigned long int[size][3]));

    // for(int i=0;i<size;i++){
    //     buff[i][2]=getc(source);
	// 	buff[i][1]=getc(source);
	// 	buff[i][0]=getc(source);
	// }

    // for(int x=1;x<height-1;x++){
	// 	for(int y=1;y<width-1;y++){
	// 		float sum0= 0.0;
	// 		float sum1= 0.0;
	// 		float sum2= 0.0;
	// 		for(int i=-1;i<=1;++i){
	// 			for(int j=-1;j<=1;++j){
	// 				sum0=sum0+(float)kernel[i+1][j+1]*pixels[(x+i)*width+(y+j)][0];
	// 				sum1=sum1+(float)kernel[i+1][j+1]*pixels[(x+i)*width+(y+j)][1];
	// 				sum2=sum2+(float)kernel[i+1][j+1]*pixels[(x+i)*width+(y+j)][2];
	// 			}
	// 		}
	// 		pixels[(x)*width+(y)][0]=sum0;
	// 		pixels[(x)*width+(y)][1]=sum1;
	// 		pixels[(x)*width+(y)][2]=sum2;
	// 	}
	// }
    WriteImage(fileOutputLocation, pixels, width, height, bytesPerPixel, upsideDown);
    // for(int i=0;i<size;i++){
	// 	putc(out[i][2],dest);
	// 	putc(out[i][1],dest);
	// 	putc(out[i][0],dest);

	// }
    // fclose(source);
    // fclose(dest);
    // free(buff);
    free(pixels);
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

void ReadImage(const char *fileName,byte **pixels, int32 *width, int32 *height, int32 *bytesPerPixel, int * upsideDown)
{
        FILE *imageFile = fopen(fileName, "rb");
        int32 dataOffset;
        fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
        fread(&dataOffset, 4, 1, imageFile);
        fseek(imageFile, WIDTH_OFFSET, SEEK_SET);
        fread(width, 4, 1, imageFile);
        fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
        fread(height, 4, 1, imageFile);

        if( (int)*height < 0){
            printf("lines upside down \n");
            (*height) *=-1;
            *upsideDown = 1;
        }
        else{
            *upsideDown = 0;
        }
        int16 bitsPerPixel;
        fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
        fread(&bitsPerPixel, 2, 1, imageFile);
        *bytesPerPixel = ((int32)bitsPerPixel) / 8;

        int paddedRowSize = (int)(4 * ceil((float)(*width) / 4.0f))*(*bytesPerPixel);
        int unpaddedRowSize = (*width)*(*bytesPerPixel);
        int totalSize = unpaddedRowSize*(*height);
        printf("size: %d \n",totalSize);
        printf("Width: %d, Height: %d , ::short %lu \n",*width,*height, sizeof(unsigned short));

        *pixels = (byte*)malloc(totalSize);
        int i = 0;
        byte *currentRowPointer = *pixels+((*height-1)*unpaddedRowSize);
        for (i = 0; i < *height; i++)
        {
            fseek(imageFile, dataOffset+(i*paddedRowSize), SEEK_SET);
            fread(currentRowPointer, 1, unpaddedRowSize, imageFile);
            currentRowPointer -= unpaddedRowSize;
        }

        fclose(imageFile);
}

void WriteImage(const char *fileName, byte *pixels, int32 width, int32 height,int32 bytesPerPixel, int upsideDown)
{
        FILE *outputFile = fopen(fileName, "wb");
        //*****HEADER************//
        const char *BM = "BM";
        fwrite(&BM[0], 1, 1, outputFile);
        fwrite(&BM[1], 1, 1, outputFile);
        int paddedRowSize = (int)(4 * ceil((float)width/4.0f))*bytesPerPixel;
        int32 fileSize = paddedRowSize*height + HEADER_SIZE + INFO_HEADER_SIZE;
        fwrite(&fileSize, 4, 1, outputFile);
        int32 reserved = 0x0000;
        fwrite(&reserved, 4, 1, outputFile);
        int32 dataOffset = HEADER_SIZE+INFO_HEADER_SIZE;
        fwrite(&dataOffset, 4, 1, outputFile);

        //*******INFO*HEADER******//
        int32 infoHeaderSize = INFO_HEADER_SIZE;
        fwrite(&infoHeaderSize, 4, 1, outputFile);
        fwrite(&width, 4, 1, outputFile);
        if(upsideDown){
            int newHeight = height * -1;
            fwrite(&newHeight, 4, 1, outputFile);
        }
        else{
            fwrite(&height, 4, 1, outputFile);
        }

        int16 planes = 1; //always 1
        fwrite(&planes, 2, 1, outputFile);
        int16 bitsPerPixel = bytesPerPixel * 8;
        fwrite(&bitsPerPixel, 2, 1, outputFile);
        //write compression
        int32 compression = NO_COMPRESION;
        fwrite(&compression, 4, 1, outputFile);
        // write image size (in bytes)
        int32 imageSize = width*height*bytesPerPixel;
        fwrite(&imageSize, 4, 1, outputFile);
        int32 resolutionX = 11811; //300 dpi
        int32 resolutionY = 11811; //300 dpi
        fwrite(&resolutionX, 4, 1, outputFile);
        fwrite(&resolutionY, 4, 1, outputFile);
        int32 colorsUsed = MAX_NUMBER_OF_COLORS;
        fwrite(&colorsUsed, 4, 1, outputFile);
        int32 importantColors = ALL_COLORS_REQUIRED;
        fwrite(&importantColors, 4, 1, outputFile);
        int i = 0;
        int unpaddedRowSize = width*bytesPerPixel;
        for ( i = 0; i < height; i++)
        {
                int pixelOffset = ((height - i) - 1)*unpaddedRowSize;
                fwrite(&pixels[pixelOffset], 1, paddedRowSize, outputFile);
        }
        fclose(outputFile);
}

// int main()
// {
//         byte *pixels;
//         int32 width;
//         int32 height;
//         int32 bytesPerPixel;
//         ReadImage("img.bmp", &pixels, &width, &height,&bytesPerPixel);
//         WriteImage("img2.bmp", pixels, width, height, bytesPerPixel);
//         free(pixels);
//         return 0;
// }
