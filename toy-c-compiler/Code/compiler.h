#ifndef _COMPILER
#define COMPILER

#include "syntax-tree.h"
#include "semantic.h"
#include "ir.h"
#include "assemble.h"

// lexical
extern int lexicalErrorFlag;
int yyrestart(FILE *);
int yyparse();
int yydebug;

// syntax
extern int syntaxErrorFlag;
extern node_t* syntaxTreeRoot;

// semantic
void semantic(node_t* program);
extern SymTable structureTable;
extern SymTable varTable;
extern SymTable funcTable;

#endif
