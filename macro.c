//
// Created by itay8 on 30/07/2023.
//

#include <string.h>
#include "macro.h"
#include "assmbler.h"
#define MACRO_TEXT_MAX_LEN 128 /*@TODO check this is the right number*/
static struct nlist *macro_table[TAB_SIZE];

static int valid_file;

FILE *preprocess(FILE *output_file, FILE *input_file) {

    valid_file = TRUE;/*a flag for the validity of the file*/

    static enum flag {
        IN, OUT
    };
    static int state = OUT;
    static char *token, *macro_name, macro_replacement_text[MACRO_TEXT_MAX_LEN];
    static char input_line[MAX_LINE_LEN];
    int i;
    for(i = 0; i < TAB_SIZE;i++)/*rest the macro table from the previous file*/
        macro_table[i] = NULL;

    while (fgets(input_line, MAX_LINE_LEN, input_file)) {

        token = strtok(input_line, " \t\n");/*reads the first token in line*/

        if(token == NULL)
        /*empty line does nothing*/
            printf("");
        else if(state == OUT) {/*not reading macro text replacement*/

            if (!strcmp(token, MACRO_KEYWORD)) {
                state = IN;/*inside macro decleration*/
                read_macro_name(&macro_name);/*reads the rest of the line to get the name of the macro, checks if the number of words is valid (check for validity of the macro itself is later while installing the macro in the lookup table*/
            }/*the first word is a macro decleration*/
            else {
                read_non_macro_line(output_file, token);
            }/*the first token is not a macro decleration, reads the line as a regular line from file */

        } else {
            if (strcmp(token, MACRO_END_KEYWORD) != 0) {
                read_macro_text(token, macro_replacement_text);
            }

            else {/*Keyword for the end of macro initialization. */
            state = OUT;
            get_macro(macro_replacement_text,token, &macro_name);
            }
        }/*state == IN, reading the text of a macro*/
    }

    if (valid_file == TRUE)
        return output_file;
    else
        return NULL;
}

void read_macro_text(char *token, char *macro_replacement_text) {
    do {
        if (!strcmp(token, MACRO_END_KEYWORD)) {
            printf("can't declare a end of macro mid-line.\n");
            valid_file = FALSE;
        }/*there is an end mcro decleration mid-line from file (not valid: https://opal.openu.ac.il/mod/ouilforum/discuss.php?d=3139882&p=7450222#p7450222)*/

        else/*another token for the macro text replacement*/
            strcat(macro_replacement_text, token);
        strcat(macro_replacement_text, " ");/*replace the last space as newline char later */
    } while ((token = strtok(NULL, " \t\n")) != NULL);/*reads token while there is another word in line*/

    *strrchr(macro_replacement_text,' ') = '\n';/*replace the last occurrence of space with newline char, end of line*/
}


void read_non_macro_line(FILE *output_file, char *token) {

    static struct nlist *macro_ptr = NULL;
    int last_token_macro = FALSE; /*Flag for knowing when to append to the output a newline (the macros have one so should put another after a macro)*/
    while (token != NULL) {
        if (!strcmp(token, MACRO_KEYWORD)) {
            printf("Error: Cannot declare a macro mid-line.\n");
            valid_file = FALSE;

        } else if ((macro_ptr = lookup_macro(token)) != NULL) {
            /*Found a saved macro (assumes validly)*/
            fputs((char*) macro_ptr->data, output_file);/* Place the macro text in the output file*/
            last_token_macro = TRUE;
        } else {
            /*Just another non-macro-related token*/
            fputs(token, output_file);
            fputc(' ', output_file);
            last_token_macro = FALSE;
        }
        token = strtok(NULL, " \t\n");/*Read the next token*/
    }
    if(!last_token_macro)
        fputc('\n',output_file);
}
/*checks the syntax of the given macro If the syntax is valid then install the macro in the macro table */
void get_macro(char *macro_replacement_text, char *token, char **macro_name) {

    while ((token = strtok(NULL, " \t\n")) != NULL) {
        printf("can't have more text at line with end macro definition.\n");
        valid_file = FALSE;
    }/*there is one token or more after the end of the macro is declared (not valid: https://opal.openu.ac.il/mod/ouilforum/discuss.php?d=3139882&p=7450222#p7450222))*/
    if (install_macro((*macro_name), macro_replacement_text) == NULL) {
        printf("the name of the macro: \"%s\" is not valid .\n", *macro_name);
        valid_file = FALSE;
    }/*installing the macro inside a lookup table, and checking the name is valid for a macro*/
    macro_replacement_text[0] = '\0';/*reset the macro name and text buffers*/
    macro_name = NULL;
}

void read_macro_name(char **macro_name) {

    char *token;

    *macro_name = strdup(strtok(NULL, " \n\t"));/*reads the name of the macro*/

    if (*macro_name == NULL)/*can't declare a macro without a name in same line of decleration (not valid: https://opal.openu.ac.il/mod/ouilforum/discuss.php?d=3139882&p=7450222#p7450222) */
    {
        printf("can't declare a macro without a name in the same line of decleration.\n");
        valid_file = FALSE;
    }
    while ((token = strtok(NULL, " \n\t")) != NULL) {
        printf("can't have a mcro name with spaces/write macro text inside the decleration line.\n");
        valid_file = FALSE;
    }/*there is one token or more after the name of the declared macro (not valid: https://opal.openu.ac.il/mod/ouilforum/discuss.php?d=3139882&p=7450222#p7450222))*/
}

struct nlist *lookup_macro(char *name){
    return lookup(macro_table, name, TAB_SIZE);
}

struct nlist *install_macro(char *name, char* replacement_text){

    if(get_command_code(name) != NOT_OPCODE)
        printf("the macro name is not valid \n");

    return defult_install(macro_table, name, replacement_text, TAB_SIZE);

}
