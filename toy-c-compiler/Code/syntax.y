%{
    /* includes and macros */
    #include "syntax-tree.h"
    #include "lex.yy.c"
    
    int errorMarks[1000] = {0};
    int syntaxErrorFlag = 0;
    node_t* syntaxTreeRoot = NULL;

    void yyerror(const char *s) {
        syntaxErrorFlag = 1;
        if (!strcmp(s, "syntax error"))
            return;
        if (errorMarks[yylineno] == 1)
            return;
        printf("Error type B at Line %d: %s\n",
            yylineno, s);
        errorMarks[yylineno] = 1;
    }
%}

%locations
%start Program

/* declared types */

%union {
    struct Node* type_node;
    char* type_string;
}

/* declared tokens */
%token <type_node> TYPE_INT TYPE_FLOAT STRING 
%token <type_node> PLUS MINUS STAR DIV AND OR NOT ASSIGNOP
%token <type_node> BT LT BE LE EQ NE
%token <type_node> DOT COMMA SEMI LP RP LB RB LC RC NEWLINE
%token <type_node> STRUCT WHILE IF ELSE RETURN
%token <type_string> ID INTD INTO INTH FLOAT FLOATE
%token LEXICAL_ERROR

/* declared non-terminals */
%type <type_node> Program ExtDefList ExtDef ExtDecList
%type <type_node> Specifier Type StructSpecifier OptTag Tag
%type <type_node> VarDec FunDec VarList ParamDec
%type <type_node> CompSt StmtList Stmt
%type <type_node> DefList Def DecList Dec
%type <type_node> Exp Args Int Float

%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE

/* ops */
%right ASSIGNOP
%left OR
%left AND
%left NE EQ BE LE BT LT
%left PLUS MINUS
%left STAR DIV
%nonassoc UMINUS
%right NOT
%left DOT LB RB LP RP 

%%
/*-------------------------rules-------------------------*/

/* High-level Definitions */
Program : ExtDefList { 
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Program", 1); 
        insert($$, $1);
        
        if (syntaxErrorFlag == 1) {
            cleanTree($$);
            syntaxTreeRoot = NULL;
        } else {
            syntaxTreeRoot = $$;
        }
    }
    ;
ExtDefList : ExtDef ExtDefList { 
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "ExtDefList", 2); 
        insert($$, $1); 
        insert($$, $2); 
    }
    | { $$ = createNode(SYNTAX_TOKEN_EMPTY, (@$).first_line, "", 0); }  /* empty */
    ;
ExtDef : Specifier ExtDecList SEMI { 
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "ExtDef", 3); 
        $3 = createNode(LEXICAL_TOKEN_NORMAL, (@3).first_line, "SEMI", 0); 
        insert($$, $1); 
        insert($$, $2); 
        insert($$, $3); 
    }
    | Specifier ExtDecList error { yyerror("Missing \";\""); }
    | Specifier SEMI { 
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "ExtDef", 2);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "SEMI", 0); 
        insert($$, $1); 
        insert($$, $2); 
    }
    | Specifier error { yyerror("Missing \";\""); }
    | Specifier FunDec CompSt { 
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "ExtDef", 3); 
        insert($$, $1); 
        insert($$, $2); 
        insert($$, $3); 
    }
    | Specifier FunDec SEMI {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "ExtDef", 3);
        $3 = createNode(LEXICAL_TOKEN_NORMAL, (@3).first_line, "SEMI", 0); 
        insert($$, $1); 
        insert($$, $2); 
        insert($$, $3);
    }
    ;
ExtDecList : VarDec { 
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "ExtDecList", 1); 
        insert($$, $1); 
    }
    | VarDec COMMA ExtDecList { 
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "ExtDecList", 3); 
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "COMMA", 0); 
        insert($$, $1); 
        insert($$, $2); 
        insert($$, $3); 
    }

/* Specifiers */
Specifier : Type { 
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Specifier", 1); 
        insert($$, $1); 
    }
    | StructSpecifier { 
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Specifier", 1); 
        insert($$, $1); 
    }
    ;
