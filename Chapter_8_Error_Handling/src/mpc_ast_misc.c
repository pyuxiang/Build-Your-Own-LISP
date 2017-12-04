#include "mpc_ast_misc.h"

// Write a recursive function to compute the number of leaves of a tree.
long count_leaves(mpc_ast_t *node) {
    int i, result = 0;
    for (i = 0; i < node->children_num; i++) {
        result += count_leaves(node->children[i]);
    }
    return 1 + result;
}
