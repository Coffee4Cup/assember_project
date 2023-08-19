

#ifndef ASMBLER_2_ASSMBLER_H
#define ASMBLER_2_ASSMBLER_H


#include <stdio.h>
#include "lookup_table.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_LINE_LEN 81
#define MAX_LABEL_LEN 31
#define MAX_DATA_VALUE 2047
#define MIN_DATA_VALUE (-2048)

/*instructed that the maximum line length is 80, not including the newline char*/

/*The max and min values that a word in memory can have using 12 bits using two's complement representation. */


/*------------- structs ------------*/

/* Represents a symbol entry in the .as file */
typedef struct {
    char *label;
    int memory_address;
    int symbol_type;
} symbol;

/* Represents a word in memory */
typedef struct {
    unsigned word : 12;
} memory_word;

/* Represents the value of an instruction with the attributes for Absolute or Relocatable
 * using "Parameter_address_types" enum for addressing values */
typedef struct {
    unsigned ARE : 2;
    unsigned opcode : 4;
    unsigned dest_operand : 3;
    unsigned src_operand : 3;
} instruction_signature;

/* Represents an absolute value */
typedef struct {
    unsigned value : 10;
    unsigned ARE : 2;
} instruction_absolute_value;

/* Represents register values */
typedef struct {
    unsigned source_register : 5;
    unsigned destination_register : 5;
    unsigned ARE : 2;
} instruction_register_value;

/* Represents an instruction word or a value passed to an instruction as a data type or symbol reference */
typedef union {
    instruction_signature signature;
    instruction_absolute_value data_value;
    instruction_register_value reg_value;
    char label[MAX_LABEL_LEN];
} instruction_word;

/* Represents the possible types of elements a given operand can be for a given function */
typedef struct {
    unsigned immediate_address : 1;
    unsigned Direct_address : 1;
    unsigned dir_register_address : 1;
} operand_type;

/* Represents a prototype of a function */
typedef struct {
    operand_type src_operand;
    operand_type dest_operand;
} prototype;

/*---------- enums ------------------*/

/* Possible data types */
enum data_types {
    DATA, STRING, EXTERNAL, ENTRY, NOT_DATA_TYPE
};

/* Possible command types */
enum command_types {
    MOV, CMP, ADD, SUB, NOT, CLR,
    LEA, INC, DEC, JMP, BNE, RED,
    PRN, JSR, RTS, STOP, NOT_OPCODE
};

/* Address Reallocation Enumerator */
enum ARE {
    Absolute, External, Relocatable
};

/* Parameter address values */
enum parameter_address_types {
    IMMEDIATE_ADD = 1, DIR_ADD = 3, DIR_REGISTER_ADD = 5, NOT_ADD = 0
};

/* Symbol types */
enum symbol_type {
    DATA_TYPE, COMMAND_TYPE
};

enum {
    FALSE, TRUE
};


/*-------- main functions ------------*/

void assemble(FILE *file, char *file_name);
int first_pass(void);
int second_pass(void);
void reset_first_pass_values();
/**---------parsing functions-----------**/


/*data related functions*/

int is_data(const char *data);
int data_value_is_valid(int value);
int get_data_type(const char *token);
void get_data(const char *data_value_str);
void put_data_in_image(int value);
void get_string(const char *parameters);
int is_string_decleration(const char *data_values);
int is_character_declaration(const char *operand);
void append_to_entry_file(const char *label);

/*instruction related functions */

void get_command(int command_code, const char *parameters_string);
int get_command_code(const char *token);
void get_command_parameters(int command_code, const char *parameters_string, char **first_parameter, char **second_parameter);
void is_operand_match(operand_type operand_type, int type);
void get_operand_type(const char *operand,  int *operand_type);
unsigned int get_operand_data(char *parameter);
int is_prototype_match(int command_type, int param1_type, int param2_type);
unsigned int get_register(char *register_string);
int is_register_requests(const char *reg);
int is_10bit_number(char *operand);
void put_instruction_in_image(int code, int source_type, int destination_type, char *destination_operand, char * source_operand);
void put_instruction_values_in_image(int source_type, int destination_type, char *destination_operand, char *source_operand);


/*----------implementation of the generic lookup table for symbols------------*/
    /*note: I tied to use the lookup table example from the book and implement a generic type of it*/

int is_label_decleration(const char *token);
int is_valid_label(const char *token);
char *get_label(char *token);
struct nlist *symbol_lookup(char *label);
struct nlist *symbol_install(symbol *data);
symbol *duplicate_symbol(const symbol *original);

/*testing funcitons*/
void print_instruction_image();
void print_instruction(instruction_signature inst);
int print_parameters(int i, instruction_signature signature);
void print_absolute_value(instruction_absolute_value inst);
void printBinaryRegister(instruction_register_value inst);
void print_memory_word(unsigned int word);
void print_word_image();

#endif

