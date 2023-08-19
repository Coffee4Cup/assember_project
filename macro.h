
#ifndef MMN14_COPY_MACRO_H
#define MMN14_COPY_MACRO_H

#include <stdio.h>
#include "lookup_table.h"

#define MACRO_MAX_LEN 100
#define NAMELEN 100
#define MAX_PATH_LEN 256

#define MACRO_KEYWORD "mcro"
#define MACRO_END_KEYWORD "endmcro"


/**
 *paras a line of code for finding macro usage and decleration
*/

FILE *preprocess(FILE *output_file, FILE *input_file);

/**
 * use implementation of lookup table
 */
struct nlist *install_macro(char *name, char *replacement_text);
struct nlist *lookup_macro(char *name);
void read_macro_name(char **macro_name);
void get_macro(char *macro_replacement_text, char *token, char **macro_name);
void read_non_macro_line(FILE *output_file, char *token);
void read_macro_text(char *token, char *macro_replacement_text);


#endif //MMN14_COPY_MACRO_H