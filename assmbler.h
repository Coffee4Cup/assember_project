

#ifndef ASMBLER_2_ASSMBLER_H
#define ASMBLER_2_ASSMBLER_H


#include <stdio.h>
#include "lookup_table.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define TAB_SIZE 101
#define MAX_LINE_LEN 81
#define MAX_LABEL_LEN 31
#define MAX_DATA_VALUE 2047
#define MIN_DATA_VALUE (-2048)

/*instructed that the maximum line length is 80, not including the newline char*/

/*The max and min values that a word in memory can have using 12 bits using two's complement representation. */


/*------------- structs------------*/
typedef struct
{/*represents a symbol entry in the .as file*/
    char *label;
    int memory_address;
    int symbol_type;
}symbol;

typedef struct
{/*represents a word in memory */
    unsigned word : 12;
}word;
typedef struct
{/*represents instruction word*/
    unsigned ARE : 2;
    unsigned opcode : 4;
    unsigned dest_operand :3;
    unsigned src_operand :3;

}instruction_word;
typedef struct
{/*represent the possible types of elements a given operand can be for a given function*/
    /*each of these is a flag to receive in the prototype struct */
    unsigned immediate_address :1;
    unsigned Direct_address : 1;
    unsigned dir_register_address :1;

}operand_type;
typedef struct
{/*represents a prototype of a function if a function takes only one destination or none at all, the operand will be null */
    operand_type src_operand;
    operand_type dest_operand;
}prototype;

/*----------enums------------------*/
enum data_types{DATA, STRING, EXTERN, ENTRY, NOT_DATA_TYPE};
enum commend_types
{
    MOV, CMP, ADD, SUB, NOT, CLR,
    LEA, INC, DEC, JMP, BNE, RED,
    PRN, JSR, RTS, STOP, NOT_OPCODE
};
enum parameter_address_types{IMMEDIATE_ADD = 1, DIR_ADD = 3, DIR_REGISTER_ADD = 5, NOT_ADD = 0};
enum symbol_type {DATA_TYPE, COMMEND_TYPE};
enum {NOT_VALID, VALID};
enum {FALSE, TRUE};

/*-------- main functions ------------*/
void assemble(FILE *file);
int first_pass(void);
int second_pass(void);

/*---------parsing functions-----------*/

int is_label_decleration(const char *token);
char *get_label(char *token);

int data_value_is_valid(int value);
int get_data_type(const char *token);
void get_data(const char *data_value_str);
void put_data_in_image(int value);


void get_string(const char *parameters);
int is_string(const char *data_values);
int get_commend(const char *token);
void get_command_parameters(int command_type,const char *token);

/*----------implementation of the generic lookup table for symbols------------*/

/*note: I tied to use the lookup table example from the book and implement a generic type of it*/
struct nlist *symbol_lookup(char *label);
struct nlist *symbol_install(symbol *data);

void is_operand_match(operand_type operand_type, int type);
void is_prototype_match(int command_type, int param1_type, int param2_type);
void get_operand_type(const char *operand,  int *operand_type);
int is_data(const char *data);
int is_register(const char *reg);

#endif /*ASSMBLER_ASSMBLER_H*/

