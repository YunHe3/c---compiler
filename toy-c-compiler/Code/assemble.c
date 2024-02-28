/**
 * Activation Record -- from fp to sp
 * Low
 *  -----------------    <-- sp
 * |      Locals     |
 *  -----------------
 * |      old fp     | 4
 *  -----------------
 * |   return addr   | 4
 *  -----------------    <-- fp
 * |   Args(if > 4)  |
 *  -----------------
 * High
*/

#include "assemble.h"

InstrList instrListHead, instrListTail;
Op2PosList op2PosListHead, op2PosListTail;

REGs regs;
int spOffset; // sp's offset to fp, should be negtive; actually is (sp - fp)
int argCount; // mark the number of args, assign to 0 when call
int paramCount; // mark the number of param, assign to 0 when enter a function
int argInStack; // mark the number of args stay in stack, argCount-4
const int baseOffset = -8; // from fp to postion where locals stored

char* _ret = "\n";

void writeStatic(FILE* fp) {
    // start
    fprintf(fp, ".data\n");
    // strings
    fprintf(fp, "_ret: .asciiz \"\\n\"\n");
    // function
    // fprintf(fp, ".globl main\n");
    // end
    fprintf(fp, ".text\n");
}

InstrList translateIR2MIPS(CodeList ir) {
    CodeList walk = ir;
    op2PosListHead = op2PosListTail = NULL;
    instrListHead = instrListTail = NULL;
    regs = newREGs();
    spOffset = baseOffset;
    argCount = 0;

    while (walk != NULL) {
        InstrList list = IC2Instr(walk->code);
        walk = walk->next;
        if (list == NULL) continue;

        // update the head
        if (instrListHead == NULL) instrListHead = list;

        // update the tail
        if (instrListTail != NULL) instrListTail->next = list;
        InstrList instrWalk = list;
        while (instrWalk->next != NULL) instrWalk = instrWalk->next;
        instrListTail = instrWalk;
    }

    instrListTail->next = createInstrExit();
    instrListTail = instrListTail->next;

    return instrListHead;
}

InstrList IC2Instr(InterCode ic) {
    InstrList instrAction = NULL;
    InstrList instrGetReg = NULL;

    switch (ic->kind)
    {
    // intra process
    case IC_LABEL:
        instrAction = createInstrLabel(ic);
        break;
    case IC_ASSIGN:
        instrAction = createInstrAssign(ic);
        break;
    case IC_PLUS:
    case IC_MINUS:
    case IC_DIV:
    case IC_MULTI:
        instrAction = createInstrArithmetic(ic);
        break;
    case IC_REF:
        instrAction = createInstrRef(ic);
        break;
    case IC_DEREF:
        instrAction = createInstrDeref(ic);
        break;

    // jump
    case IC_JUMP:
    case IC_CONDJUMP:
        instrAction = createInstrJump(ic);
        break;

    // inter process
    case IC_FUNC:
        instrAction = createInstrFunc(ic);
        /* Prologue just after function entry */
        op2PosListHead = op2PosListTail = NULL;
        spOffset = baseOffset;
        break;
    case IC_PARAM:
        instrAction = createInstrParam(ic);
        break;
    case IC_RETURN:
        instrAction = createInstrRet(ic);
        break;
    case IC_ARG:
        instrAction = createInstrArg(ic);
        break;
    case IC_CALL:
        instrAction = createInstrCall(ic);
        break;

    // special
    case IC_DEC:
        // TBD
        break;
    case IC_READ:
        instrAction = createInstrRead(ic);
        break;
    case IC_WRITE:
        instrAction = createInstrWrite(ic);
        break;
    default:
        break;
    }

    return instrAction;
}

InstrList getReg(int num, ...) {
    InstrList instrList = NULL;
    va_list args;
    va_start(args, num);

    Operand op = NULL;
    for (int i = 0; i < num; i++) {
        op = va_arg(args, Operand);
        if (op->kind == Em_CONSTANT) continue;
        // check if the operand of ic already been store in stack
        Op2PosList pos = getPosByOp(op);
        if (pos == NULL) {
            // pos = newOp2PosList(op, NULL, spOffset);
            int offset = -getTypeSize(op->type);
            spOffset += offset;
            pos = newOp2PosList(op, NULL, spOffset);

            // modify $sp -> $sp = $sp + offset
            InstrList spSub = newInstrList();
            spSub->instr->kind = INSTR_ADDI;
            spSub->instr->ops.addi.cst = offset;
            spSub->instr->ops.addi.rd = regs->u.name.sp;
            spSub->instr->ops.addi.rs = regs->u.name.sp;

            instrList = instrConcatenate(2, instrList, spSub);
        }
        // choose a free register for the operand
        pos->reg = regs->u.name.t07[i];
        // load the value from stack to register
        instrList = instrConcatenate(2, instrList, 
            createInstrLoad(op, pos->reg));
    }
    va_end(args);

    return instrList;
}

