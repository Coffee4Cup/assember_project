#include <stdint.h>
#include "assmbler.h"
#define SKIP_WHITE_SPACE(A) while((A) != NULL && *(A) != '\0' && (*(A) == ' ' || *(A) == '\t')) (A)++;
/*copy a word and changes the last char to be an end-of-line char, for the cases when <word><,>, this copy only <word>*/

#define PUT_WORD(A,B) while((B) != NULL && *(B) != '\0' &&  *(B) != '\t' && *(B) != ' ' && *(B) != ',' ) \
       *(A)++ = *(B)++;                                                                                           \
*(A) ='\0';


#define word_size 12
static int ic = 0;
static int dc = 0;
static struct nlist *symbol_table[TAB_SIZE];
static memory_word word_image[TAB_SIZE];
static instruction_word instruction_image[TAB_SIZE];
static int valid_file;
static int line_count;
static FILE *input_file;

void assemble(FILE *file) {
    input_file = file;
    first_pass();
    print_instruction_image();
    print_word_image();
    /*second_pass();*/
}
void print_word_image(){
    int i;
    for(i = 0; i < dc ; i++)
        print_memory_word(word_image[i].word);
}
void print_memory_word(unsigned int word) {


    int i;
    for (i = 11; i >= 0; i--) {
        printf("%d", (word >> i) & 1);
    }
    printf("\n");
}
void print_absolute_value(instruction_absolute_value inst) {

    unsigned int value = (inst.value << 2) | inst.ARE;

    int i;
    for (i = 11; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
    }
    printf("\n");
}
void print_word(unsigned int value) {

    int i;
    for (i = 11; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
    }
    printf("\n");
}
void reset_first_pass_values() {
    ic = 0;
    dc = 0;
    valid_file = TRUE;
    line_count = 1;

    memset(symbol_table, 0, sizeof(symbol_table));
    memset(word_image, 0, sizeof(word_image));
    memset(instruction_image, 0, sizeof(instruction_image));
}


void print_instruction(instruction_signature inst) {
    int i;
    unsigned int value = (inst.src_operand << 9) | (inst.opcode << 5) | (inst.dest_operand << 2) | inst.ARE;

    for (i = word_size - 1; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
    }
    printf("\n");
}
void printBinaryRegister(instruction_register_value inst) {
    int i;
    unsigned int value = (inst.source_register << 7) | (inst.destination_register << 2) | inst.ARE;

    for (i = word_size - 1; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
    }
    printf("\n");
}



void print_instruction_image() {
    int i = 0;
    int param_lines_num = 0; /* the number of parameter lines from the command line */
    int first_param_type = instruction_image[i].signature.dest_operand;
    while (i < ic) {
        print_instruction(instruction_image[i].signature);

        fflush(stdout);
        i += (print_parameters(i, instruction_image[i].signature) + 1);/*prints the parameters and return the number of lines it took*/
        fflush(stdout);
    }
}

int print_parameters(int i, instruction_signature signature) {

    int num_lines = 2;
    if(signature.src_operand == DIR_REGISTER_ADD && signature.dest_operand == DIR_REGISTER_ADD) {
        printBinaryRegister(instruction_image[i + 1].reg_value);
    return 0;
    }
    if(signature.src_operand == DIR_REGISTER_ADD)
        printBinaryRegister(instruction_image[++i].reg_value);
    else if(signature.src_operand == DIR_ADD)
        printf("%s\n", instruction_image[++i].label);
    else if(signature.src_operand == IMMEDIATE_ADD)
        print_absolute_value(instruction_image[++i].data_value);
    else
        num_lines--;
    if(signature.dest_operand == DIR_REGISTER_ADD)
        printBinaryRegister(instruction_image[++i].reg_value);
    else if(signature.dest_operand == DIR_ADD)
        printf("%s\n", instruction_image[++i].label);
    else if(signature.dest_operand == IMMEDIATE_ADD)
        print_absolute_value(instruction_image[++i].data_value);
    else
        num_lines--;
    return num_lines;
}


