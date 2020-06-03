#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

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


struct timeval timer_start(){
	struct timeval start_time;
	gettimeofday(&start_time,NULL);
	return start_time;
}

// get microseconds
long timer_end(struct timeval start_time){
	struct timeval end_time;
	gettimeofday(&end_time,NULL);
	long diffInMicros = (end_time.tv_sec - start_time.tv_sec) * (long)1e6 + (end_time.tv_usec - start_time.tv_usec);
	return diffInMicros;
}


void ReadImage(const char *fileName,byte **pixels, byte ** pixelsOut, int32 *width, int32 *height, int32 *bytesPerPixel, int * upsideDown, int* unpaddedRowSizeCopy)
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

        //int paddedRowSize = (int)(4 * ceil((float)(*width) / 4.0f))*(*bytesPerPixel);
        int paddedRowSize = (*width*3 + 3) & (~3);


        int unpaddedRowSize = (*width)*(*bytesPerPixel);
        *unpaddedRowSizeCopy = unpaddedRowSize;
        int totalSize = unpaddedRowSize*(*height);
        printf("size: %d, unpaddedrowsize: %d paddedrowsize: %d \n",totalSize,unpaddedRowSize,paddedRowSize);
        printf("Width: %d, Height: %d , ::short %lu \n",*width,*height, sizeof(unsigned short));

        *pixels = (byte*)malloc(totalSize);
        *pixelsOut = (byte*)malloc(totalSize);

        unsigned int i = 0;
        byte *currentRowPointer = *pixels+((*height-1)*unpaddedRowSize);
        for (i = 0; i < *height; i++) {
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
        //int paddedRowSize = (int)(4 * ceil((float)width/4.0f))*bytesPerPixel;
        int paddedRowSize = (width*3 + 3) & (~3);


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
        unsigned int i = 0;
        int unpaddedRowSize = width*bytesPerPixel;
        for ( i = 0; i < height; i++){
                int pixelOffset = ((height - i) - 1)*unpaddedRowSize;
                fwrite(&pixels[pixelOffset], 1, paddedRowSize, outputFile);
        }
        fclose(outputFile);
}



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
//
//    // Blurring kernel testing
//    for (int i = 0; i < ks; i++){
//        for (int j = 0; j < ks; j++){
//            kernel[i][j] = 1.0/9.0;
//        }
//    }

    // kernel[0][0] = 0.0;
    // kernel[ks-1][ks-1] = 1.0;

    return kernel;


}

void performConv(char* fileInputLocation,char* fileOutputLocation,int nt, double **kernel, int kernelSize, double *timingData, int shouldSave, int makeGreyScale){
    // FILE *source, *dest;
    // source = fopen(fileInputLocation, "rb");
    // dest = fopen(fileOutputLocation, "wb+");
    byte *pixels;
    byte *pixelsOut;
    int32 width;
    int32 height;
    int32 bytesPerPixel;
    int unPaddedRowSize;
    int upsideDown;



    //print the kernel
    printf("kernel size: %d , num threads:\n",kernelSize);
    for(int r = 0; r< kernelSize; r++){
        for(int c = 0; c< kernelSize; c++){
            printf("%f \t",kernel[r][c]);
        }
        printf("\n");
    }






    ReadImage(fileInputLocation, &pixels,&pixelsOut, &width, &height,&bytesPerPixel, &upsideDown, &unPaddedRowSize);
    printf("unpadded row size %d\n",unPaddedRowSize);


    int pixStart = 0, rowStart=0, kPixStart=0, kRowStart;

    int rowSize = width * 3;
    float sum0 = 0, sum1 = 0, sum2 = 0;
    int kStart, kEnd;
    kStart = kernelSize==3 ? -1 : -2;
    kEnd = kernelSize==3 ? 1 : 2;


    //preprocessing step, leave out of timing
    if(makeGreyScale){
        for(unsigned int row=0; row<height; row++){
            rowStart = rowSize * row;
            for(unsigned int col=0;col<width;col++){
                pixStart = rowStart + col* 3;

                sum0 = pixels[pixStart + 0];
                sum0 += pixels[pixStart + 1];
                sum0 += pixels[pixStart + 2];
                sum0 /= 3;
                pixels[pixStart + 0] = sum0;
                pixels[pixStart + 1] = sum0;
                pixels[pixStart + 2] = sum0;
            }
		}



    }




    struct timeval beforeTime = timer_start();



//    int pixelSize = 3;

//    for(int p = 0; p < unPaddedRowSize*height; p++){
//        int rowPos = (p % unPaddedRowSize) / 3;
//
//        int nRow = p / unPaddedRowSize;
//        int nCol = p - nRow*unPaddedRowSize;
//
//        if(nRow % 2 == 0){
//            pixels[p] = 255;
//        }
//        else{
//            pixels[p] = 0;
//            //pixelsOut[p] = pixels[p];
//        }
//    }

    for(unsigned int row=0; row<height; row++){
        rowStart = rowSize * row;

//        printf("row: %d,height: %d, rowStart: %d \n",row,height, rowStart);
		for(unsigned int col=0;col<width;col++){
		    pixStart = rowStart + col* 3;

		    if( row== 0 || col == 0 || row== height-1 || col== width-1){

                pixelsOut[pixStart + 0] = pixels[pixStart + 0];
                pixelsOut[pixStart + 1] = pixels[pixStart + 1];
                pixelsOut[pixStart + 2] = pixels[pixStart + 2];



		    }
            else if(kernelSize == 5 && (row== 1 || col == 1 || row== height-2 || col== width-2)){
                pixelsOut[pixStart + 0] = pixels[pixStart + 0];
		        pixelsOut[pixStart + 1] = pixels[pixStart + 1];
		        pixelsOut[pixStart + 2] = pixels[pixStart + 2];
            }
		    else{
		        sum0= 0.0;
                sum1= 0.0;
                sum2= 0.0;
                for(int i=kStart;i<=kEnd;i++){
                    kRowStart = rowStart + rowSize*i;
                    for(int j=kStart;j<=kEnd;j++){

                        kPixStart = kRowStart + col*3 + j*3;
                        sum0+=(float)kernel[i+kEnd][j+kEnd]*pixels[kPixStart + 0];
                        sum1+=(float)kernel[i+kEnd][j+kEnd]*pixels[kPixStart + 1];
                        sum2+=(float)kernel[i+kEnd][j+kEnd]*pixels[kPixStart + 2];
                    }
                }
                pixelsOut[pixStart + 0] = sum0;
		        pixelsOut[pixStart + 1] = sum1;
		        pixelsOut[pixStart + 2] = sum2;
		    }

		}
	}
    long micros = timer_end(beforeTime);

    *timingData =(double)micros / 1.e6;
    printf("done! \n");

    printf("should save: %d \n",shouldSave);
    if(shouldSave){
        WriteImage(fileOutputLocation, pixelsOut, width, height, bytesPerPixel, upsideDown);
    }


    // for(int i=0;i<size;i++){
	// 	putc(out[i][2],dest);
	// 	putc(out[i][1],dest);
	// 	putc(out[i][0],dest);

	// }
    // fclose(source);
    // fclose(dest);
    // free(buff);
    free(pixels);
    free(pixelsOut);
}

