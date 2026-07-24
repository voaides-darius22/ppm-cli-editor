#include <stdlib.h>
#include <string.h>

#include "lsys.h"
#include "io_utils.h"

Lsystem *open_lsystem_file(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return NULL;
    }

    char buffer[1024];

    Lsystem *new_file = calloc(1, sizeof(*new_file));
    if (!new_file) {
        fclose(fp);
        return close_lsystem_file(new_file);
    }

    // Reading lsystem axiom
    fgets(buffer, 1024, fp);
    clean_fgets_input(buffer, fp);

    new_file->axiom = malloc(strlen(buffer) + 1);
    if (!new_file->axiom) {
        fclose(fp);
        return close_lsystem_file(new_file);
    }
    strcpy(new_file->axiom, buffer);

    // Reading lsystem rules
    uint32_t nrules;
    fscanf(fp, "%d", &nrules);
    fgetc(fp);
    
    new_file->rules = create_hash_table(nrules, hash_helper);
    if (!new_file->rules) {
        fclose(fp);
        return close_lsystem_file(new_file);
    }

    char *token = NULL, *symbol, *successor;
    for (int i = 0; i < nrules; i++) {
        fgets(buffer, 1024, fp);
        clean_fgets_input(buffer, fp);
        token = strtok(buffer, " ");
        if (!token) {
            fclose(fp);
            return close_lsystem_file(new_file);
        }

        // Memory allocation for key
        symbol = malloc(strlen(token) + 1);
        if (!symbol) {
            fclose(fp);
            return close_lsystem_file(new_file);
        }
        strcpy(symbol, token);

        token = strtok(NULL, " ");
        if (!token) {
            fclose(fp);
            free(symbol);
            return close_lsystem_file(new_file);
        }
        successor = malloc(strlen(token) + 1);
        if (!successor) {
            free(symbol);
            fclose(fp);
            return close_lsystem_file(new_file);
        }
        strcpy(successor, token);
        put(new_file->rules, symbol, successor);
    }

    fclose(fp);
    return new_file;
}

Lsystem *close_lsystem_file(Lsystem *lsys)
{
    if (!lsys) {
        return NULL;
    }

    free(lsys->axiom);
    free_hash_table(lsys->rules, free);
    free(lsys);
    return NULL;
}