#include "syntax-tree.h"

static int hex2dec(char *hex) {
    int idx = 2, res = 0;

    for (; idx < strlen(hex); idx++) {
        res *= 16;
        if (hex[idx] >= '0' && hex[idx] <= '9')
            res += hex[idx] - '0';
        else if (hex[idx] >= 'a' && hex[idx] <= 'f')
            res += hex[idx] - 'a';
        else
            res += hex[idx] - 'A'; 
    }
    
    return res;
}

static int oct2dec(char *oct) {
    int idx = 1, res = 0;

    for (; idx < strlen(oct); idx++) {
        res *= 8;
        res += oct[idx] - '0';
    }

    return res;
}

node_t* createNode(type_t type, int line, char *name, int childNum) {
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->childNum = 0;
    node->childs = (node_t**)calloc(sizeof(node_t*), childNum);
    node->lineNo = line;
    node->tokenName = (char *)calloc(MAX_NAME_LENGTH, sizeof(char));
    node->decName = (char*)calloc(MAX_NAME_LENGTH, sizeof(char));
    node->type = type;
    node->isLHS = 0;

    strcpy(node->tokenName, name);
    node->tokenName[strlen(name)] = '\0';
    return node;
}

void insert(node_t* par, node_t *child) {
    // printf("Insert %s to %s\n", child->tokenName, par->tokenName);
    // fflush(stdout);

    par->childs[par->childNum++] = child;
    child->par = par;
    if(child->lineNo < par->lineNo)
        par->lineNo = child->lineNo;
}

void printTree(node_t *root, int depth) {
    if (root->type == SYNTAX_TOKEN_EMPTY)
        return;

    for (int i = 0; i < depth; i++)
        printf("  ");
    
    node_t *child;
    switch (root->type)
    {
    case LEXICAL_TOKEN_ID:
        printf("ID: %s\n", root->tokenName);
        break;
    
    case LEXICAL_TOKEN_NORMAL:
        printf("%s\n", root->tokenName);
        break;

    case LEXICAL_TOKEN_NUMBER:
        child = root->childs[0];
        assert(child->type == LEXICAL_TOKEN_NUMBER);
        
        if (!strcmp(root->tokenName, "INTD")) {
            printf("INT: %d\n", atoi(child->tokenName));
        } else if (!strcmp(root->tokenName, "INTO")) {
            printf("INT: %d\n", oct2dec(child->tokenName));
        } else if (!strcmp(root->tokenName, "INTH")) {
            printf("INT: %d\n", hex2dec(child->tokenName));
        } else {
            // float
            printf("FLOAT: %f\n", strtof(child->tokenName, NULL));
        }
        break;

    case LEXICAL_TOKEN_TYPE:
        child = root->childs[0];
        assert(child->type == LEXICAL_TOKEN_TYPE);
        printf("TYPE: %s\n", child->tokenName);
        break;
    
    case SYNTAX_TOKEN_NONEMPTY:
        printf("%s (%d)\n", root->tokenName, root->lineNo);
        for (int i = 0; i < root->childNum; i++) {
            printTree(root->childs[i], depth+1);
        }
        break;

    default:
        break;
    }
}

void cleanTree(node_t* root) {
    return;
}