double ** getDummyKernel(int size){
    if(size != 3 && size != 5){
        return NULL;
    }
    double **kernel = (double **)malloc(size * sizeof(double *));
    for (int r = 0; r < size; r++){
        kernel[r] = (double *)malloc(size * sizeof(double));
        for(int c = 0; c < size; c++){
           kernel[r][c] = 1.;
        }
    }
    return kernel;
}

int main(int argc, char * argv[]){
    char* fileInputLocation;
    char* fileOutputLocation;
    int numThreads;
    int kernelSize;
    double ** kernel;

    int shouldSave = 0, shouldWriteToTiming = 0, makeGreyScale = 0;

    if(argc > 1){
        //usage: ./openMP fileInputLocation numThreads fileOutputLocation [3|5]
        if( argc < 5){
            fprintf(stderr, "Usage: ./openMP fileInputLocation [fileOutputLocation|-nosave] numThreads [3|5]");
            return 1;
        }
        else{
            fileInputLocation = argv[1];
            fileOutputLocation = argv[2];
            shouldSave = strncmp(argv[2],"-nosave",7);
            printf("shouldSave:  %d\n",shouldSave);

            numThreads = atoi(argv[3]);

            kernelSize = atoi(argv[4]);

            kernel = getDummyKernel(kernelSize);

            shouldWriteToTiming = 0;

        }




    }
    else{
        //load data from config file
        shouldSave = 1;
        shouldWriteToTiming = 1;
        cJSON *configjson = getConfig();

        cJSON *fileInputJSON = cJSON_GetObjectItem(configjson, "fileInputLocation");
        cJSON *fileOutputJSON = cJSON_GetObjectItem(configjson, "openMPOutputLocation");
        cJSON *greyScaleBoolJSON = cJSON_GetObjectItem(configjson,"greyScale");

        fileInputLocation = fileInputJSON->valuestring;
        fileOutputLocation = fileOutputJSON->valuestring;

        numThreads = (int)cJSON_GetNumberValue( cJSON_GetObjectItem(configjson,"numThreads"));

        makeGreyScale = cJSON_IsTrue(greyScaleBoolJSON);
//        makeGreyScale = strncmp(greyScaleBoolJSON->valuestring,"true",4);
        printf("grey scale:  %d\n",makeGreyScale);
        kernel = getKernel(configjson, &kernelSize);

    }

    if(kernelSize !=3 && kernelSize != 5){
        printf("%s\n","Bad Kernel Size, must be either 3 or 5, exiting");
        return 1;
    }


    if(numThreads < 1){
        numThreads = 1;
        printf("%s\n","Lower limit of num threads is 1, using 1 as num threads value");
    }
    if(numThreads > 64){
        numThreads = 64;
        printf("%s\n","Upper limit of num threads is 64, using 64 as num threads value");
    }


    double timingData;

    performConv(fileInputLocation,fileOutputLocation, numThreads, kernel, kernelSize, &timingData, shouldSave, makeGreyScale);

    if(shouldWriteToTiming){
        writeToTimingJSON(timingData, fileOutputLocation);
    }
    else{
        printf("Time: %f", timingData);
    }


    return 0;
}
