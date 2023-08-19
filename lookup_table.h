#ifndef ASMBLER_2_LOOKUP_TABLE_H
#define ASMBLER_2_LOOKUP_TABLE_H
#include <stdlib.h>

#define TAB_SIZE 101

struct nlist {
    struct nlist *next;
    char *name;
    void *data;
};

/*generic function for duplication of the data */
typedef void *(*dup_func)(void *data);

/*implementation of the lookup and install fucntions from the book, implemented it generically for using it to store symbols and macros */

struct nlist *lookup(struct nlist *hashtab[], char *name, int hash_size);
struct nlist *install(struct nlist *hashtab[], char *name, void *data, int hash_size, dup_func duplicate);
struct nlist *defult_install(struct nlist *hashtab[], char *name, void *data, int hash_size);
void *duplicate_string(void *data);

#endif //ASMBLER_2_LOOKUP_TABLE_H