//
// Created by itay8 on 17/08/2023.
//

#include "assmbler.h"
#include "assmbler.h"  // You should include your own header file here
#define SKIP_WHITE_SPACE(A) while((A) != NULL && *(A) != '\0' && (*(A) == ' ' || *(A) == '\t')) (A)++;
/*copy a word and changes the last char to be end of line char, for the cases when <word><,>, this copy only <word>*/
#define PUT_WORD(A,B) while((B) != NULL && *(B) != '\0' &&  *(B) != '\t' && *(B) != ' ' && *(B) != ',' ) \
       *(A)++ = *(B)++;                                                                                           \
*(A) ='\0';

/* Add constant definitions for your opcode and data types */

/* Add your struct definitions here */

static int ic = 0;
static int dc = 0;
static struct nlist *symbol_table[TAB_SIZE];
static word word_image[TAB_SIZE];
static instruction_word instruction_image[TAB_SIZE];
static int valid_file;
static int line_count;
static FILE *input_file;

void assemble(FILE *file) {
    input_file = file;
    first_pass();
    /*second_pass();*/
}

int first_pass() {
    valid_file = 1;  // Initialize valid_file to TRUE
    line_count = 1;  // Initialize line_count

    int data_type, command_type;
    char *token, line[MAX_LINE_LEN];

    symbol temp_symbol; /*temporary symbol struct that holds values for a symbol entry later*/
    int is_symbol;      /*flag for defining symbol state ==OUT/ IN*/

    while (fgets(line, sizeof(line), input_file)) {
        token = strtok(line, " \t\n");

        if (is_label_decleration(token)) {
            /* If the line starts with a label */
            temp_symbol.label = get_label(token);
            is_symbol = TRUE;

            token = strtok(NULL, " \t\n");
        }

        if ((data_type = get_data_type(token)) != NOT_DATA_TYPE) {

            temp_symbol.symbol_type = DATA_TYPE;
            temp_symbol.memory_address = dc;

            token = strtok(NULL, " \t\n"); /* Takes the rest of the line as a parameter for the data type */

            switch (data_type) {
                case (DATA):
                    get_data(token);
                    break;
                case (STRING):
                    get_string(token);
                    break;
                case (ENTRY):
                    break; /* @todo add here rest of the cases for entry and extern */
            }
        } else if ((command_type = get_commend(token)) != NOT_OPCODE) {/* If the token is a command */

            temp_symbol.symbol_type = COMMEND_TYPE;
            temp_symbol.memory_address = ic;

            token = strtok(NULL, " \t\n"); /* Takes the rest of the line as a parameter for the data type */
            get_command_parameters(command_type, token);
        }

        if (is_symbol == TRUE) {
            /* Found all the information for making symbol entry */


            if (symbol_lookup(temp_symbol.label) != NULL) {
                /* There is already a symbol with that label */
                valid_file = NOT_VALID;
                printf("ERROR: the symbol \"%s\" is defined more than once at line number %d\n",
                       temp_symbol.label, line_count);
            }

            if (symbol_install(&temp_symbol) == NULL) {
                valid_file = NOT_VALID;
                printf("ERROR: failed to install the symbol \"%s\" at line number %d\n",
                       temp_symbol.label, line_count);
            }
            is_symbol = FALSE;
            free(temp_symbol.label);
        }

        line_count++;
        is_symbol = FALSE; /* Reset the symbol's reading flag */
    }

    return valid_file;
}


int is_label(const char *token) {
    char *label_ptr;


    if (get_commend(token) != NOT_OPCODE || !isalpha(*token)) {
        return FALSE;
    }

    label_ptr = (char*)token;
    while (isalnum(*++label_ptr)) {
        // Iterate through the characters to check if they're alphanumeric
    }

    if (*label_ptr != '\0') {
        return FALSE;  // Return FALSE if the label contains non-alphanumeric characters
    }

    return TRUE;  // Return TRUE if the label is a valid label
}



int is_label_decleration(const char *token) {
    return (strlen(token) >= 2 && token[strlen(token) - 1] == ':');
}

char *get_label(char *token) {
    char *label;
    label =  malloc(sizeof(char) * MAX_LABEL_LEN);

    if (label == NULL) {
        printf("ERROR: Memory allocation failed for label in line %d.\n", line_count);
        valid_file = FALSE;
    }

    char *label_ptr = label;
    PUT_WORD(label_ptr, token);

    if (is_label_decleration(label)) {
        label[strlen(label) - 1] = '\0';
    }

    if (get_commend(label) != NOT_OPCODE) {
        valid_file = NOT_VALID;
        printf("ERROR: the label %s with invalid name at line %d, can't use command keyword as label\n",
               label, line_count);
        free(label); // Free the allocated memory before returning NULL
        return NULL;
    }

    if (!isalpha(*label)) {
        valid_file = NOT_VALID;
        printf("ERROR: the label %s with invalid name at line %d, can't start label with non-letter character\n",label, line_count);
        free(label); // Free the allocated memory before returning NULL
        return NULL;
    }

    label_ptr = label;
    while (isalnum(*++label_ptr));

    if (*label_ptr != '\0') {
        valid_file = NOT_VALID;
        printf("ERROR: the label %s with invalid name at line %d, can't have label with non-letter or digit character\n",
               label, line_count);
        free(label); // Free the allocated memory before returning NULL
        return NULL;
    }

    return label;
}