void print_Instr(InstrList head, FILE* fp) {
    writeStatic(fp);

    InstrList walk = head;
    Instruction instr = NULL;

    while (walk != NULL) {
        instr = walk->instr;
        walk = walk->next;

        switch (instr->kind)
        {
        case INSTR_ADD:
            fprintf(fp, "   add $%s, $%s, $%s\n",
                instr->ops.add.rd->name,
                instr->ops.add.rs->name,
                instr->ops.add.rt->name
            );
            break;
        
        case INSTR_ADDI:
            fprintf(fp, "   addi $%s, $%s, %d\n",
                instr->ops.addi.rd->name,
                instr->ops.addi.rs->name,
                instr->ops.addi.cst
            );
            break;
        
        case INSTR_SUB:
            fprintf(fp, "   sub $%s, $%s, $%s\n",
                instr->ops.sub.rd->name,
                instr->ops.sub.rs->name,
                instr->ops.sub.rt->name
            );
            break;

        case INSTR_MUL:
            fprintf(fp, "   mul $%s, $%s, $%s\n",
                instr->ops.mul.rd->name,
                instr->ops.mul.rs->name,
                instr->ops.mul.rt->name
            );
            break;

        case INSTR_DIV:
            fprintf(fp, "   div $%s, $%s\n",
                instr->ops.div.rs->name,
                instr->ops.div.rt->name
            );
            break;

        case INSTR_LW:
            fprintf(fp, "   lw $%s, %d($%s)\n",
                instr->ops.lw.rd->name,
                instr->ops.lw.offset,
                instr->ops.lw.rs->name
            );
            break;

        case INSTR_SW:
            fprintf(fp, "   sw $%s, %d($%s)\n",
                instr->ops.sw.rs->name,
                instr->ops.sw.offset,
                instr->ops.sw.rt->name
            );
            break;

        case INSTR_LA:
            fprintf(fp, "   la $%s, %s\n",
                instr->ops.la.rd->name,
                instr->ops.la.name
            );
            break;

        case INSTR_LI:
            fprintf(fp, "   li $%s, %d\n",
                instr->ops.li.rd->name,
                instr->ops.li.imm32
            );
            break;

        case INSTR_MOVE:
            fprintf(fp, "   move $%s, $%s\n",
                instr->ops.move.rd->name,
                instr->ops.move.rs->name
            );
            break;

        case INSTR_MFLO:
            fprintf(fp, "   mflo $%s\n",
                instr->ops.mflo.rd->name
            );
            break;

        case INSTR_J:
            fprintf(fp, "   j %s\n",
                op2String(instr->ops.j.label)
            );
            break;

        case INSTR_JAL:
            if (strcmp(instr->ops.jal.funcName, "main") == 0) {
                fprintf(fp, "   jal %s\n", instr->ops.jal.funcName); 
            } else {
                fprintf(fp, "   jal _%s\n", instr->ops.jal.funcName);
            }
            break;
        
        case INSTR_JR:
            fprintf(fp, "   jr $%s\n",
                instr->ops.jr.rs->name
            );
            break;

        case INSTR_BEQ:
            fprintf(fp, "   beq $%s, $%s, %s\n",
                instr->ops.branch.rs->name,
                instr->ops.branch.rt->name,
                op2String(instr->ops.branch.label)
            );
            break;

        case INSTR_BNE:
            fprintf(fp, "   bne $%s, $%s, %s\n",
                instr->ops.branch.rs->name,
                instr->ops.branch.rt->name,
                op2String(instr->ops.branch.label)
            );
            break;

        case INSTR_BGT:
            fprintf(fp, "   bgt $%s, $%s, %s\n",
                instr->ops.branch.rs->name,
                instr->ops.branch.rt->name,
                op2String(instr->ops.branch.label)
            );
            break;

        case INSTR_BLT:
            fprintf(fp, "   blt $%s, $%s, %s\n",
                instr->ops.branch.rs->name,
                instr->ops.branch.rt->name,
                op2String(instr->ops.branch.label)
            );
            break;

        case INSTR_BGE:
            fprintf(fp, "   bge $%s, $%s, %s\n",
                instr->ops.branch.rs->name,
                instr->ops.branch.rt->name,
                op2String(instr->ops.branch.label)
            );
            break;

        case INSTR_BLE:
            fprintf(fp, "   ble $%s, $%s, %s\n",
                instr->ops.branch.rs->name,
                instr->ops.branch.rt->name,
                op2String(instr->ops.branch.label)
            );
            break;

        case INSTR_LABEL:
            fprintf(fp, "%s:\n",
                op2String(instr->ops.label.label)
            );
            break;

        case INSTR_FUNC:
            if (strcmp(instr->ops.func.funcName, "main") == 0) {
                fprintf(fp, "%s:\n", instr->ops.func.funcName);
            } else {
                fprintf(fp, "_%s:\n", instr->ops.func.funcName);
            }
            
            break;

        case INSTR_SYSCALL:
            fprintf(fp, "   syscall\n");
            break;

        default:
            break;
        }
    }
}

