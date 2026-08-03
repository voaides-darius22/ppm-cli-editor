#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "../header_files/lsys.h"
#include "../header_files/io_utils.h"
#include "../header_files/queue.h"

#define BUFFER_SIZE 1024
#define LSYSTEM_SYMBOL_SIZE 2

// SuccessorDataRule Functions
LsystemSuccessorRule *create_successor_data(const char *buffer)
{
    if (!buffer) {
        return NULL;
    }

    LsystemSuccessorRule *data = calloc(1, sizeof(*data));
    if (!data) {
        return NULL;
    }

    data->length = strlen(buffer);
    data->successor = malloc(data->length + 1);
    if (!data->successor) {
        free(data);
        return NULL;
    }
    strcpy(data->successor, buffer);
    return data;
}

void free_successor_data(void *ptr)
{
    if (!ptr) {
        return;
    }

    LsystemSuccessorRule *data = ptr;
    free(data->successor);
    free(data);
    return;
}

// Lsystem File Functions
Lsystem *open_lsystem_file(const char *path)
{
    if (!path) {
        return NULL;
    }

    FILE *fp = fopen(path, "r");
    if (!fp) {
        return NULL;
    }

    char buffer[BUFFER_SIZE];

    Lsystem *new_file = calloc(1, sizeof(*new_file));
    if (!new_file) {
        fclose(fp);
        return close_lsystem_file(new_file);
    }

    // Reading lsystem axiom
    fgets(buffer, BUFFER_SIZE, fp);
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
    new_file->nrules = nrules;
    fgetc(fp);
    
    const uint32_t PRINTABLE_ASCII_CHARS = 95;
    new_file->rules = create_hash_table(PRINTABLE_ASCII_CHARS, hash_helper);
    if (!new_file->rules) {
        fclose(fp);
        return close_lsystem_file(new_file);
    }

    char *token = NULL, *symbol;
    LsystemSuccessorRule *data;
    for (int i = 0; i < nrules; i++) {
        fgets(buffer, BUFFER_SIZE, fp);
        clean_fgets_input(buffer, fp);
        token = strtok(buffer, " ");
        if (!token) {
            fclose(fp);
            return close_lsystem_file(new_file);
        }
        symbol = token;
        token = strtok(NULL, " ");
        // Memory allocation for value
        data = create_successor_data(token);
        if (!data) {
            free(symbol);
            fclose(fp);
            return close_lsystem_file(new_file);
        }
        put(new_file->rules, symbol, data);
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
    free_hash_table(lsys->rules, free_successor_data);
    free(lsys);
    return NULL;
}

char *derive_lsys(const Lsystem *lsys, uint32_t n)
{
    if (!lsys) {
        return NULL;
    }

    if (!n) {
        char *derivative = malloc(strlen(lsys->axiom) + 1);
        if (!derivative) {
            return NULL;
        }
        strcpy(derivative, lsys->axiom);
        return derivative;
    }

    Queue queue_slots[2];
    memset(queue_slots, 0, sizeof(queue_slots));
   
    // Initialising the first queue slot with the lsystem axiom
    uint32_t axiom_length = strlen(lsys->axiom);
    for (int i = 0; i < axiom_length; i++) {
        enqueue(&queue_slots[0], &lsys->axiom[i]);
    }

    char key[LSYSTEM_SYMBOL_SIZE];
    memset(key, 0, sizeof(key));
    uint32_t derivative_length;

    for (int i = 0; i < n; i++) {
        derivative_length = 0;
        uint8_t index_1 = i % 2;
        uint8_t index_2 = !index_1;
        while (!is_empty_queue(&queue_slots[index_1])) {
            char *char_ptr = (char *)dequeue(&queue_slots[index_1]);
            key[0] = *char_ptr;
            LsystemSuccessorRule *value = get(lsys->rules, key);
            // Checking if the production rule exists
            if (value) {
                derivative_length += value->length;
                for (int j = 0; j < value->length; j++) {
                    enqueue(&queue_slots[index_2], &value->successor[j]);
                }
            } else {
                enqueue(&queue_slots[index_2], char_ptr);
                derivative_length++;
            } 
        }
    }
    
    uint8_t result_index = n % 2;
    char *derivative = malloc(derivative_length + 1);
    if (!derivative) {
        while (!is_empty_queue(&queue_slots[result_index])) {
            dequeue(&queue_slots[result_index]);
        }
        return NULL;
    }

    derivative[derivative_length] = '\0';
    for (int i = 0; i < derivative_length; i++) {
        derivative[i] = *(char *)dequeue(&queue_slots[result_index]);
    }
    return derivative; 
}