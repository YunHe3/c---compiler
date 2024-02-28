#ifndef _SYNTAX_TREE
#define _SYNTAX_TREE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
typedef struct Type_* Type;

#define MAX_NAME_LENGTH 20

typedef enum Types {
    LEXICAL_TOKEN_NORMAL,
    LEXICAL_TOKEN_ID,
    LEXICAL_TOKEN_TYPE,
    LEXICAL_TOKEN_NUMBER,
    SYNTAX_TOKEN_NONEMPTY,
    SYNTAX_TOKEN_EMPTY,
} type_t;

typedef struct Node {
    type_t type;            // syntax and lexical type
    int lineNo;             // line number in source file
    char *tokenName;        // node's name
    int childNum;           // node's children node number
    struct Node **childs;   // node's children
    struct Node* par;       // node's parent

    char *decName;          // if the node is a variable, its name
    int isLHS;              // if the node is a variable and has a address
    Type semanticType;      // semantic type
} node_t;

typedef struct Tree {
    node_t root;
} tree_t;

void insert(node_t* par, node_t* child);
node_t* createNode(type_t type, int line, char *name, int childNum);
void setParent(node_t* child, node_t* par);
void printTree(node_t* root, int depth);
void cleanTree(node_t* root);

void semantic(node_t* root);

#endif