/*------------- Data Structure -------------*/ 
InstrList newInstrList() {
    InstrList instrList = (InstrList) malloc(sizeof(struct _InstrList));
    instrList->instr = newInstruction();
    instrList->next = instrList->pre = NULL;
    return instrList;
}

Instruction newInstruction() {
    Instruction instruction = (Instruction) malloc(sizeof(struct _Instruction));
    return instruction;
} 

Op2PosList newOp2PosList(Operand op, Register reg, int offset) {
    Op2PosList op2PosList = (Op2PosList) malloc(sizeof(struct _Op2PosList));
    op2PosList->op = op;
    op2PosList->reg = reg;
    op2PosList->offset = offset;
    op2PosList->next = NULL;

    // add to the tail of posList
    if (op2PosListHead == NULL) {
        op2PosListHead = op2PosList;
        op2PosListTail = op2PosList;
    } else {
        op2PosListTail->next = op2PosList;
        op2PosListTail = op2PosListTail->next;
    }
    return op2PosList;
}

Register newReigster(int rno, char *name) {
    Register reg = (Register) malloc(sizeof(struct _Register));
    reg->name = name;
    reg->rno = rno;
    return reg;
}

REGs newREGs() {
    REGs regs = (REGs) malloc(sizeof(struct _REGs));
    regs->u.name.zero = newReigster(0, "zero");
    regs->u.name.at = newReigster(1, "at");

    regs->u.name.v[0] = newReigster(2, "v0");
    regs->u.name.v[1] = newReigster(3, "v1");

    regs->u.name.a[0] = newReigster(4, "a0");
    regs->u.name.a[1] = newReigster(5, "a1");
    regs->u.name.a[2] = newReigster(6, "a2");
    regs->u.name.a[3] = newReigster(7, "a3");

    regs->u.name.t07[0] = newReigster(8, "t0");
    regs->u.name.t07[1] = newReigster(9, "t1");
    regs->u.name.t07[2] = newReigster(10, "t2");
    regs->u.name.t07[3] = newReigster(11, "t3");
    regs->u.name.t07[4] = newReigster(12, "t4");
    regs->u.name.t07[5] = newReigster(13, "t5");
    regs->u.name.t07[6] = newReigster(14, "t6");
    regs->u.name.t07[7] = newReigster(15, "t7");

    regs->u.name.s[0] = newReigster(16, "s0");
    regs->u.name.s[1] = newReigster(17, "s1");
    regs->u.name.s[2] = newReigster(18, "s2");
    regs->u.name.s[3] = newReigster(19, "s3");
    regs->u.name.s[4] = newReigster(20, "s4");
    regs->u.name.s[5] = newReigster(21, "s5");
    regs->u.name.s[6] = newReigster(22, "s6");
    regs->u.name.s[7] = newReigster(23, "s7");

    regs->u.name.t89[0] = newReigster(24, "t8");
    regs->u.name.t89[1] = newReigster(25, "t9");

    regs->u.name.k[0] = newReigster(26, "k0");
    regs->u.name.k[1] = newReigster(27, "k1");

    regs->u.name.gp = newReigster(28, "gp");
    regs->u.name.sp = newReigster(29, "sp");
    regs->u.name.fp = newReigster(30, "fp");
    regs->u.name.ra = newReigster(31, "ra");

    return regs;
}

Register getRegByOp(Operand op) {
    Op2PosList walk = op2PosListHead;
    while (walk != NULL) {
        if (strcmp(walk->op->name, op->name) == 0) return walk->reg;
        walk = walk->next;
    }
    assert(0);
    return NULL;
}

int getOffsetByOp(Operand op) {
    Op2PosList walk = op2PosListHead;
    while (walk != NULL) {
        if (strcmp(walk->op->name, op->name) == 0) return walk->offset;
        walk = walk->next;
    }
    return -1;
}

Op2PosList getPosByOp(Operand op) {
    Op2PosList walk = op2PosListHead;
    while (walk != NULL) {
        if (strcmp(walk->op->name, op->name) == 0) return walk;
        walk = walk->next;
    }
    return NULL;
}

