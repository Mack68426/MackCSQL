#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/random.h>

#define MAX_SIZE 10000
#define NumberOfData 10

int randomInt(int );
unsigned int randomUInt(int);

int GenerateInt(void *data, int isNegative){
    int num;
    if (isNegative>0)
    {
        getrandom(&num, sizeof(signed int),1)==-1?perror("getrandom"):"";
        
        data = &num;

        return 1;
    }
    else
    {
        unsigned int unum = (unsigned int)num;

        data = &num;

        return 0;
    }

    return -1;

}

int main(){
    
    int data;
    for(int i=1; i<=NumberOfData; i++)
    {
        int dtype = GenerateInt(&data,1);
        char *out = calloc(20,sizeof(char));

        sprintf(out, dtype?"%u":"%d",dtype?(unsigned int)data:data);
        puts(out);
    }

    return 0;
}