int first_pass() {

    int data_type, command_code;
    char *token, line[MAX_LINE_LEN];

    reset_first_pass_values();

    symbol temp_symbol; /*temporary symbol struct that holds values for a symbol entry later*/
    int is_symbol;      /*flag for defining symbol state ==OUT/ IN*/

    while (fgets(line, sizeof(line), input_file)) {
        token = strtok(line, " \t\n");

        if(token == NULL) {
            line_count++;
            break;
        }
        else if (is_label_decleration(token)) {
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
                    break;
            }
        } else if ((command_code = get_command_code(token)) != NOT_OPCODE) {/* If the token is a command */

            /*saves the type of level and the address*/
            temp_symbol.symbol_type = COMMAND_TYPE;
            temp_symbol.memory_address = ic;

            /*install instruction in the instruction image*/
            token = strtok(NULL, "\n"); /* Takes the rest of the line as a parameter for the data type */
            get_command(command_code, token);
        }

        if (is_symbol == TRUE) {
            /* Found all the information for making symbol entry */

            if (symbol_lookup(temp_symbol.label) != NULL) {
                /* There is already a symbol with that label */
                valid_file = FALSE;
                printf("ERROR: the symbol \"%s\" is defined more than once at line number %d\n", temp_symbol.label,
                       line_count);
            }

            if (symbol_install(&temp_symbol) == NULL) {
                valid_file = FALSE;
                printf("ERROR: failed to install the symbol \"%s\" at line number %d\n", temp_symbol.label, line_count);
            }
            is_symbol = FALSE;/*reset the flag of the symbol*/
            free(temp_symbol.label);
        }

        line_count++;
    }
    return valid_file;
}

/***
 * checks if a given string could be a valid label name.
 * */
int is_valid_label(const char *token) {

    if (get_command_code(token) != NOT_OPCODE || !isalpha(*token))
        return FALSE;

    while (isalnum(*++token));

    if (*token != '\0')
        return FALSE;

    return TRUE;
}



int is_label_decleration(const char *token) {

    return (strlen(token) >= 2 && token[strlen(token) - 1] == ':');
}
/*takes a token that represents a string of a label, checks if the token is valid as label and returns */
char *get_label(char *token)
{
    char *label;
    char *label_ptr;
    label =  malloc(sizeof(char) * MAX_LABEL_LEN);

    if (label == NULL)
    {
        printf("ERROR: Memory allocation failed for label in line %d.\n", line_count);
        valid_file = FALSE;
        return NULL;
    }

    label_ptr = label;
    PUT_WORD(label_ptr, token);

    if (is_label_decleration(label))
        label[strlen(label) - 1] = '\0';
    else
        label[strlen(label)] = '\0';

    if (get_command_code(label) != NOT_OPCODE)
    {
        valid_file = FALSE;
        printf("ERROR: the label %s with invalid name at line %d, can't use command keyword as label\n",
           label, line_count);
        valid_file = FALSE;
        free(label);
        return NULL;
    }
    else if(!isalpha(*label))
    {
        valid_file = FALSE;
        printf("ERROR: the label %s with invalid name at line %d, can't start label with non-letter character\n",label, line_count);
        free(label);
        return NULL;
    }

    label_ptr = label;
    while (isalnum(*++label_ptr));

    if (*label_ptr != '\0') {
        valid_file = FALSE;
        printf("ERROR: the label %s with invalid name at line %d, can't have label with non-letter or digit character\n",
               label, line_count);
        free(label); // Free the allocated memory before returning NULL
        return NULL;
    }

    return label;
}



void put_instruction_in_image(int code, int source_type, int destination_type, char *destination_parameter, char * source_parameter)
{
    instruction_image[ic].signature.ARE = Absolute;
    instruction_image[ic].signature.opcode = code;
    instruction_image[ic].signature.src_operand = source_type;
    instruction_image[ic].signature.dest_operand = destination_type;
    ic++;
    put_instruction_values_in_image(source_type, destination_type, destination_parameter, source_parameter);

}