InstrList instrConcatenate(int num, ...) {
    va_list args;
    va_start(args, num);

    InstrList head, tail;
    head = tail = va_arg(args, InstrList);
    for (int i = 1; i < num; i++) {
        InstrList tmp = va_arg(args, InstrList);
        if (tmp == NULL) continue;
        if (head == NULL) {
            head = tmp;
            tail = tmp;
        } else {
            instrConnect(tail, tmp);
            tail = tmp;
        }
    }
    va_end(args);

    return head;
}

void instrConnect(InstrList i1, InstrList i2) {
    if (i1 == NULL) return;
    // get the tail of c1
    InstrList walk = i1;
    while (walk != NULL && walk->next != NULL) walk = walk->next;
    walk->next = i2;
    i2->pre = walk;
}

/*---------- Instrution Generation ---------*/
InstrList createInstrArithmetic(InterCode ic) {
    InstrList interList = newInstrList();
    InstrList temp = NULL;

    Operand res = ic->u.binop.result;
    Operand op1 = ic->u.binop.op1;
    Operand op2 = ic->u.binop.op2;

    InstrList instrGetReg = getReg(3, res, op1, op2);

    Register rs = NULL, rt = NULL;
    InstrList instrLoad = NULL;

    switch (ic->kind)
    {
    case IC_PLUS:    
        if (op1->kind == Em_CONSTANT || op2->kind == Em_CONSTANT) {
            interList->instr->kind = INSTR_ADDI;
            interList->instr->ops.addi.rd = getRegByOp(res);
            if (op1->kind == Em_CONSTANT) {
                interList->instr->ops.addi.cst = op1->u.val;
                interList->instr->ops.addi.rs = getRegByOp(op2);
            } else {
                interList->instr->ops.addi.cst = op2->u.val;
                interList->instr->ops.addi.rs = getRegByOp(op1);
            }
        } else {
            interList->instr->kind = INSTR_ADD;
            interList->instr->ops.add.rd = getRegByOp(res);
            interList->instr->ops.add.rs = getRegByOp(op1);
            interList->instr->ops.add.rt = getRegByOp(op2);
        }
        break;

    case IC_MINUS:
        if (op1->kind == Em_CONSTANT || op2->kind == Em_CONSTANT) {
            interList->instr->kind = INSTR_ADDI;
            interList->instr->ops.addi.rd = getRegByOp(res);
            if (op2->kind == Em_CONSTANT) {
                interList->instr->ops.addi.cst = 0 - op2->u.val;
                interList->instr->ops.addi.rs = getRegByOp(op1);
            } else {
                // $t = $zero - $t
                temp = newInstrList();
                temp->instr->kind = INSTR_SUB;
                temp->instr->ops.sub.rd = temp->instr->ops.sub.rt = getRegByOp(op2);
                temp->instr->ops.sub.rs = regs->u.name.zero;

                interList->instr->ops.addi.cst = op1->u.val;
                interList->instr->ops.addi.rs = getRegByOp(op2);
                interList = instrConcatenate(2, temp, interList);
            }
        } else {
            interList->instr->kind = INSTR_SUB;
            interList->instr->ops.sub.rd = getRegByOp(res);
            interList->instr->ops.sub.rs = getRegByOp(op1);
            interList->instr->ops.sub.rt = getRegByOp(op2);
        }
        break;
    case IC_MULTI:
        instrLoad = NULL;
        rs = rt = NULL;
        if (op1->kind == Em_CONSTANT || op2->kind == Em_CONSTANT) {
            if (op1->kind == Em_CONSTANT && op2->kind == Em_CONSTANT) {
                // load op1 and op2 into a register(here we use t4)
                rs = regs->u.name.t07[3];
                rt = regs->u.name.t07[4];

                InstrList i1 = newInstrList();
                i1->instr->kind = INSTR_LI;
                i1->instr->ops.li.imm32 = op1->u.val;
                i1->instr->ops.li.rd = rs;

                InstrList i2 = newInstrList();
                i2->instr->kind = INSTR_LI;
                i2->instr->ops.li.imm32 = op2->u.val;
                i2->instr->ops.li.rd = rt;
                
                instrLoad = instrConcatenate(2, i1, i2);
            } else if (op1->kind == Em_CONSTANT){
                // load op1 into a register(here we use t4)
                rs = regs->u.name.t07[3];
                rt = getRegByOp(op2);

                InstrList i1 = newInstrList();
                i1->instr->kind = INSTR_LI;
                i1->instr->ops.li.imm32 = op1->u.val;
                i1->instr->ops.li.rd = rs;

                instrLoad = i1;
            } else {
                // load op2 into a register(here we use t4)
                rs = getRegByOp(op1);
                rt = regs->u.name.t07[3];

                InstrList i1 = newInstrList();
                i1->instr->kind = INSTR_LI;
                i1->instr->ops.li.imm32 = op2->u.val;
                i1->instr->ops.li.rd = rt;

                instrLoad = i1;
            }
        } else {
            rs = getRegByOp(op1);
            rt = getRegByOp(op2);
        }
        interList->instr->kind = INSTR_MUL;
        interList->instr->ops.mul.rd = getRegByOp(res);
        interList->instr->ops.mul.rs = rs;
        interList->instr->ops.mul.rt = rt;

        interList = instrConcatenate(2, instrLoad, interList);
        break;
    case IC_DIV:
        interList->instr->kind = INSTR_DIV;
        interList->instr->ops.div.rs = getRegByOp(op1);
        interList->instr->ops.div.rt = getRegByOp(op2);

        interList = instrConcatenate(2, interList, 
            createInstrMflo(getRegByOp(res)));
        break;
    default:
        break;
    }

    // store back to stack
    InstrList instrStore = createInstrStore(res, getRegByOp(res));
    return instrConcatenate(3, instrGetReg, interList, instrStore);
}

