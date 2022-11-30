#include <stdio.h>

int main(){
    FILE *fptr;
    ptr = fopen("Archive/f1.txt","w+");
    if(fptr == NULL)
          {
            printf("Error!");   
            exit(1);             
          }
        fprintf(fptr,"%s",num);
        fclose(fptr);
        return 0;
    }