/*puts the instruction in the image of the instructions. assumes that the parameter types are valid and checks the values of the parameter for the claimed type given */
void put_instruction_values_in_image(int source_type, int destination_type, char *destination_parameter, char *source_parameter) {
    instruction_word inst_word;


    if(source_type == DIR_REGISTER_ADD && destination_type == DIR_REGISTER_ADD)
    {/*in case both parameters are a register type*/
        inst_word.reg_value.source_register = get_register(source_parameter);
        inst_word.reg_value.destination_register = get_register(destination_parameter);
        inst_word.reg_value.ARE = Absolute;
        instruction_image[ic] = inst_word;
        ic++;
        return;
    }
    if (source_type != NOT_ADD && source_parameter != NULL ) {
        if (source_type == DIR_REGISTER_ADD) {
            inst_word.reg_value.source_register = get_register(source_parameter);
            inst_word.reg_value.ARE = Absolute;
        } else if (source_type == IMMEDIATE_ADD) {
            inst_word.data_value.value = get_operand_data(source_parameter);
            inst_word.data_value.ARE = Absolute;
        } else if (source_type == DIR_ADD) {
            inst_word.data_value.value = 0;
            inst_word.data_value.ARE = Relocatable;
            strncpy(inst_word.label, source_parameter, MAX_LABEL_LEN - 1);
            inst_word.label[MAX_LABEL_LEN - 1] = '\0';  // Ensure null-termination
        }
        instruction_image[ic] = inst_word;
        ic++;
    }

    if (destination_type != NOT_ADD && destination_parameter != NULL) {
        if (destination_type == DIR_REGISTER_ADD) {
            inst_word.reg_value.destination_register = get_register(destination_parameter);
            inst_word.reg_value.ARE = Absolute;
        } else if (destination_type == IMMEDIATE_ADD) {
            inst_word.data_value.value = get_operand_data(destination_parameter);
            inst_word.data_value.ARE = Absolute;
            instruction_image[ic] = inst_word;
            ic++;
        } else if (destination_type == DIR_ADD) {
            inst_word.data_value.ARE = Relocatable;
            strncpy(inst_word.label, destination_parameter, strlen(destination_parameter) + 1);
            inst_word.label[strlen(destination_parameter) + 1] = '\0';
            instruction_image[ic] = inst_word;
            ic++;
        }
    }
}
/* return the value of a single character or a number given to a function, checks if their values can fit inside a 10 bit word
 * if the values are not valid return -1 */
unsigned int get_operand_data(char *operand) {
    if (is_character_declaration(operand)) /* if it is a single character with syntax: <'><char><'> */
        return *(operand + 1); /* return the char itself */
    else if (is_10bit_number(operand))
        return atoi(operand);
    else {
        printf("ERROR: invalid immediate parameter for function at line %d", line_count);
        valid_file = FALSE; // Assuming valid_file is a global flag variable
    }
    return 0;
}
int is_character_declaration(const char *operand) {
    return (strlen(operand) == 3 &&
            operand[0] == '\'' &&
            operand[2] == '\'' &&
            isprint(operand[1]));
}

/*checks if the operand is a 10 bit number that will fit inside a parameter word of a function */
int is_10bit_number(char *operand) {
    char *endptr;
    long number = strtol(operand, &endptr, 10); /*Convert string to long integer*/

    /*Check if the conversion was successful and the entire operand was consumed*/
    if (*endptr == '\0' && number >= -512 && number <= 511) {
        return 1; /*Return 1 if it's a valid 10-bit number*/
    }

    return 0; /*Return 0 if it's not a valid 10-bit number*/
}