InstrList createInstrJump(InterCode ic) {
    InstrList instrList = newInstrList();

    if (ic->kind == IC_JUMP) {
        instrList->instr->kind = INSTR_J;
        instrList->instr->ops.j.label = ic->u.jump.label;
    } else if (ic->kind == IC_CONDJUMP) {
        char *relop = ic->u.condjump.relop;
        Operand op1 = ic->u.condjump.op1;
        Operand op2 = ic->u.condjump.op2;
        InstrList instrGetReg = getReg(2, op1, op2);
        InstrList instrLoad = NULL;

        instrList->instr->ops.branch.label = ic->u.condjump.label;
        instrList->instr->ops.branch.rs = getRegByOp(op1);

        if (op2->kind == Em_CONSTANT) {
            instrLoad = newInstrList();
            instrLoad->instr->kind = INSTR_LI;
            instrLoad->instr->ops.li.imm32 = op2->u.val;
            instrLoad->instr->ops.li.rd = regs->u.name.t07[3];
            instrList->instr->ops.branch.rt = regs->u.name.t07[3];
        } else {
            instrList->instr->ops.branch.rt = getRegByOp(op2);
        }
        
        if (strcmp(relop, "==") == 0) instrList->instr->kind = INSTR_BEQ;
        else if (strcmp(relop, "!=") == 0) instrList->instr->kind = INSTR_BNE;
        else if (strcmp(relop, ">") == 0) instrList->instr->kind = INSTR_BGT;
        else if (strcmp(relop, "<") == 0) instrList->instr->kind = INSTR_BLT;
        else if (strcmp(relop, ">=") == 0) instrList->instr->kind = INSTR_BGE;
        else if (strcmp(relop, "<=") == 0) instrList->instr->kind = INSTR_BLE;
        else {
            // shouldn't run to here
            fprintf(stderr, "Error\n");
            return NULL;
        }

        instrList = instrConcatenate(3, instrGetReg, instrLoad, instrList);
    } else {
        // shouldn't run to here
        fprintf(stderr, "Error\n");
        return NULL;
    }
    return instrList;
}

InstrList createInstrLoad(Operand src, Register dst) {
    InstrList instrList = newInstrList();
    instrList->instr->kind = INSTR_LW;
    instrList->instr->ops.lw.offset = getOffsetByOp(src);
    instrList->instr->ops.lw.rd = dst;
    instrList->instr->ops.lw.rs = regs->u.name.fp;
    return instrList;
}

InstrList createInstrStore(Operand dst, Register src) {
    InstrList instrList = newInstrList();
    instrList->instr->kind = INSTR_SW;
    instrList->instr->ops.sw.offset = getOffsetByOp(dst);
    instrList->instr->ops.sw.rs = src;
    instrList->instr->ops.sw.rt = regs->u.name.fp;
    return instrList;
}

InstrList createInstrLabel(InterCode ic) {
    InstrList instrList = newInstrList();
    instrList->instr->kind = INSTR_LABEL;
    instrList->instr->ops.label.label = ic->u.label;
    return instrList;
}

