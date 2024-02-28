/*-------- Include, Macros and Defs --------*/
#ifndef _ASSEMBLE
#define _ASSEMBLE

#include "ir.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))

typedef struct _Instruction* Instruction;
typedef struct _InstrList* InstrList;
typedef struct _Register* Register;
typedef struct _Op2PosList* Op2PosList;
typedef struct _REGs* REGs;
typedef struct _ActivationRecord* ActivationRecord;

/*---------- Function Declaration ----------*/

// translate ir code to mips32 instructions list
InstrList translateIR2MIPS(CodeList ir); 

// translation for each ic
InstrList IC2Instr(InterCode ic);
InstrList createInstrArithmetic(InterCode ic);
InstrList createInstrJump(InterCode ic);
InstrList createInstrLoad(Operand src, Register dst);
InstrList createInstrStore(Operand dst, Register src);
InstrList createInstrLabel(InterCode ic);
InstrList createInstrAssign(InterCode ic);
InstrList createInstrMflo(Register rs);
InstrList createInstrFunc(InterCode ic);
InstrList createInstrCall(InterCode ic);
InstrList createInstrRet(InterCode ic);
InstrList createInstrRef(InterCode ic);
InstrList createInstrDeref(InterCode ic);
InstrList createInstrParam(InterCode ic);
InstrList createInstrArg(InterCode ic);

InstrList createInstrRead(InterCode ic);
InstrList createInstrExit();
InstrList createInstrWrite(InterCode ic);

// useful instr
InstrList pushValue();
InstrList pushRegister();

// get registers for InterCode
// InstrList getReg(InterCode ic);
InstrList getReg(int num, ...);

// print to file
void print_Instr(InstrList head, FILE* fp);

// return the head of lits (can be null)
InstrList instrConcatenate(int num, ...);
void instrConnect(InstrList i1, InstrList i2);

// get register by operand (have been alloc by function getReg)
Register getRegByOp(Operand op);
int getOffsetByOp(Operand op);
Op2PosList getPosByOp(Operand op);

// function 
InstrList funcPrologue();
InstrList funcEpilogue();

/*------------- Data Structure -------------*/ 
InstrList newInstrList();
Instruction newInstruction();
Op2PosList newOp2PosList(Operand op, Register reg, int offset);
void freeOp2PosList(Op2PosList head); // free a list when a function return
REGs newREGs();
Register newReigster(int rno, char *name);

struct _InstrList {
    Instruction instr;
    InstrList next, pre;
};

struct _Instruction{
    enum {
        INSTR_ADD,      // RD = RS + RT
        INSTR_ADDI,     // RD = RS + CONST16
        INSTR_SUB,      // RD = RS - RT
        INSTR_MUL,      // RD = RS x RT
        INSTR_DIV,      // Lo = RS / RT; Hi = RS MOD RT

        INSTR_LW,       // RD = MEM32(RS+OFF16)
        INSTR_SW,       // MEM32(RT + OFF16) = RS

        INSTR_LA,       // RD = addr
        INSTR_LI,       // RD = IMM32
        INSTR_MOVE,     // RD = RS
        INSTR_MFLO,     // RD = Lo
        INSTR_NEGU,     // RD = -RS

        INSTR_J,        // PC = x Label
        INSTR_JAL,      // RA = PC + 8; PC = f Label
        INSTR_JR,       // PC = RS

        INSTR_BEQ,      // IF RS = RT, GOTO label
        INSTR_BNE,      // IF RS = RT
        INSTR_BGT,      // IF RS > RT
        INSTR_BLT,      // IF RS < RT
        INSTR_BGE,      // IF RS >= RT
        INSTR_BLE,      // IF RS <= RT

        INSTR_LABEL,    // x Label: 
        INSTR_FUNC,     // func: 
        
        INSTR_SYSCALL,  //syscall
    } kind;
    union {
        struct { Register rd, rs, rt; }           add;
        struct { Register rd, rs; int cst; }      addi;
        struct { Register rd, rs, rt; }           sub;
        struct { Register rd, rs, rt; }           mul;
        struct { Register rs, rt; }               div;
        struct { Register rd, rs; int offset;}    lw;
        struct { Register rs, rt; int offset;}    sw;
        struct { Register rd; char* name; }       la;
        struct { Register rd; int imm32;}         li;
        struct { Register rd, rs; }               move;
        struct { Register rd; }                   mflo;
        struct { Operand label; }                 j;
        struct { char* funcName; }                jal;
        struct { Register rs; }                   jr;
        struct { Register rs, rt; Operand label;} branch;
        struct { Operand label; }                 label;
        struct { char *funcName; }                func;
        struct { Register rd, rs; }               negu;
    } ops;
};

struct _REGs {
    union {
        Register regs[32];
        struct
        {
            Register zero;
            Register at;
            Register v[2];
            Register a[4];
            Register t07[8];
            Register s[8];
            Register t89[2];
            Register k[2];
            Register gp;
            Register sp;
            Register fp;
            Register ra;
        } name;
    } u;
};

struct _Register{
    int rno;
    char *name;
};

struct _Op2PosList{
    Operand op;
    Register reg;   
    int offset;           // offset to fp, should be negtive
    Op2PosList next, pre;
};

struct _ActivationRecord {
    int size;

};

#endif