/*get_register checks if register_string is in format: <@r><number><end-of-line> and checks if the number is a valid number of a register. return -1 if the string is not valid and the number of register if valid */
unsigned int get_register(char *register_string) {
    int register_number = -1;
    if(sscanf(register_string, "@r%d", &register_number) == 0)
    {
        printf("ERROR: unknown register type at line %d", line_count);
        valid_file = FALSE;
    }
    if(register_number < 1 || register_number > 8)
    {
        printf("ERROR: unknown register number at line %d", line_count);
        valid_file = FALSE;
    }
    return register_number;

}

/*handles the process of the parameters from the given string, and using the other functions check their validity and put the instruction in the image*/
void get_command(int command_code, const char *parameters_string)
{
    char* first_parameter = malloc(MAX_LINE_LEN);
    char*  second_parameter = malloc(MAX_LINE_LEN);
    int first_type, second_type;
    get_command_parameters(command_code, parameters_string, &first_parameter, &second_parameter);

        get_operand_type(first_parameter, &first_type);
        get_operand_type(second_parameter, &second_type);

    if(is_prototype_match(command_code, first_type, second_type))
    {
        if(!second_type)
            put_instruction_in_image(command_code, 0,first_type , first_parameter, 0);
        else
             put_instruction_in_image(command_code, first_type, second_type, second_parameter ,first_parameter);


    }
}
/*return a number value representing the command that was writen in the source file*/
int get_command_code(const char *token)
{
    int i;
    static const char *commend_list[] =
            {
            "mov", "cmp", "add", "sub", "not", "clr", "lea",
            "inc", "dec", "jmp", "bne", "red", "prn", "jsr",
            "rts", "stop"
    };

    for (i = 0; i < sizeof(commend_list) / sizeof(commend_list[0]); i++) {
        if (strcmp(token, commend_list[i]) == 0) {
            return i;  /*Return the corresponding command code value*/
        }
    }

    return NOT_OPCODE;  /*Returns if the token is not a recognized command keyword*/
}


int is_string_decleration(const char *data_values) {
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
        valid_file = FALSE;
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
        valid_file = FALSE;
    }

    return NOT_DATA_TYPE;  /*Return NOT_DATA_TYPE if the token is not a recognized data type*/
}




/*gets a string from the file that holds the parameter or parameters of a command */
void get_command_parameters(int command_code, const char *parameters_string, char **first_parameter, char **second_parameter) {
    int second_parameter_type = NOT_OPCODE, first_parameter_type = NOT_OPCODE;
    char *param_temp_ptr;

    SKIP_WHITE_SPACE(parameters_string);

    param_temp_ptr = *first_parameter;
    PUT_WORD(*first_parameter, parameters_string);
    *first_parameter = param_temp_ptr;
    get_operand_type(*first_parameter, &first_parameter_type);

    SKIP_WHITE_SPACE(parameters_string);

    if (parameters_string != NULL && *parameters_string == ',') {
        parameters_string++;
        SKIP_WHITE_SPACE(parameters_string);
        param_temp_ptr = *second_parameter;
        PUT_WORD(*second_parameter, parameters_string);
        *second_parameter = param_temp_ptr;
        get_operand_type(*second_parameter, &second_parameter_type);
    }
}

void get_operand_type(const char *operand, int *operand_type) {
    if (is_string_decleration(operand) || is_data(operand)) {
        *operand_type = IMMEDIATE_ADD;
    } else if (is_valid_label(operand)) {
        *operand_type = DIR_ADD;
    } else if (is_register_requests(operand)) {
        *operand_type = DIR_REGISTER_ADD;
    } else {
        *operand_type = NOT_ADD;
    }
}

int is_register_requests(const char *operand) {
    return (operand != NULL && *operand == '@'); /*Check if operand starts with '@'*/
}