Type : TYPE_INT { 
        $$ = createNode(LEXICAL_TOKEN_TYPE, (@$).first_line, "TYPE", 1); 
        $1 = createNode(LEXICAL_TOKEN_TYPE, (@1).first_line, "int", 0); 
        insert($$, $1); 
    }
    | TYPE_FLOAT { 
        $$ = createNode(LEXICAL_TOKEN_TYPE, (@$).first_line, "TYPE", 1); 
        $1 = createNode(LEXICAL_TOKEN_TYPE, (@1).first_line, "float", 0); 
        insert($$, $1); 
    }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC { 
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "StructSpecifier", 5);
        $1 = createNode(LEXICAL_TOKEN_NORMAL, (@1).first_line, "STRUCT", 0);
        $3 = createNode(LEXICAL_TOKEN_NORMAL, (@3).first_line, "LC", 0);
        $5 = createNode(LEXICAL_TOKEN_NORMAL, (@5).first_line, "RC", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
        insert($$, $5);
    }
    | STRUCT Tag {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "StructSpecifier", 2);
        $1 = createNode(LEXICAL_TOKEN_NORMAL, (@1).first_line, "STRUCT", 0);
        insert($$, $1);
        insert($$, $2);
    }
    ;
OptTag : ID { 
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "OptTag", 1);
        node_t* nodeId = createNode(LEXICAL_TOKEN_ID, (@1).first_line, $1, 0);
        insert($$, nodeId); 
    }
    | { $$ = createNode(SYNTAX_TOKEN_EMPTY, (@$).first_line, "", 0); }  /* empty */
    ;
Tag : ID { 
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Tag", 1);
        node_t* nodeId = createNode(LEXICAL_TOKEN_ID, (@1).first_line, $1, 0);
        insert($$, nodeId); 
    }
    ;

/* Declarators */
VarDec : ID {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "VarDec", 1);
        node_t* nodeId = createNode(LEXICAL_TOKEN_ID, (@1).first_line, $1, 0);
        insert($$, nodeId);  
    }
    | VarDec LB Int RB {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "VarDec", 4);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "LB", 0);
        $4 = createNode(LEXICAL_TOKEN_NORMAL, (@4).first_line, "RB", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
    }
    | VarDec LB Int error { yyerror("Missing \"]\""); }
    | VarDec LB error RB { yyerror("INT expected."); }
    ;
FunDec : ID LP VarList RP {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "FunDec", 4);
        node_t* nodeId = createNode(LEXICAL_TOKEN_ID, (@1).first_line, $1, 0);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "LP", 0);
        $4 = createNode(LEXICAL_TOKEN_NORMAL, (@4).first_line, "RP", 0);
        insert($$, nodeId);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
    }
    | ID LP VarList error { yyerror("Missing \")\""); }
    | ID LP RP {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "FunDec", 3);
        node_t* nodeId = createNode(LEXICAL_TOKEN_ID, (@1).first_line, $1, 0);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "LP", 0);
        $3 = createNode(LEXICAL_TOKEN_NORMAL, (@3).first_line, "RP", 0);
        insert($$, nodeId);
        insert($$, $2);
        insert($$, $3);
    }
    | ID LP error { yyerror("Missing \")\""); }
    ;
VarList : ParamDec COMMA VarList {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "VarList", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "COMMA", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | ParamDec {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "VarList", 1);
        insert($$, $1);
    }
    ;
ParamDec : Specifier VarDec {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "ParamDec", 2);
        insert($$, $1);
        insert($$, $2);
    }
    ;

/* Statements */
CompSt : LC DefList StmtList RC {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "CompSt", 4);
        $1 = createNode(LEXICAL_TOKEN_NORMAL, (@1).first_line, "LC", 0);
        $4 = createNode(LEXICAL_TOKEN_NORMAL, (@4).first_line, "RC", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
    }
    ;
StmtList : Stmt StmtList {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "StmtList", 2);
        insert($$, $1);
        insert($$, $2);
    }
    | { $$ = createNode(SYNTAX_TOKEN_EMPTY, (@$).first_line, "", 0); }  /* empty */
    ;
