//
// Created by itay8 on 17/08/2023.
//

#ifndef ASMBLER_2_LOOKUP_TABLE_H
#define ASMBLER_2_LOOKUP_TABLE_H
#include <stdlib.h>


struct nlist {
    struct nlist *next;
    char *name;
    void *data;
};

/*generic function for duplication of the data */
void *dup_data(void *data);

/*implementation of the lookup and install fucntions from the book, implemented it generically for using it to store symbols and macros */
struct nlist *lookup(struct nlist *hashtab[], char *name, int hash_size);
struct nlist *install(struct nlist *hashtab[], char *name, void *data, int hash_size);


#endif //ASMBLER_2_LOOKUP_TABLE_H
