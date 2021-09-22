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
    if(!nptr) {
        return NO_TYPE;
    } else if (nptr->tok == TOK_ID) {
        //get the type of entry_t from the variable name
        
        if (get(nptr->val.sval)) {
            nptr->type = get(nptr->val.sval) -> type;
        } else { //undefined var
            handle_error(ERR_UNDEFINED);
        }

        return nptr->type;
    }
    else if(nptr->node_type == NT_LEAF) {
        return nptr->type;
    } else {
        //traverses further down left subtree
        type_t left_type;
        type_t right_type;
        type_t third_type;

        left_type = find_node_type(nptr->children[0]);
        right_type = find_node_type(nptr->children[1]);
        third_type = find_node_type(nptr->children[2]);

        if(nptr->tok == TOK_PLUS || nptr->tok == TOK_BMINUS || nptr->tok == TOK_TIMES || nptr->tok == TOK_DIV || nptr->tok == TOK_MOD) {
            // printf("token is either plus or minus!\n");
            if ((left_type == right_type) && (left_type == INT_TYPE)) {
                // printf("conndition met!\n");
                nptr->type = INT_TYPE;
                return nptr->type;
            } else if ((left_type == right_type) && left_type == STRING_TYPE && nptr->tok == TOK_PLUS) {
                nptr->type = STRING_TYPE;
                return nptr->type;
            } else if (left_type == STRING_TYPE && right_type == INT_TYPE && nptr->tok == TOK_TIMES) {
                nptr->type = STRING_TYPE;
                return nptr->type;
            }
        } else if (nptr->tok == TOK_LT || nptr->tok == TOK_GT || nptr->tok == TOK_EQ) {
            if ((left_type == right_type) && (left_type == INT_TYPE)) {                // printf("conndition met!\n");
                nptr->type = INT_TYPE;
                return nptr->type;
            } else if ((left_type == right_type) && (left_type == STRING_TYPE)) {
                nptr->type = BOOL_TYPE;
                return nptr->type;
            }
        } else if (nptr->tok == TOK_AND || nptr->tok == TOK_OR) {
            if ((left_type == right_type) && (left_type == BOOL_TYPE)) {
                nptr->type = BOOL_TYPE;
                return nptr->type;
            }
        } 
        else if (nptr->tok == TOK_QUESTION) {
            if (right_type == third_type) {
                if (right_type == INT_TYPE) {
                    nptr->type = INT_TYPE;
                    return nptr->type;
                } else if (right_type == BOOL_TYPE) {
                    nptr->type = BOOL_TYPE;
                    return nptr->type;
                } else if (right_type == STRING_TYPE) {
                    nptr->type = STRING_TYPE;
                    return nptr->type;
                }
            }
        } 
        else if (nptr->tok == TOK_UMINUS) {
            if (left_type == INT_TYPE) {
                nptr->type = INT_TYPE;
                return nptr->type;
            } else if (left_type == STRING_TYPE) {
                nptr-> type = STRING_TYPE;
                return nptr->type;
            }
        } else if (nptr->tok == TOK_NOT) {
            if (left_type == BOOL_TYPE) {
                nptr->type = BOOL_TYPE;
                return nptr->type;
            }
        } else if (nptr->tok == TOK_IDENTITY) {
            nptr->type = left_type;
            return nptr->type;
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
    if (nptr->tok == TOK_ID && nptr->node_type == NT_LEAF) {
        nptr->type = get(nptr->val.sval)->type;
        nptr->val = (get(nptr->val.sval)->val);
        return nptr->val;
    } else if(nptr->node_type == NT_LEAF) {
        return nptr->val;
    }
    //TERNARY OPERATOR
    else if (nptr->tok == TOK_QUESTION) {
        value_t left_val;

        if (nptr->children[0]) {
            left_val = calculate_value(nptr->children[0]);
        }
        
        if (left_val.bval) {
            value_t right_val;
            if (nptr->children[1]) {
                right_val = calculate_value(nptr->children[1]);
            }
            if (nptr->type == INT_TYPE) {
                nptr->tok = TOK_NUM;
                nptr->val.ival = right_val.ival;
                return nptr->val;
            } else if (nptr->type == BOOL_TYPE) {
                nptr->tok = (right_val.bval) ? TOK_TRUE : TOK_FALSE;
                nptr->val.ival = right_val.bval;
                return nptr->val;
            } else if (nptr->type == STRING_TYPE) {
                nptr->tok = TOK_STR;
                nptr->val.sval = right_val.sval;
                return nptr->val;
            }
        } else {
            value_t third_val;
            if (nptr->children[2]) {
                third_val = calculate_value(nptr->children[2]);
            }
            if (nptr->type == INT_TYPE) {
                nptr->tok = TOK_NUM;
                nptr->val.ival = third_val.ival;
                return nptr->val;
            } else if (nptr->type == BOOL_TYPE) {
                nptr->tok = (third_val.bval) ? TOK_TRUE : TOK_FALSE;
                nptr->val.ival = third_val.bval;
                return nptr->val;
            } else if (nptr->type == STRING_TYPE) {
                nptr->tok = TOK_STR;
                nptr->val.sval = third_val.sval;
                return nptr->val;
            }
        }

        handle_error(ERR_TYPE);
        return nptr->val;

    }  //END OF TERNARY OPERATOR
    else {
        //traverses further down left subtree
        value_t left_val;
        value_t right_val;
        // value_t third_val;

        // left_val = calculate_value(nptr->children[0]);
        // right_val = calculate_value(nptr->children[1]);
        // third_val = calculate_value(nptr->children[2]);

        if (nptr->children[0]) {
            left_val = calculate_value(nptr->children[0]);
        }
        if (nptr->children[1]) {
            right_val = calculate_value(nptr->children[1]);
        }
        // if (nptr->children[2]) {
        //     third_val = calculate_value(nptr->children[2]);
        // }

        //ADD INTEGERS
        if (nptr->tok == TOK_IDENTITY) {
            nptr->val = left_val;
            return nptr->val;
        }
        if(nptr->tok == TOK_PLUS && nptr->type == INT_TYPE) {
            nptr->tok = TOK_NUM;
            nptr->val.ival = (left_val.ival) + (right_val.ival);
            return nptr->val;
        }
        //SUBTRACT INTEGERS
        else if (nptr->tok == TOK_BMINUS && nptr->type == INT_TYPE) {
            nptr->tok = TOK_NUM;
            nptr->val.ival = (left_val.ival) - (right_val.ival);
            return nptr->val;
        }
        //MULTPLY INTEGERS
        else if (nptr->tok == TOK_TIMES && nptr->type == INT_TYPE) {
            nptr->tok = TOK_NUM;
            nptr->val.ival = (left_val.ival) * (right_val.ival);
            return nptr->val;
        }
        //DIVDE INTEGERS
        else if (nptr->tok == TOK_DIV && nptr->type == INT_TYPE) {
            nptr->tok = TOK_NUM;
            if (right_val.ival == 0) {
                handle_error(ERR_EVAL);
            } else {
                nptr->val.ival = (left_val.ival) / (right_val.ival);
            }
            return nptr->val;
        }
        //MODULO INTEGERS
        else if (nptr->tok == TOK_MOD && nptr->type == INT_TYPE) {
            nptr->tok = TOK_NUM;
            nptr->tok = TOK_NUM;
            if (right_val.ival == 0) {
                handle_error(ERR_EVAL);
            } else {
                nptr->val.ival = (left_val.ival) % (right_val.ival);
            }
            return nptr->val;
        }
        //COMPARE > INTEGERS/STRINGS
        else if (nptr->tok == TOK_GT) {
            if (nptr->type == INT_TYPE) {
                bool result = (left_val.ival) > (right_val.ival);
                if (result == true) {
                    nptr->tok = TOK_TRUE;
                    nptr->val.bval = true;
                } else {
                    nptr->tok = TOK_FALSE;
                    nptr->val.bval = false;
                }
                return nptr->val;
            } else if (nptr->type == BOOL_TYPE) {
                int result = strcmp(left_val.sval, right_val.sval);
                if (result > 0) {
                    // nptr->tok = TOK_TRUE;
                    nptr->val.bval = true;
                } else {
                    nptr->tok = TOK_FALSE;
                    nptr->val.bval = false;
                }
                return nptr->val;
            }
        }
        //COMPARE < INTEGERS/STRINGS
        else if (nptr->tok == TOK_LT) {	
            if (nptr->type == INT_TYPE) {	
                bool result = (left_val.ival) < (right_val.ival);	
                if (result) {	
                    nptr->tok = TOK_TRUE;	
                    nptr->val.bval = true;	
                } else {	
                    nptr->tok = TOK_FALSE;	
                    nptr->val.bval = false;	
                }	
                return nptr->val;	
            } else if (nptr->type == BOOL_TYPE) {	
                int result = strcmp(left_val.sval, right_val.sval);	
                if (result < 0) {	
                    nptr->tok = TOK_TRUE;	
                    nptr->val.bval = true;	
                } else {	
                    nptr->tok = TOK_FALSE;	
                    nptr->val.bval = false;	
                }	
                return nptr->val;	
            }	
        }
        //COMPARE == (~) INTEGERS AND STRINGS
        else if (nptr->tok == TOK_EQ) {
            if (nptr->type == INT_TYPE) {
                bool result = (left_val.ival) == (right_val.ival);
                if (result) {
                    nptr->tok = TOK_TRUE;
                } else {
                    nptr->tok = TOK_FALSE;
                }
                nptr->val.bval = result;
                return nptr->val;
            } else if (nptr->type == BOOL_TYPE) {
                int result = strcmp(left_val.sval, right_val.sval);
                if (result == 0) {
                    nptr->tok = TOK_TRUE;
                    nptr->val.bval = true;
                } else {
                    nptr->tok = TOK_FALSE;
                    nptr->val.bval = false;
                }
                return nptr->val;
            }
        }
        //COMPARE & BOOLEANS
        else if (nptr->tok == TOK_AND && nptr->type == BOOL_TYPE) {
            bool result = (left_val.bval && right_val.ival);
            if (result)
                nptr->tok = TOK_TRUE;
            else 
                nptr->tok = TOK_FALSE;
            nptr->val.bval = result;
            return nptr->val;
        }
        //COMPARE | BOOLEANS
        else if (nptr->tok == TOK_OR && nptr->type == BOOL_TYPE) {
            bool result = (left_val.bval || right_val.ival);
            if (result)
                nptr->tok = TOK_TRUE;
            else 
                nptr->tok = TOK_FALSE;
            nptr->val.bval = result;
            return nptr->val;
        }
        //ADD/CONCATENATE STRINGS
        else if(nptr->tok == TOK_PLUS && nptr->type == STRING_TYPE) {
            nptr->tok = TOK_STR;
            char *string_holder = (char *) malloc(sizeof(left_val.sval) + sizeof(right_val.sval) + 1);
            strcpy(string_holder, left_val.sval);
            strcat(string_holder, right_val.sval);
            nptr->val.sval = string_holder;
            return nptr->val;
        }
        //MULTIPLY STRINGS
        else if(nptr->tok == TOK_TIMES && nptr->type == STRING_TYPE) {
            int repeat = right_val.ival;
            char *string_holder = (char *) calloc((sizeof(left_val.sval) * repeat) + 1, 1);
            nptr->tok = TOK_STR;
            for (int i = 0; i < repeat; ++i) {
                strcat(string_holder, left_val.sval);
            }
            nptr->val.sval = string_holder;
            return nptr->val;
        }
        //NEGATION OPERATOR
        else if (nptr->tok == TOK_UMINUS) {
            if (nptr->type == INT_TYPE) {
                nptr->tok = TOK_NUM;
                nptr->val.ival = (-1) * left_val.ival;
                return nptr->val;
            } else if (nptr->type == STRING_TYPE) {
                nptr->tok = TOK_STR;
                nptr->val.sval = strrev(left_val.sval);
                return nptr->val;
            }
        } else if (nptr->tok == TOK_NOT) {
            if (nptr->type == BOOL_TYPE) {
                nptr->val.bval = !left_val.bval;
                if (!left_val.bval) {
                    nptr->tok = TOK_TRUE;
                } else {
                    nptr->tok = TOK_FALSE;
                }
                return nptr->val;
            }
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

    //check for lone variable - piazza post @569
    if(!nptr->children[1] && nptr->children[0] && nptr->children[0]->type == ID_TYPE) {
        //assign the root the value of our variablee
        // nptr->val.sval = nptr->children[0]->val.sval;
        nptr->val = get(nptr->children[0]->val.sval)->val;

        //assign the root node the type of the variable
        nptr->type = get(nptr->children[0]->val.sval)->type;
    }
    // check for assignment
    if(nptr->type == ID_TYPE) {
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
    print_tree(nptr);
    return;
}

/* strrev() - helper function to reverse a given string 
 * Parameter: The string to reverse.
 * Return value: The reversed string. The input string is not modified.
 * (STUDENT TODO)
 */

char *strrev(char *str) {
    char *string_holder = (char *) calloc(strlen(str) + 1, 1);

    char *start_pos = str; //loc of string mem address
    char *end_pos = (strlen(str) - 1) + start_pos;
    for (int i = 0; i < strlen(str); ++i) {
        *(string_holder + i) = *(end_pos - i);
    }
    return string_holder;
}