Stmt : Exp SEMI {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Stmt", 2);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "SEMI", 0);
        insert($$, $1);
        insert($$, $2);
    }
    | CompSt {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Stmt", 1);
        insert($$, $1);
    }
    | RETURN Exp SEMI {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Stmt", 3);
        $1 = createNode(LEXICAL_TOKEN_NORMAL, (@1).first_line, "RETURN", 0);
        $3 = createNode(LEXICAL_TOKEN_NORMAL, (@3).first_line, "SEMI", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | RETURN Exp error { yyerror("Missing \";\""); }
    | RETURN error SEMI { yyerror("Syntax error."); }
    | IF LP Exp RP Stmt %prec LOWER_THEN_ELSE{
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Stmt", 5);
        $1 = createNode(LEXICAL_TOKEN_NORMAL, (@1).first_line, "IF", 0);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "LP", 0);
        $4 = createNode(LEXICAL_TOKEN_NORMAL, (@4).first_line, "RP", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
        insert($$, $5);
    }
    | IF LP Exp error Stmt %prec LOWER_THEN_ELSE{ yyerror("Missing \")\""); }
    | IF LP Exp RP Stmt ELSE Stmt {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Stmt", 7);
        $1 = createNode(LEXICAL_TOKEN_NORMAL, (@1).first_line, "IF", 0);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "LP", 0);
        $4 = createNode(LEXICAL_TOKEN_NORMAL, (@4).first_line, "RP", 0);
        $6 = createNode(LEXICAL_TOKEN_NORMAL, (@6).first_line, "ELSE", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
        insert($$, $5);
        insert($$, $6);
        insert($$, $7);
    }
    | IF LP Exp RP error ELSE Stmt { yyerror("Expect statement between 'if' and 'else'"); }
    | IF LP Exp RP Stmt ELSE error { yyerror("Expect statement after 'else'"); }
    | IF LP Exp error Stmt ELSE Stmt { yyerror("Missing \")\""); }
    | WHILE LP Exp RP Stmt {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Stmt", 5);
        $1 = createNode(LEXICAL_TOKEN_NORMAL, (@1).first_line, "WHILE", 0);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "LP", 0);
        $4 = createNode(LEXICAL_TOKEN_NORMAL, (@4).first_line, "RP", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
        insert($$, $5);
    }
    | WHILE LP Exp error Stmt { yyerror("Missing \")\""); }
    | WHILE LP Exp RP error SEMI{ yyerror("No Stmt."); }
    ;

/* Local Definitions */
DefList : Def DefList {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "DefList", 2);
        insert($$, $1);
        insert($$, $2);
    }
    | { $$ = createNode(SYNTAX_TOKEN_EMPTY, (@$).first_line, "", 0); }  /* empty */
    ;
Def : Specifier DecList SEMI {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Def", 3);
        $3 = createNode(LEXICAL_TOKEN_NORMAL, (@3).first_line, "SEMI", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Specifier DecList error { yyerror("Missing \";\""); }
    | error DecList SEMI { yyerror("Illegal type specifier"); }
    | Specifier error SEMI { yyerror("Syntax error."); }
    ;
DecList : Dec {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "DecList", 1);
        insert($$, $1);
    }
    | Dec COMMA DecList {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "DecList", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "COMMA", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Dec error { yyerror("Syntax error"); }
    ;
Dec : VarDec {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Dec", 1);
        insert($$, $1);
    }
    | VarDec error { yyerror("Syntax error."); }
    | VarDec ASSIGNOP Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Dec", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "ASSIGNOP", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | VarDec ASSIGNOP Exp error { yyerror("Syntax error."); }
    ;

/* Expressions */
Op : ASSIGNOP
    | AND
    | OR
    | BE
    | LE
    | BT
    | LT
    | EQ
    | NE
    | PLUS
    | MINUS
    | STAR
    | DIV
Exp : Exp ASSIGNOP Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "ASSIGNOP", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp AND Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "AND", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp OR Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "OR", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp BE Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "RELOP", 1);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);

        node_t* be = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "BE", 0);
        insert($2, be);
    }
    | Exp LE Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "RELOP", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);

        node_t* le = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "LE", 0);
        insert($2, le);
    }
    | Exp BT Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "RELOP", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);

        node_t* bt = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "BT", 0);
        insert($2, bt);
    }
    | Exp LT Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "RELOP", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);

        node_t* lt = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "LT", 0);
        insert($2, lt);
    }
    | Exp EQ Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "RELOP", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);

        node_t* eq = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "EQ", 0);
        insert($2, eq);
    }
    | Exp NE Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "RELOP", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);

        node_t* ne = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "NE", 0);
        insert($2, ne);
    }
    | Exp PLUS Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "PLUS", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp MINUS Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "MINUS", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp STAR Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "STAR", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp DIV Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "DIV", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp Op error Exp { yyerror("Illegal opreation"); }
    | Exp error Op Exp { yyerror("Illegal opreation"); }
    | Exp Op error { yyerror("Illegal opreation"); }
    | Exp error { yyerror("Syntax error."); }
    | LP Exp RP {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $1 = createNode(LEXICAL_TOKEN_NORMAL, (@1).first_line, "LP", 0);
        $3 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "RP", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | LP Exp error { yyerror("Missing \")\""); }
    | MINUS Exp %prec UMINUS{
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 2);
        $1 = createNode(LEXICAL_TOKEN_NORMAL, (@1).first_line, "MINUS", 0);
        insert($$, $1);
        insert($$, $2);
    }
    | NOT Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 2);
        $1 = createNode(LEXICAL_TOKEN_NORMAL, (@1).first_line, "NOT", 0);
        insert($$, $1);
        insert($$, $2);
    }
    | ID LP Args RP {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 4);
        node_t* nodeId = createNode(LEXICAL_TOKEN_ID, (@1).first_line, $1, 0);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "LP", 0);
        $4 = createNode(LEXICAL_TOKEN_NORMAL, (@4).first_line, "RP", 0);
        insert($$, nodeId);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
    }
    | ID LP Args error { yyerror("Missing \")\""); }
    | ID LP RP {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        node_t* nodeId = createNode(LEXICAL_TOKEN_ID, (@1).first_line, $1, 0);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "LP", 0);
        $3 = createNode(LEXICAL_TOKEN_NORMAL, (@3).first_line, "RP", 0);
        insert($$, nodeId);
        insert($$, $2);
        insert($$, $3);
    }
    | ID LP error { yyerror("Missing \")\""); }
    | Exp LB Exp RB {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 4);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "LB", 0);
        $4 = createNode(LEXICAL_TOKEN_NORMAL, (@4).first_line, "RB", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
    }
    | Exp LB Exp error { yyerror("Missing \"]\""); }
    | Exp DOT ID {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "DOT", 0);
        node_t* nodeId = createNode(LEXICAL_TOKEN_ID, (@3).first_line, $3, 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, nodeId);
    }
    | ID {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 1);
        node_t* nodeId = createNode(LEXICAL_TOKEN_ID, (@1).first_line, $1, 0);
        insert($$, nodeId);
    }
    | Int {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 1);
        insert($$, $1);
    }
    | Float {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Exp", 1);
        insert($$, $1);
    }
    ;