int get_commend(const char *token) {
    int i;
    static const char *commend_list[] = {
            "mov", "cmp", "add", "sub", "not", "clr", "lea",
            "inc", "dec", "jmp", "bne", "red", "prn", "jsr",
            "rts", "stop"
    };

    for (i = 0; i < sizeof(commend_list) / sizeof(commend_list[0]); i++) {
        if (strcmp(token, commend_list[i]) == 0) {
            return i;  /*Return the corresponding opcode value*/
        }
    }

    return NOT_OPCODE;  /*Return NOT_OPCODE if the token is not a recognized opcode*/
}


int is_string(const char *data_values) {
    return ((strlen(data_values) >= 3) &&
            (data_values[strlen(data_values) - 1] == '\"') &&
            (data_values[0] == '\"'));  // Check if the string starts and ends with double quotes
}



int is_data(const char *data) {
    int num;
    char *data_ptr;

    num = (int) strtol(data, &data_ptr, 10);

    if (*data_ptr == '\0' && data_value_is_valid(num) && data_ptr != data ) {
        return TRUE;  /*Return TRUE if the data is a valid value*/
    }

    return FALSE;  /*Return FALSE if the data is not a valid value*/
}

int data_value_is_valid(int value) {
    if (value <= MAX_DATA_VALUE && value >= MIN_DATA_VALUE) {
        return TRUE;  /*Return TRUE if the value is within the valid range*/
    }

    return FALSE; /*Return FALSE if the value is not within the valid range*/
}

void put_data_in_image(int value) {
    if (dc + 1 <= TAB_SIZE - 1) {
        word_image[dc++].word = value;  /*Store the value in the image*/
    } else {
        printf("ERROR: no more space in assembler's data image at line: %d\n", line_count);
        valid_file = NOT_VALID;
    }
}

int get_data_type(const char *token) {
    int i;
    static char *data_keywords[] = { ".data", ".string", "extern", "entry" };

    if (*token == '.') {
        for (i = 0; i < sizeof(data_keywords) / sizeof(data_keywords[0]); i++) {
            if (strcmp(data_keywords[i], token) == 0) {
                return i;  // Return the index of the matched data type
            }
        }
        printf("ERROR: unknown data type at line %d\n", line_count);
        valid_file = NOT_VALID;
    }

    return NOT_DATA_TYPE;  /*Return NOT_DATA_TYPE if the token is not a recognized data type*/
}




struct nlist *symbol_install(symbol *symbol_entry) {
    return install(symbol_table, symbol_entry->label, symbol_entry, TAB_SIZE);

}

struct nlist *symbol_lookup(char *label)
{
    return lookup(symbol_table, label, TAB_SIZE);
}


void get_command_parameters(int command_type, const char *parameters) {
    char *param1 = (char *)malloc(MAX_LINE_LEN * sizeof(char));
    char *param2 = (char *)malloc(MAX_LINE_LEN * sizeof(char));
    int param1_type = 0, param2_type = 0;
    char *param_temp_ptr;

    if(parameters == NULL)
        return;

    SKIP_WHITE_SPACE(parameters);
    param_temp_ptr = param1;
    PUT_WORD(param1, parameters);
    param1 = param_temp_ptr;
    get_operand_type(param1, &param1_type);

    SKIP_WHITE_SPACE(parameters);
    if (parameters != NULL && *parameters == ',') {
        parameters++;  // Move past the comma
        SKIP_WHITE_SPACE(parameters);
        param_temp_ptr = param2;
        PUT_WORD(param2, parameters);
        param2 = param_temp_ptr;
        get_operand_type(param2, &param2_type);
    }

    is_prototype_match(command_type, param1_type, param2_type);
}

void get_operand_type(const char *operand, int *operand_type) {
    if (is_string(operand) || is_data(operand)) {
        *operand_type = IMMEDIATE_ADD;
    } else if (is_label(operand)) {
        *operand_type = DIR_ADD;
    } else if (is_register(operand)) {
        *operand_type = DIR_REGISTER_ADD;
    } else {
        *operand_type = NOT_ADD;
        printf("ERROR: Operand at line %d is not a known operand type or not valid.\n", line_count);
        valid_file = NOT_VALID;
    }
}


int is_register(const char *operand) {
    return (operand != NULL && *operand == '@'); /*Check if operand starts with '@'*/
}

