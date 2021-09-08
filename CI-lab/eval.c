/**************************************************************************
 * C S 429 EEL interpreter
 * 
 * eval.c - This file contains the skeleton of functions to be implemented by
 * you. When completed, it will contain the code used to evaluate an expression
 * based on its AST.
 * 
 * Copyright (c) 2021. S. Chatterjee, X. Shen, T. Byrd. All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/ 

#include "ci.h"

extern bool is_binop(token_t);
extern bool is_unop(token_t);
char *strrev(char *str);
static type_t find_node_type(node_t *nptr);
static value_t calculate_value(node_t *nptr);

/* infer_type() - set the type of a non-root node based on the types of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated.
 * (STUDENT TODO)
 */

static void infer_type(node_t *nptr) {

    //we must do a post-order traversal of the tree here
    //we must visit EVERY INTERNAL NODE

    
    //based on what the children are, what is the type of calculated value i 
    //should expect here?
    //do not actually evaluate anything in this method

    if (nptr != NULL) {
        find_node_type(nptr);
    }

    return;
}

static type_t find_node_type(node_t *nptr) {
    if(nptr->node_type == NT_LEAF) {
        // printf("returning %d", nptr->type);
        return nptr->type;
    } else {
        //traverses further down left subtree
        type_t left_type;
        type_t right_type;

        left_type = find_node_type(nptr->children[0]);
        right_type = find_node_type(nptr->children[1]);

        if(nptr->tok == TOK_PLUS || nptr->tok == TOK_BMINUS) {
            // printf("token is either plus or minus!\n");
            if ((left_type == right_type) && (left_type == INT_TYPE)) {
                // printf("conndition met!\n");
                nptr->type = INT_TYPE;
                return nptr->type;
            }
        }

        handle_error(ERR_TYPE);
        return nptr->type;
    }
}

/* infer_root() - set the type of the root node based on the types of children
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated. 
 */

static void infer_root(node_t *nptr) {
    if (nptr == NULL) return;
    // check running status
    if (terminate || ignore_input) return;

    // check for assignment
    if (nptr->type == ID_TYPE) {
        infer_type(nptr->children[1]);
    } else {
        for (int i = 0; i < 3; ++i) {
            infer_type(nptr->children[i]);
        }
        if (nptr->children[0] == NULL) {
            logging(LOG_ERROR, "failed to find child node");
            return;
        }
        nptr->type = nptr->children[0]->type;
    }
    return;
}

/* eval_node() - set the value of a non-root node based on the values of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The val field of the node is updated.
 * (STUDENT TODO) 
 */

static void eval_node(node_t *nptr) {
    if (nptr != NULL) {
        calculate_value(nptr);
    }
    return ;
}

static value_t calculate_value(node_t *nptr) {
    if(nptr->node_type == NT_LEAF) {
        return nptr->val;
    } else {
        //traverses further down left subtree
        value_t left_val;
        value_t right_val;

        left_val = calculate_value(nptr->children[0]);
        right_val = calculate_value(nptr->children[1]);

        if(nptr->tok == TOK_PLUS && nptr->type == INT_TYPE) {
            nptr->tok = TOK_NUM;
            nptr->val.ival = (left_val.ival) + (right_val.ival);
            return nptr->val;
        }

        handle_error(ERR_TYPE);
        return nptr->val;
    }
}



/* eval_root() - set the value of the root node based on the values of children 
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The val dield of the node is updated. 
 */

void eval_root(node_t *nptr) {
    if (nptr == NULL) return;
    // check running status
    if (terminate || ignore_input) return;

    // check for assignment
    if (nptr->type == ID_TYPE) {
        eval_node(nptr->children[1]);
        if (terminate || ignore_input) return;
        
        if (nptr->children[0] == NULL) {
            logging(LOG_ERROR, "failed to find child node");
            return;
        }
        put(nptr->children[0]->val.sval, nptr->children[1]);
        return;
    }

    for (int i = 0; i < 2; ++i) {
        eval_node(nptr->children[i]);
    }
    if (terminate || ignore_input) return;
    
    if (nptr->type == STRING_TYPE) {
        (nptr->val).sval = (char *) malloc(strlen(nptr->children[0]->val.sval) + 1);
        if (! nptr->val.sval) {
            logging(LOG_FATAL, "failed to allocate string");
            return;
        }
        strcpy(nptr->val.sval, nptr->children[0]->val.sval);
    } else {
        nptr->val.ival = nptr->children[0]->val.ival;
    }
    return;
}

/* infer_and_eval() - wrapper for calling infer() and eval() 
 * Parameter: A pointer to a root node.
 * Return value: none.
 * Side effect: The type and val fields of the node are updated. 
 */

void infer_and_eval(node_t *nptr) {
    infer_root(nptr);
    eval_root(nptr);
    return;
}

/* strrev() - helper function to reverse a given string 
 * Parameter: The string to reverse.
 * Return value: The reversed string. The input string is not modified.
 * (STUDENT TODO)
 */

char *strrev(char *str) {
    return NULL;
}