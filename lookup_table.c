// Created by itay8 on 30/07/2023.

#include <stdlib.h>
#include <string.h>
#include "lookup_table.h"

static unsigned int _hash(char *name, int hash_size) {
    unsigned hashval;
    if (name == NULL)
        return -1;
    for (hashval = 0; *name != '\0'; name++) {
        hashval = *name + 31 * hashval;
    }
    return hashval % hash_size;
}

typedef void *(*dup_func)(void *data);

struct nlist *lookup(struct nlist *hashtab[], char *name, int hash_size) {
    struct nlist *np;
    for (np = hashtab[_hash(name,hash_size) ]; np != NULL; np = np->next)
        if (!strcmp(np->name, name))
            return np;
    return NULL;
}

struct nlist *install(struct nlist *hashtab[], char *name, void *data, int hash_size, dup_func duplicate) {
    struct nlist *np;
    unsigned int hashval;

    if ((np = lookup(hashtab, name, hash_size)) == NULL) {
        np = (struct nlist *)malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL)
            return NULL;
        hashval = _hash(name, hash_size);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else {
        free(np->data);
    }

    np->data = duplicate(data);

    return np;
}