/* Function to parse and handle a single data value */
void get_data(const char *data_values) {
    int data_value;
    char *line_ptr, *end_ptr;

    if(*data_values == '\0')
        return;

    line_ptr = (char *)data_values; /* Initialize line_ptr */

    /* Loop through the data values */
    while (*line_ptr) {
        SKIP_WHITE_SPACE(line_ptr);

        /* Convert the next data value */
        data_value = strtol(line_ptr, &end_ptr, 10);

        /* Check if a valid number was parsed */
        if (line_ptr == end_ptr) {
            printf("ERROR: No valid data value found at line %d.\n", line_count);
            valid_file = NOT_VALID;
            break;
        }

        /* Check for non-digit characters */
        if (ispunct(*line_ptr) && *line_ptr != ',') {
            printf("ERROR: Wrong syntax for data values found at line %d.\n", line_count);
            valid_file = NOT_VALID;
            break;
        }

        /* Check if the integer can be stored in a 12-bit word */
        if (!data_value_is_valid(data_value)) {
            printf("ERROR: Data value out of range at line %d.\n", line_count);
            valid_file = NOT_VALID;
            break;
        } else {
            /* Store the valid data value in the image */
            put_data_in_image(data_value);
        }

        /* Move to the next character after the parsed number */
        line_ptr = end_ptr;

        /* Check for a comma and prepare for the next iteration */
        if (*line_ptr == ',') {
            line_ptr++;  /* Move past the comma */
        } else if (*line_ptr) {
            printf("ERROR: Unexpected character found at line %d.\n", line_count);
            valid_file = NOT_VALID;
            break;
        }
    }
}

/* Gets a string that represents the value of the string data type and stores it in the image of declarations */
void get_string(const char *string_value) {
    SKIP_WHITE_SPACE(string_value);
    if (is_string(string_value)) {
        while (isprint(*++string_value) && *string_value != '\"') {
            word_image[(dc)++].word = *(unsigned char *)string_value;
        }
        if (*string_value != '\"') {
            printf("ERROR: the character: %c is not valid for string an argument at line %d\n",*string_value, line_count);
            valid_file = NOT_VALID;
        }
        word_image[(dc)++].word = '\0'; /* Appends "end-of-line" indicator */
    } else {
        printf("ERROR: the string argument: \"%s\" is not in the correct string format: \"<string>\" at line %d\n",string_value, line_count);
        valid_file = NOT_VALID;
    }
}


void is_prototype_match(int command_type, int param1_type, int param2_type) {
    const static prototype prototypes[] = {
            {{TRUE,  TRUE,  TRUE},  {FALSE, TRUE,  TRUE}},/* mov */
            {{TRUE,  TRUE,  TRUE},  {TRUE,  TRUE,  TRUE}},/* cmp */
            {{TRUE,  TRUE,  TRUE},  {FALSE, TRUE,  TRUE}},/* add */
            {{TRUE,  TRUE,  TRUE},  {FALSE, TRUE,  TRUE}},/* sub */
            {{FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE}},/* not */
            {{FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE}},/* clr */
            {{FALSE, TRUE,  FALSE}, {FALSE, TRUE,  TRUE}},/* lea */
            {{FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE}},/* inc */
            {{FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE}},/* dec */
            {{FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE}},/* jmp */
            {{FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE}},/* bne */
            {{FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE}},/* red */
            {{FALSE, FALSE, FALSE}, {TRUE,  TRUE,  FALSE}},/* prn */
            {{FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE}},/* jsr */
            {{FALSE, FALSE, FALSE}, {FALSE, FALSE, FALSE}},/* rts */
            {{FALSE, FALSE, FALSE}, {FALSE, FALSE, FALSE}}/* stop */
    };
    if (command_type >= 0 && command_type < sizeof(prototypes) / sizeof(prototypes[0]))
    {
        if ((MOV <= command_type && command_type <= SUB || command_type == LEA) && param1_type &&
            param2_type)/*if the command is a two-parameter type*/
        {
            is_operand_match(prototypes[command_type].src_operand, param1_type);
            is_operand_match(prototypes[command_type].dest_operand, param2_type);
        }
        else if ((command_type == NOT || command_type == CLR || INC <= command_type && command_type <= JSR) &&
                 param1_type && !param2_type)
        {
            is_operand_match(prototypes[command_type].dest_operand, param1_type);
        }
        else if (!((command_type == RTS || command_type == STOP) && !param1_type && !param2_type))
        {/*if the */
            printf("ERROR: The command doesn't match the number of parameters given in line %d\n", line_count);
            valid_file = NOT_VALID;
        }

    }
}


void is_operand_match(operand_type operand_type, int type) {
    if (!(type == IMMEDIATE_ADD && operand_type.immediate_address ||
          type == DIR_ADD && operand_type.Direct_address ||
          type == DIR_REGISTER_ADD && operand_type.dir_register_address)) {
        printf("ERROR: Mismatch between parameter type accepted and operand type at line %d.\n", line_count);
        valid_file = NOT_VALID;
    }
}