InstrList createInstrAssign(InterCode ic) {
    InstrList instrList = newInstrList();

    Operand src = ic->u.assign.right;
    Operand des = ic->u.assign.left;

    InstrList instrGetReg = NULL;

    if (src->kind == Em_CONSTANT) {
        instrGetReg = getReg(1, des);

        instrList->instr->kind = INSTR_LI;
        instrList->instr->ops.li.rd = getRegByOp(des);
        instrList->instr->ops.li.imm32 = src->u.val;
    } else {
        instrGetReg = getReg(2, des, src);

        instrList->instr->kind = INSTR_MOVE;
        instrList->instr->ops.move.rd = getRegByOp(des);
        instrList->instr->ops.move.rs = getRegByOp(src);
    }
    InstrList instrStore = createInstrStore(des, getRegByOp(des));
    return instrConcatenate(3, instrGetReg, instrList, instrStore);
}

InstrList createInstrRef(InterCode ic) {
    Operand op1 = ic->u.ref.left;
    Operand op2 = ic->u.ref.right;

    // op1 = &op2 -> op1 = $fp + getOffset(op2)
    // reg(op1) = $sp + offset(op2); store(op1)
    InstrList instrGetReg = getReg(2, op1, op2);
    InstrList i1 = newInstrList();
    i1->instr->kind = INSTR_ADDI;
    i1->instr->ops.addi.rd = getRegByOp(op1);
    i1->instr->ops.addi.rs = regs->u.name.fp;
    i1->instr->ops.addi.cst = getOffsetByOp(op2);

    InstrList i3 = NULL;
    // if is array, then op1 = op1 - sizeof(op2) -> addi op1, op1, -sizeof(op2)
    // if (op2->type->kind == ARRAY) {
    //     InstrList i3 = newInstrList();
    //     i3->instr->kind = INSTR_ADDI;
    //     i3->instr->ops.addi.rd = i3->instr->ops.addi.rs = getRegByOp(op1);
    //     i3->instr->ops.addi.cst = -getTypeSize(op2->type);
    // }

    InstrList i2 = createInstrStore(op1, getRegByOp(op1));

    return instrConcatenate(4, instrGetReg, i1, i3, i2);
}

InstrList createInstrDeref(InterCode ic) {
    InstrList instrList = newInstrList();
    
    Operand left = ic->u.deref.left;
    Operand right = ic->u.deref.right;

    InstrList instrGetReg = getReg(2, left, right);
    if (left->kind == Em_ADDRESS) {
        // *left = right
        // step1. 
        instrList->instr->kind = INSTR_SW;
        instrList->instr->ops.sw.rs = getRegByOp(right);
        instrList->instr->ops.sw.rt = getRegByOp(left);
        instrList->instr->ops.sw.offset = 0;
        return instrConcatenate(2, instrGetReg, instrList);
    } else {
        instrList->instr->kind = INSTR_LW;
        instrList->instr->ops.lw.rd = getRegByOp(left);
        instrList->instr->ops.lw.rs = getRegByOp(right);
        instrList->instr->ops.sw.offset = 0;
        
        InstrList instrStore = createInstrStore(left, getRegByOp(left));
        return instrConcatenate(3, instrGetReg, instrList, instrStore);
    }
    
    return NULL;
}

InstrList createInstrMflo(Register reg) {
    InstrList instrList = newInstrList();
    instrList->instr->kind = INSTR_MFLO;
    instrList->instr->ops.mflo.rd = reg;
    return instrList;
}

InstrList createInstrArg(InterCode ic) {
    // from high to low
    InstrList instrAction = NULL;

    InstrList instrGetReg = getReg(1, ic->u.arg.arg);
    if (ic->u.arg.number <= 4) {
        // store in the register
        InstrList instrLoad = createInstrLoad(ic->u.arg.arg, regs->u.name.a[ic->u.arg.number-1]);
        instrAction = instrConcatenate(3, instrAction, instrGetReg , instrLoad);
    } else {
        // store in stack
        // modify the sp
        InstrList subSp = newInstrList();
        subSp->instr->kind = INSTR_ADDI;
        subSp->instr->ops.addi.cst = -4;
        subSp->instr->ops.addi.rd = subSp->instr->ops.addi.rs = regs->u.name.sp;
        // push
        InstrList storeARG = newInstrList();
        storeARG->instr->kind = INSTR_SW;
        storeARG->instr->ops.sw.offset = 0;
        storeARG->instr->ops.sw.rs = getRegByOp(ic->u.arg.arg);
        storeARG->instr->ops.sw.rt = regs->u.name.sp;

        instrAction = instrConcatenate(4, instrAction, instrGetReg, subSp, storeARG);
    }

    return instrAction;
}

InstrList createInstrCall_helper(InterCode ic) {
    Operand res = ic->u.call.result;

    InstrList instrGetReg = getReg(1, res);

    InstrList instrJal = newInstrList();
    instrJal->instr->kind = INSTR_JAL;
    instrJal->instr->ops.jal.funcName = ic->u.call.func;

    InstrList instrMove = newInstrList();
    instrMove->instr->kind = INSTR_MOVE;
    instrMove->instr->ops.move.rd = getRegByOp(res);
    instrMove->instr->ops.move.rs = regs->u.name.v[0];

    InstrList instrStore = createInstrStore(res, getRegByOp(res));
    return instrConcatenate(4, 
        instrGetReg,
        instrJal,
        instrMove,
        instrStore
        ); 
}

