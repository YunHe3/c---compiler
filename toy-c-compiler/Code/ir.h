#ifndef _IR
#define _IR

#include "syntax-tree.h"
#include "semantic.h"
#include <stdarg.h>

typedef struct _Operand* Operand;
typedef struct _InterCode* InterCode;
typedef struct _CodeList* CodeList;
typedef struct _ArgList* ArgList;
typedef struct _Variable* Variable;

void print_IR(CodeList ir, FILE* file);

ArgList new_arglist();

Operand new_operand();
Operand new_temp();
Operand new_label();
Operand new_const(int value);
Operand new_addresss();
int isConst(node_t*);
int isVar(node_t*);
Operand getConst(node_t*);
Operand getVar(node_t*);

int getTypeSize(Type type);

Variable new_var(char* name);
Variable getVarByName(char*);

CodeList interCode(node_t *root);
CodeList label2CodeList(Operand);
CodeList new_CodeList();
CodeList concatenate(int num, ...);
CodeList createBinOp(Operand des, Operand op1, Operand op2);
CodeList createCodeRet(Operand op);
CodeList createCodeGOTO(Operand label);
CodeList createCodeCondGOTO(Operand label, Operand op1, Operand op2, char* relop);
CodeList createCodeAssign(Operand left, Operand right);
CodeList createCodeDeref(Operand des, Operand src);
CodeList createCodeRef(Operand def, Operand src);
CodeList createCodeCall(char *funcName, Operand des);
CodeList createCodeArg(Operand, int);
CodeList createCodeRead(Operand);
CodeList createCodeWrite(Operand);
CodeList createCodeFunc(char *func);
CodeList createCodeParam(Operand, int);
CodeList createCodeDec(Operand, int);

CodeList translate_ExtDef(node_t*);
CodeList translate_FunDec(node_t*);
CodeList translate_CompSt(node_t*);
CodeList translate_DefList(node_t*);
CodeList translate_StmtList(node_t*);
CodeList translate_Stmt(node_t*);
CodeList translate_Exp(node_t*, Operand);
CodeList translate_Args(node_t*, ArgList*);
CodeList translate_Cond(node_t*, Operand, Operand);
CodeList translate_Array(node_t*, Operand);
CodeList translate_Structure(node_t*, Operand);
CodeList getOffsetByIdx(Type type, Operand idx, Operand place);
CodeList getOffsetOfField(Type type, char* fieldName, Operand place);

char* getRELOP(node_t* exp);
char *op2String(Operand op);

struct _Operand{ 
    enum{
        Em_VARIABLE, // 变量(var)
        Em_CONSTANT, // 常量(#1)
        Em_ADDRESS,  // 地址(&var)
        Em_LABEL,    // 标签(LABEL label1:) 
        Em_ARR,      // 数组(arr[2])
        Em_STRUCT,   // 结构体(struct Point p) 
        Em_TEMP,     // 临时变量(t1)
    } kind; 

    union{
        int varno;   // 变量定义的序号
        int labelno; // 标签序号
        int val;     // 操作数的值
        int tempno;  // 临时变量序号(唯一性)
    } u;

    Type type;       // 计算数组、结构体占用size
    int para;        // 标识函数参数
    char *name;
};

struct _InterCode{
    enum{
        IC_ASSIGN, 
        IC_LABEL, 
        IC_PLUS,    // op1 + op2
        IC_MINUS,   // op1 - op2
        IC_MULTI,   // op1 * op2
        IC_DIV,     // op1 / op2
        IC_CALL,
        IC_PARAM, 
        IC_READ,
        IC_WRITE, 
        IC_RETURN,
        IC_FUNC,
        IC_JUMP,
        IC_CONDJUMP,
        IC_DEREF,
        IC_REF,
        IC_ARG,
        IC_DEC,
    } kind;
    union{
        Operand op;
        Operand label;
        char *func;
        struct {Operand left, right; } assign;
        struct {Operand result, op1, op2; } binop; //三地址代码
        struct {Operand result; char *func; } call;
        struct {Operand label; } jump; 
        struct {Operand left, right; } deref; // 解引用
        struct {Operand left, right; } ref; // 取地址
        struct {Operand op1, op2, label; char* relop; } condjump; //条件跳转
        struct {Operand op1; int size; } dec;
        struct {Operand param; int number; } param; 
        struct {Operand arg; int number; } arg;
    }u; 
};

// 中间代码列表实现方式
struct _CodeList{ 
    InterCode code;     
    CodeList prev, next;
};

// 参数列表实现方式
struct _ArgList{ 
    Operand args;
    ArgList next;
};

// 变量的实现方式
struct _Variable{ 
    char* name; 
    Operand op;
    Variable next;
};

#endif