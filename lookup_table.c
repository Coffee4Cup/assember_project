/** This program includes functions related to a hash table and symbol manipulation. */

#include <stdlib.h>
#include <string.h>
#include "lookup_table.h"

/** Computes a hash value for a given name and hash size. */
static unsigned int hash(char *name, int hash_size) {
    unsigned hashval;

    /* Handle case where name is NULL */
    if (name == NULL)
        return -1;

    /* Calculate hash value using a simple algorithm */
    for (hashval = 0; *name != '\0'; name++) {
        hashval = *name + 31 * hashval;
    }
    return hashval % hash_size;
}

/** Typedef for a function that duplicates data. */
typedef void *(*dup_func)(void *data);

/** Looks up a name in a hash table and returns the corresponding nlist. */
struct nlist *lookup(struct nlist *hashtab[], char *name, int hash_size) {
    struct nlist *np;

    /* Search for the name in the hash table */
    for (np = hashtab[hash(name, hash_size)]; np != NULL; np = np->next) {
        if (!strcmp(np->name, name))
            return np;
    }
    return NULL;
}

/**
 * Installs a name and associated data in the hash table, using a provided duplication function.
 * If the name already exists, updates the associated data with the new duplicated version.
 */
struct nlist *install(struct nlist *hashtab[], char *name, void *data, int hash_size, dup_func duplicate) {
    struct nlist *np;
    unsigned int hashval;

    /* Check if the name is already in the hash table */
    if ((np = lookup(hashtab, name, hash_size)) == NULL) {
        np = (struct nlist *)malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL)
            return NULL;

        hashval = hash(name, hash_size);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else {
        free(np->data);
    }

    np->data = duplicate(data);

    return np;
}

/** Duplicates a string by allocating new memory and copying the data. */
void *duplicate_string(void *data) {
    if (data == NULL) {
        return NULL;
    }

    char *original_string = (char *)data;
    char *new_string = strdup(original_string);
    if (new_string == NULL) {
        /* Memory allocation failed */
        return NULL;
    }

    return new_string;
}

/**
 * Installs a name and associated data in the hash table using the default duplication function.
 * The default duplication function duplicates strings.
 */
struct nlist *defult_install(struct nlist *hashtab[], char *name, void *data, int hash_size) {
    return install(hashtab, name, data, hash_size, duplicate_string);
}