/* Function to parse and handle  */
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
        SKIP_WHITE_SPACE(end_ptr);/*skips the space between the end of the number and the next comma*/

        /* Check if a valid number was parsed */
        if (line_ptr == end_ptr) {
            printf("ERROR: No valid data value found at line start of initialization at line %d.\n", line_count);
            valid_file = FALSE;
            break;
        }

        /*Check for non-digit characters, a print  */
        if (*end_ptr == ',')
            end_ptr++;
        else if(*end_ptr != '\0')
        {/*it is not a comma and not the end of the data_values,
 * and we skipped spaces, so it must be non-valid syntax for data*/
            printf("ERROR: Wrong syntax for data values found at line %d.\n", line_count);
            valid_file = FALSE;
           break;
        }

        /* Check if the integer can be stored in a 12-bit word */
        if (!data_value_is_valid(data_value)) {
            printf("ERROR: Data value out of range at line %d.\n", line_count);
            valid_file = FALSE;
            break;
        } else
        {
            /* Store the valid data value in the image */
            put_data_in_image(data_value);
        }
        line_ptr = end_ptr;

    }
}

/* Gets a string that represents the value of the string data type and stores it in the image of declarations */
void get_string(const char *string_value)
{

    SKIP_WHITE_SPACE(string_value);
    if (is_string_decleration(string_value))
    {
        while (isprint(*++string_value) && *string_value != '\"')
            put_data_in_image(*string_value);
        if (*string_value != '\"') {
            printf("ERROR: the character: %c is not valid for string an argument at line %d\n",*string_value, line_count);
            valid_file = FALSE;
        }
        word_image[(dc)++].word = '\0'; /* Appends "end-of-line" indicator */
    } else {
        printf("ERROR: the string argument: \"%s\" is not in the correct string format: \"<string>\" at line %d\n",string_value, line_count);
        valid_file = FALSE;
    }
}


int is_prototype_match(int command_code, int first_param_type, int second_param_type) {
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
    if (command_code >= 0 && command_code < sizeof(prototypes) / sizeof(prototypes[0]))
    {
        if ((MOV <= command_code && command_code <= SUB || command_code == LEA) && first_param_type &&
            second_param_type)/*if the command is a two-parameter type*/
        {
            is_operand_match(prototypes[command_code].src_operand, first_param_type);
            is_operand_match(prototypes[command_code].dest_operand, second_param_type);
        }
        else if ((command_code == NOT || command_code == CLR || INC <= command_code && command_code <= JSR) &&
                 first_param_type && !second_param_type)
        {
            is_operand_match(prototypes[command_code].dest_operand, first_param_type);
        }
        else if (!((command_code == RTS || command_code == STOP) && !first_param_type && !second_param_type))
        {/*if the */
            printf("ERROR: The command doesn't match the number of parameters given in line %d\n", line_count);
            valid_file = FALSE;
            return FALSE;
        }
    }
    return TRUE;
}


void is_operand_match(operand_type operand_type, int type) {
    if (!(type == IMMEDIATE_ADD && operand_type.immediate_address ||
          type == DIR_ADD && operand_type.Direct_address ||
          type == DIR_REGISTER_ADD && operand_type.dir_register_address)) {
        printf("ERROR: Mismatch between parameter type accepted and operand type at line %d.\n", line_count);
        valid_file = FALSE;
    }
}

struct nlist *symbol_install(symbol *symbol_entry) {
    return install(symbol_table, symbol_entry->label, symbol_entry, TAB_SIZE, (dup_func) duplicate_symbol);

}

struct nlist *symbol_lookup(char *label)
{
    return lookup(symbol_table, label, TAB_SIZE);
}


symbol *duplicate_symbol(const symbol *original) {
    if (original == NULL) {
        return NULL;
    }

    symbol *new_symbol = (symbol *)malloc(sizeof(symbol));
    if (new_symbol == NULL) {
         /*Memory allocation failed*/
        return NULL;
    }

    /*Duplicate the label*/
    new_symbol->label = strdup(original->label);
    if (new_symbol->label == NULL) {
        /*Memory allocation for label failed*/
        free(new_symbol);
        return NULL;
    }

 /*   Copy other fields*/
    new_symbol->memory_address = original->memory_address;
    new_symbol->symbol_type = original->symbol_type;

    return new_symbol;
}