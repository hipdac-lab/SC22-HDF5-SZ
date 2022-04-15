#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "sz.h"
#include "rw.h"

struct timeval startTime;
struct timeval endTime;  /* Start and end times */
struct timeval costStart; /*only used for recording the cost*/
double totalCost = 0;


void cost_start()
{
	totalCost = 0;
        gettimeofday(&costStart, NULL);
}

void cost_end()
{
        double elapsed;
        struct timeval costEnd;
        gettimeofday(&costEnd, NULL);
        elapsed = ((costEnd.tv_sec*1000000+costEnd.tv_usec)-(costStart.tv_sec*1000000+costStart.tv_usec))/1000000.0;
        totalCost += elapsed;
}


int main(int argc, char * argv[])
{
    size_t r5=0,r4=0,r3=0,r2=0,r1=0;
    char oriFilePath[640], outputFilePath[650];
    char *cfgFile;
   
    cfgFile="sz.config";
    sprintf(oriFilePath, "%s", "temperature.dat");
		r1 = 512
		r2 = 512
		r3 = 512
    int status = SZ_Init(cfgFile);
   
    size_t nbEle;
    float *data = readFloatData(oriFilePath, &nbEle, &status);
   
    size_t outSize; 
    //char *bytes = (char *)malloc(nbEle*sizeof(float)); //
    //SZ_compress_args2(SZ_FLOAT, data, bytes, &outSize, ABS, 0.0001, 0.0001, r5, r4, r3, r2, r1);    
    //char *bytes = SZ_compress_rev(SZ_FLOAT, data, revValue, &outSize, r5, r4, r3, r2, r1);
    cost_start();
    unsigned char *bytes = SZ_compress(SZ_FLOAT, data, &outSize, r5, r4, r3, r2, r1);
    cost_end();
    printf("timecost=%f\n",totalCost); 
    writeByteData(bytes, outSize, outputFilePath, &status);
    if(status != SZ_SCES)
    {
        printf("Error: data file %s cannot be written!\n", outputFilePath);
        exit(0);
    }

    printf("done\n");
    free(bytes); 
    free(data);
    SZ_Finalize();
    
    return 0;
}
