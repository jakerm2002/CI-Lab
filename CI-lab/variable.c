/**************************************************************************
 * C S 429 EEL interpreter
 * 
 * variable.c - This file contains the skeleton of functions to be implemented
 * for EEL-2. When completed, it will contain the code to maintain a hashtable
 * for defined variables.
 * 
 * Work on it only after finishing EEL-0 and EEL-1.
 * 
 * Copyright (c) 2021. S. Chatterjee, X. Shen, T. Byrd. All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/ 

#include "ci.h"

table_t *var_table = NULL;
static char *bool_print[] = {"false", "true"};

void init_table(void) {
    var_table = (table_t *) calloc(1, sizeof(table_t));
    if (! var_table) {
        logging(LOG_FATAL, "failed to allocate table");
        return;
    }
    var_table->entries = (entry_t **) calloc(CAPACITY, sizeof(entry_t *));
    if (! var_table->entries) {
        free(var_table);
        logging(LOG_FATAL, "failed to allocate entries");
    }
    return;
}

void delete_entry(entry_t *eptr) {
    if (! eptr) return;
    if (eptr->type == STRING_TYPE) {
        free(eptr->val.sval);
    }
    free(eptr->id);
    free(eptr);
    return;
}

void delete_entries(entry_t *eptr) {
    if (! eptr) return;
    delete_entries(eptr->next);
    delete_entry(eptr);
    return;
}

void delete_table(void) {
    if (! var_table) return;

    for (int i = 0; i < CAPACITY; ++i) {
        delete_entries(var_table->entries[i]);
    }
    free(var_table->entries);
    free(var_table);
    return;
}

/* Pre-defined hash function to index variables by their names. */
unsigned long hash_function(char *s) {
    unsigned long i = 0;
    for (int j=0; s[j]; j++)
        i += s[j];
    return i % CAPACITY;
}

/* init_entry() - provided entry constructor
 * Parameters: Variable name, pointer to a node.
 * Return value: An allocated entry. */
entry_t * init_entry(char *id, node_t *nptr) {
    if (nptr == NULL) {
        logging(LOG_FATAL, "failed to allocate entry");
        return NULL;
    }
    entry_t *eptr = (entry_t *) calloc(1, sizeof(entry_t));
    if (! eptr) {
        logging(LOG_FATAL, "failed to allocate entry");
        return NULL;
    }
    eptr->id = (char *) malloc(strlen(id) + 1);
    if (! id) {
        logging(LOG_FATAL, "failed to allocate entry id");
        free(eptr);
        return NULL;
    }
    strcpy(eptr->id, id);
    eptr->type = nptr->type;
    if (eptr->type == STRING_TYPE) {
        // (eptr->val).sval = (char *) malloc(strlen(nptr->val.sval) + 1);
        (eptr->val).sval = (char *) calloc(1, strlen(nptr->val.sval) + 1);
        if (! eptr->val.sval) {
            logging(LOG_FATAL, "failed to allocate string");
            free(eptr->id);
            free(eptr);
            return NULL;
        }
        strcpy(eptr->val.sval, nptr->val.sval);
    } else {
        eptr->val.ival = nptr->val.ival;
    }
    return eptr;
}

/* put() - insert an entry into the hashtable or update the existing entry.
 * Use a linked list to handle collisions.
 * Parameters: Variable name, pointer to a node.
 * Return value: None.
 * Side effect: The entry is inserted into the hashtable, or is updated if
 * it already exists.
 * (STUDENT TODO) 
 */

void put(char *id, node_t *nptr) {

    //get the hash for this id
    int index = hash_function(id);

    // //if this linked list has not been intiialized yet
    // if (!var_table->entries[index]) {
    //     var_table->entries[index] = init_entry(id, nptr);
    //     var_table->entries[index]->next = NULL;
    // } else { //the value may be present, we must find it

    //     //we must traverse through all entries
    //     entry_t *current_node = var_table->entries[index];
    //     entry_t *prev_node = NULL;

    //     //while current node is not null
    //     while (current_node) {
    //         //if the id of this node matches our id
    //         if (*current_node->id == *id) {
    //             //we must replace this entry
    //             //free the memory!!
    //             entry_t *next_temp = NULL;
    //             if(current_node->next) {
    //                 next_temp = current_node->next;
    //             }

    //             entry_t *temp_node = init_entry(id, nptr);
                

    //             delete_entry(current_node);
    //             current_node = temp_node;
    //             current_node->next = next_temp;
    //             if(prev_node) {
    //                 prev_node->next = current_node;
    //             }

    //             //since we have found and replaced the variable, we are finished
    //             return;
    //         }

    //         //move on to the next node
    //         prev_node = current_node;
    //         current_node = current_node->next;
    //     }
    //     //we have reached the end, should append a new entry
    //     prev_node -> next = init_entry(id, nptr);
        
    // }

    //if this linked list has not been intiialized yet
    if (!var_table->entries[index]) {
        var_table->entries[index] = init_entry(id, nptr);
        var_table->entries[index]->next = NULL;
    } else { //the value may be present, we must find it

        //we must traverse through all entries
        entry_t *current_node = var_table->entries[index];
        entry_t *prev_node = NULL;

        //while current node is not null
        while (current_node) {
            //if the id of this node matches our id
            if (*current_node->id == *id) {
                //we must replace this entry
                //free the memory!!
                entry_t *next_temp = NULL;
                if(current_node->next) {
                    next_temp = current_node->next;
                }
                delete_entry(current_node);
                entry_t *temp_node = init_entry(id, nptr);
                *current_node = *temp_node;
                current_node->next = next_temp;
                if(prev_node) {
                    prev_node->next = current_node;
                }

                //since we have found and replaced the variable, we are finished
                return;
            }

            //move on to the next node
            prev_node = current_node;
            current_node = current_node->next;
        }
        //we have reached the end, should append a new entry
        prev_node -> next = init_entry(id, nptr);
        
    }
    return;
}

/* get() - search for an entry in the hashtable.
 * Parameter: Variable name.
 * Return value: Pointer to the matching entry, or NULL if not found.
 * (STUDENT TODO) 
 */
entry_t* get(char* id) {
    int index = hash_function(id);
    if (!var_table->entries[index]) {
        return NULL;
    } else {
        entry_t *current_node = var_table->entries[index];
        while(current_node) {
            if(*current_node->id == *id) {
                return current_node;
            }
            current_node = current_node->next; //next entry
        }
    }
    return NULL;
}

void print_entry(entry_t *eptr) {
    if (! eptr) return;
    switch (eptr->type) {
        case INT_TYPE:
            fprintf(outfile, "%s = %d; ", eptr->id, eptr->val.ival);
            break;
        case BOOL_TYPE:
            fprintf(outfile, "%s = %s; ", eptr->id, bool_print[eptr->val.bval]);
            break;
        case STRING_TYPE:
            fprintf(outfile, "%s = \"%s\"; ", eptr->id, eptr->val.sval);
            break;
        default:
            logging(LOG_ERROR, "unsupported entry type for printing");
            break;
    }
    print_entry(eptr->next);
    return;
}

void print_table(void) {
    if (! var_table) {
        logging(LOG_ERROR, "variable table doesn't exist");
        return;
    }
    fprintf(outfile, "\t");
    for (int i = 0; i < CAPACITY; ++i) {
        print_entry(var_table->entries[i]);
    }
    fprintf(outfile, "\n");
    return;
}