InstrList createInstrRet(InterCode ic) {
    Operand ret = ic->u.op;

    InstrList instrGetReg = getReg(1, ret);

    InstrList instrMove = newInstrList();
    instrMove->instr->kind = INSTR_MOVE;
    instrMove->instr->ops.move.rd = regs->u.name.v[0];
    instrMove->instr->ops.move.rs = getRegByOp(ret);

    InstrList instrJr = newInstrList();
    instrJr->instr->kind = INSTR_JR;
    instrJr->instr->ops.jr.rs = regs->u.name.ra;

    /* Epilogue before return */
        
    // reload $ra, $fp
    InstrList instrLoadRa = newInstrList();
    instrLoadRa->instr->kind = INSTR_LW;
    instrLoadRa->instr->ops.lw.offset = -4;
    instrLoadRa->instr->ops.lw.rd = regs->u.name.ra;
    instrLoadRa->instr->ops.lw.rs = regs->u.name.fp;

    InstrList instrLoadFp = newInstrList();
    instrLoadFp->instr->kind = INSTR_LW;
    instrLoadFp->instr->ops.lw.offset = -8;
    instrLoadFp->instr->ops.lw.rd = regs->u.name.fp;
    instrLoadFp->instr->ops.lw.rs = regs->u.name.fp;

    // modify $sp -> $sp = $sp + framzsize
    // InstrList addSp = newInstrList();
    // addSp->instr->kind = INSTR_ADDI;
    // addSp->instr->ops.addi.cst = -spOffset;
    // addSp->instr->ops.addi.rd = addSp->instr->ops.addi.rs = regs->u.name.sp;

    // or move $sp, $fp
    InstrList addSp = newInstrList();
    addSp->instr->kind = INSTR_MOVE;
    addSp->instr->ops.move.rd = regs->u.name.sp;
    addSp->instr->ops.move.rs = regs->u.name.fp;

    return instrConcatenate(6, 
        instrGetReg,
        instrMove,
        addSp,
        instrLoadRa,
        instrLoadFp,
        
        instrJr
    );
}

InstrList createInstrFunc(InterCode ic) {
    InstrList instrList = newInstrList();
    instrList->instr->kind = INSTR_FUNC;
    instrList->instr->ops.func.funcName = ic->u.func;

    /* Prologue just after function entry */

    // modify the sp, increase it ($sp = $sp + baseOffset)
    InstrList spSub = newInstrList();
    spSub->instr->kind = INSTR_ADDI;
    spSub->instr->ops.addi.cst = baseOffset;
    spSub->instr->ops.addi.rd = spSub->instr->ops.addi.rs = regs->u.name.sp;
    // push register -> $ra, $fp, callee saved regs(No Need)
    InstrList storeRa = newInstrList();
    storeRa->instr->kind = INSTR_SW;
    storeRa->instr->ops.sw.offset = (-baseOffset) - 4;
    storeRa->instr->ops.sw.rs = regs->u.name.ra;
    storeRa->instr->ops.sw.rt = regs->u.name.sp;

    InstrList storeFP = newInstrList();
    storeFP->instr->kind = INSTR_SW;
    storeFP->instr->ops.sw.offset = (-baseOffset) - 8;
    storeFP->instr->ops.sw.rs = regs->u.name.fp;
    storeFP->instr->ops.sw.rt = regs->u.name.sp;
    
    // update $fp = $sp + framesize
    InstrList instrUpdateFp = newInstrList();
    instrUpdateFp->instr->kind = INSTR_ADDI;
    instrUpdateFp->instr->ops.addi.cst = (-baseOffset);
    instrUpdateFp->instr->ops.addi.rd = regs->u.name.fp;
    instrUpdateFp->instr->ops.addi.rs = regs->u.name.sp;

    // get the args in stack, complete when translate the PARAM
    paramCount = 0;
    instrList = instrConcatenate(5, instrList, spSub, storeRa, storeFP, instrUpdateFp);
    return instrList;
}

