#include<stdio.h>


int main(){
    char *string_arr[200]= {\
        "Hello",
        "world, "
        "darling"};

    printf("%s\n", string_arr[0]);
    printf("%s\n", *string_arr[0]);
    printf("%s\n", &string_arr[0]);

}