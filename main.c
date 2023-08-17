#include <stdio.h>
#include "assmbler.h"
int main(int argc,char *argv[]) {

    /*char* s= malloc(20), *tok;*/
    FILE *input_file;
    input_file = fopen(argv[1], "r");

    /*fgets(s, 20, input_file);*/
    /*tok = strtok(s, "\n");*/
   /* printf("%s",tok);*/
    assemble(input_file);
    return 0;
}