InstrList createInstrParam(InterCode ic) {
    // from low to high
    // get the args (1~4 in register, >4 in stack)
    // create pos for operand(param)
    InstrList instrAction = NULL;

    InstrList instrGetReg = getReg(1, ic->u.param.param);
    if (ic->u.param.number <= 4) {
        // param in register
        InstrList instrStore = createInstrStore(ic->u.param.param, regs->u.name.a[ic->u.param.number-1]);
        instrAction = instrConcatenate(3, instrAction, instrGetReg, instrStore);
    } else {
        // param in stack, lw Pn, (4*(n-5))($fp)
        // in other words, ParamK = *($fp + 4*(k-5))
        InstrList instrLoad = newInstrList();
        instrLoad->instr->kind = INSTR_LW;
        instrLoad->instr->ops.lw.offset = 4*(ic->u.param.number-5);
        instrLoad->instr->ops.lw.rd = getRegByOp(ic->u.param.param);
        instrLoad->instr->ops.lw.rs = regs->u.name.fp;

        // store the param value from reg to stack
        InstrList instrStore = createInstrStore(ic->u.param.param, getRegByOp(ic->u.param.param));
        instrAction = instrConcatenate(4, instrAction, instrGetReg, instrLoad, instrStore);
    }

    return instrAction;   
}

InstrList createInstrCall(InterCode ic) {
    /* Before Call*/
    // store all the caller save register value to stack
    // No Need

    // push the args (if > 4) to stack
    // Have been done when translate InterCode ARG
    argInStack = MAX(argCount - 4, 0);
    argCount = 0;

    /* Call */
    InstrList instrAction = createInstrCall_helper(ic);

    /* After Call */
    // pop the args stay in stack
    InstrList popArgs = newInstrList();
    popArgs->instr->kind = INSTR_ADDI;
    popArgs->instr->ops.addi.cst = argInStack * 4;  // the args can be only int
    popArgs->instr->ops.addi.rd = popArgs->instr->ops.addi.rs = regs->u.name.sp;

    // load all caller save register value from stack to register
    // No Need

    instrAction = instrConcatenate(2, instrAction, popArgs);
    return instrAction;
}

InstrList createInstrRead(InterCode ic) {
    InstrList instrGetReg = getReg(1, ic->u.op);

    // syscall number 5, li $v0, 5
    InstrList i1 = newInstrList();
    i1->instr->kind = INSTR_LI;
    i1->instr->ops.li.rd = regs->u.name.v[0];
    i1->instr->ops.li.imm32 = 5;

    // syscall
    InstrList syscall = newInstrList();
    syscall->instr->kind = INSTR_SYSCALL;

    // move ans to des, move Rd, $v0
    InstrList i2 = newInstrList();
    i2->instr->kind = INSTR_MOVE;
    i2->instr->ops.move.rd = getRegByOp(ic->u.op);
    i2->instr->ops.move.rs = regs->u.name.v[0];

    // store the ans to stack
    InstrList i3 = createInstrStore(ic->u.op, getRegByOp(ic->u.op));

    return instrConcatenate(5, instrGetReg, i1, syscall, i2, i3);
}

InstrList createInstrWrite(InterCode ic) {
    InstrList instrGetReg = getReg(1, ic->u.op);
    // save old $a0

    // syscall 1, li $v0, 1
    InstrList i1 = newInstrList();
    i1->instr->kind = INSTR_LI;
    i1->instr->ops.li.imm32 = 1;
    i1->instr->ops.li.rd = regs->u.name.v[0];
    // move src to syscall para, move $a0, Rs
    InstrList i2 = newInstrList();
    i2->instr->kind = INSTR_MOVE;
    i2->instr->ops.move.rd = regs->u.name.a[0];
    i2->instr->ops.move.rs = getRegByOp(ic->u.op);
    // syscall
    InstrList syscall = newInstrList();
    syscall->instr->kind = INSTR_SYSCALL;

    // "\n", syscall 4, li $v0, 4
    InstrList i3 = newInstrList();
    i3->instr->kind = INSTR_LI;
    i3->instr->ops.li.imm32 = 4;
    i3->instr->ops.li.rd = regs->u.name.v[0];

    // la $a0, "\n"
    InstrList i4 = newInstrList();
    i4->instr->kind = INSTR_LA;
    i4->instr->ops.la.rd = regs->u.name.a[0];
    i4->instr->ops.la.name = "_ret";

    InstrList i5 = newInstrList();
    i5->instr->kind = INSTR_SYSCALL;
    
    // load old $a0
    return instrConcatenate(7, instrGetReg, i1, i2, syscall, i3, i4, i5);
}

InstrList createInstrExit() {
    // syscall 10, li $v0, 10
    InstrList i = newInstrList();
    i->instr->kind = INSTR_LI;
    i->instr->ops.li.imm32 = 10;
    i->instr->ops.li.rd = regs->u.name.v[0];
    // syscall
    InstrList syscall = newInstrList();
    syscall->instr->kind = INSTR_SYSCALL;
    
    return instrConcatenate(2, i, syscall);
}