Args : Exp COMMA Args {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Args", 3);
        $2 = createNode(LEXICAL_TOKEN_NORMAL, (@2).first_line, "COMMA", 0);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp {
        $$ = createNode(SYNTAX_TOKEN_NONEMPTY, (@$).first_line, "Args", 1);
        insert($$, $1);
    }
    ;
Int : INTD {
        $$ = createNode(LEXICAL_TOKEN_NUMBER, (@$).first_line, "INTD", 1);
        node_t* nodeInt = createNode(LEXICAL_TOKEN_NUMBER, (@1).first_line, $1, 0);
        insert($$, nodeInt);
    }
    | INTH {
        $$ = createNode(LEXICAL_TOKEN_NUMBER, (@$).first_line, "INTH", 1);
        node_t* nodeInt = createNode(LEXICAL_TOKEN_NUMBER, (@1).first_line, $1, 0);
        insert($$, nodeInt);
    }
    | INTO {
        $$ = createNode(LEXICAL_TOKEN_NUMBER, (@$).first_line, "INTO", 1);
        node_t* nodeInt = createNode(LEXICAL_TOKEN_NUMBER, (@1).first_line, $1, 0);
        insert($$, nodeInt);
    }
    ;

Float : FLOAT {
        $$ = createNode(LEXICAL_TOKEN_NUMBER, (@$).first_line, "FLOAT", 1);
        node_t* nodeFloat = createNode(LEXICAL_TOKEN_NUMBER, (@1).first_line, $1, 0);
        insert($$, nodeFloat);
    }
    | FLOATE {
        $$ = createNode(LEXICAL_TOKEN_NUMBER, (@$).first_line, "FLOATE", 1);
        node_t* nodeFloat = createNode(LEXICAL_TOKEN_NUMBER, (@1).first_line, $1, 0);
        insert($$, nodeFloat);
    }
    ;

/* others */

%%
/*--------------------user subroutines--------------------*/
