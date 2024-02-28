#ifndef _SEMANTIC
#define _SEMANTIC

#include "syntax-tree.h"

#define TABLESIZE 0x3fff
#define MAXSTACKSIZE 1000
#define BUFSIZE 100
#define MAX_DECLIST_SIZE 10
#define MAX_NAME_LENGTH 20
#define MAX_LINE 1000
extern int seed;

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct SymTable_* SymTable;
typedef struct Item_* Item;

void semantic(node_t*);
void err(char *msg, int errType, int lineno);
void dec_undef_check(SymTable);
void randomNameGenerator(char* buf);
int isLHS(node_t* exp);
void paramsHandler(Item funcItem);

Type Program(node_t*);
Type ExtDefList(node_t*);
Type Specifier(node_t*);
Type* Def(node_t*);
Type ExtDef(node_t*);
Type ExtDecList(node_t*, Type);
Type Dec(node_t*, Type);
Item FunDec(node_t*, Type, int status);
Type VarDec(node_t*, Type, int isParam);
void VarList(node_t*, Item);
Type CompSt(node_t*, Type);
Type Stmt(node_t*, Type);
Type Exp(node_t*);

Type createArrayType(Type, int);
Type createFloatType();
Type createIntType();
Type createStructureType();
int typecmp(Type, Type);
int getTypeSize(Type type);
FieldList createFieldList();
int fieldcmp(FieldList, FieldList);

SymTable createTable();
void cleanTable(SymTable);
int insertItem(SymTable, Item);
Item findItem(SymTable, char*);
Item createItem();

enum { INT, FLOAT } basicType;
struct Type_
{
    enum { BASIC, ARRAY, STRUCTURE } kind;
    union
    {
        // 基本类型
        int basic;
        // 数组类型信息包括元素类型与数组大小构成 
        struct { Type elem; int size; } array; 
        // 结构体类型信息是一个链表
        FieldList structure;
    } u;
    Type parType; // parent type, use when nested type
};

struct FieldList_
{
    char* name; // 域的名字 
    Type type; // 域的类型
    FieldList tail; // 下一个域
    int lineNo; // the line number of field
};

typedef enum { VAR, FUNC } Kind;
struct Item_
{
    Kind kind;
    char* name;         // symbol name
    Type type;          // symbol type (ret type for funcs)
    unsigned int addr;  // symbol address
    int lineNo;         // symbol line number
    union{ 
        FieldList params; // params for function item
        Item funcItem;    // function if the item stand for a param
        } func;   
    Item tail;          // next symbol in the same slot in hashing table
    enum {DEF, DEC, } funcStatus; // mark if the function already defined
};

struct SymTable_
{
    Kind kind;
    Item items[TABLESIZE];
};

extern SymTable structureTable;
extern SymTable varTable;
extern SymTable funcTable;

#endif