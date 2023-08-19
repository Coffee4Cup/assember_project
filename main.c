#include <stdio.h>
#include "assmbler.h"
#include "macro.h"


#define MAX_IMAGE_SIZE 80
#define MAX_LABEL_SIZE 80


int main(int argc, char *argv[]) {


    FILE *output_file, *input_file;
    char *output_path, *input_path;
    FILE *preprocessed_files[argc - 1];/*an array of all the preprocessed files*/
    int count = 0;



    if(argc == 1)
        printf("no files to paras have been given\n");

    while(--argc > 0) {

        printf("%s\n", *++argv);

        output_path = strdup(*argv);
        input_path = strdup(*argv);
        strcat(input_path, ".as");
        strcat(output_path, ".am");

        if ((input_file = fopen(input_path, "r")) == NULL)
            printf("can't open the file %s\n", input_path);

        else if ((output_file = fopen(output_path, "w+")) == NULL)
            printf("can't create the file named %s\n", output_path);

        else if((output_file = preprocess(output_file, input_file)) == NULL)
            printf("failed to map macros from text in file %s, moving to next file\n", output_path);

        else{
            printf("successfully map macros from text in file %s,\n", input_path);
            fseek(output_file, 0L, 0);
            assemble(output_file);

            fclose(output_file);
            fclose(input_file);
        }

    }


    return 0;
}