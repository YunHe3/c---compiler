#include "ir.h"

int var_num, label_num, temp_num; // 新变量/新标签/新临时变量编号 
CodeList code_head, code_tail; // 双链表的首部和尾部
Variable var_head, var_tail; // 变量表的首部和尾部
Operand const0, const1m, const1p;
int irErrorFlag;

CodeList interCode(node_t* Root){
    if(Root == NULL) { return NULL; }
    if(strcmp(Root->tokenName, "Program") != 0) return NULL;

    // 初始化全局变量中间代码变量等列表;
    irErrorFlag = 0;
    code_head = code_tail = NULL;
    var_tail = var_head = NULL;
    // 设定变量、标签、临时变量标号;
    var_num = 1; label_num = 1; temp_num = 1;
    // 常用的常数
    const0 = new_operand();
    const1m = new_operand();
    const1p = new_operand();
    const1m->kind = const1p->kind = const0->kind = Em_CONSTANT;
    const0->u.val = 0;
    const1m->u.val = -1;
    const1p->u.val = 1;

    // Program -> ExtDefList
    node_t* extdeflist = Root->childs[0]; 
    
    // ExfDefList -> ExtDef ExtDefList
    while(extdeflist!= NULL){
        node_t* extdef = extdeflist->childs[0]; 
        code_head  = concatenate(2, code_head, translate_ExtDef(extdef));
        extdeflist = extdeflist->childs[1];
    }
    return code_head;
}

CodeList translate_ExtDef(node_t* extdef){
    // 检查语法树是否为空;
    if (extdef == NULL) return NULL;
    if(strcmp(extdef->tokenName, "ExtDef") != 0) return NULL; 

    // ExtDef -> Specifier SEMI
    if(extdef->childNum == 2 && strcmp(extdef->childs[1]->tokenName, "SEMI") == 0)
        return NULL;
    
    // ExtDef -> Specifier ExtDecList SEMI
    if(extdef->childNum == 3 && strcmp(extdef->childs[1]->tokenName, "ExtDecList") == 0)
        return NULL;

    // ExtDef -> Specifier FunDec CompSt
    if(extdef->childNum == 3) {
        CodeList c1 = translate_FunDec(extdef->childs[1]); 
        CodeList c2 = translate_CompSt(extdef->childs[2]); 
        // 合并函数声明和函数体
        return concatenate(2, c1,c2);
    } else {
        fprintf(stderr, "error ExtDef!\n"); return NULL;
    }
} 

CodeList translate_FunDec(node_t* fundec) {
    if (fundec == NULL) return NULL;
    if (strcmp(fundec->tokenName, "FunDec") != 0) return NULL;
    
    // FunDec -> ID(VarList) | ID()
    node_t* id = fundec->childs[0];
    CodeList c1 = createCodeFunc(id->tokenName);

    if (fundec->childNum == 4) {
        // VarList, already in symbol table
        Item funcItem = findItem(funcTable, id->tokenName);
        FieldList params = funcItem->func.params;
        int paramCount = 0;
        while (params!=NULL) {
            paramCount++;
            Operand p = getVarByName(params->name)->op;
            p->type = params->type;
            CodeList codeParam = createCodeParam(p, paramCount);
            p->para = 1;
            c1 = concatenate(2, c1, codeParam);
            params = params->tail;
        }
    }
    return c1;
}

CodeList translate_CompSt(node_t *compst) {
    if (compst == NULL) return NULL; 
    if (strcmp(compst->tokenName, "CompSt") != 0) return NULL;

    // CompSt -> {DefList StmtList}
    node_t* deflist = compst->childs[1];
    node_t* stmtlist = compst->childs[2];
    
    CodeList c1 = translate_DefList(deflist); // DefList    
    CodeList c2 = translate_StmtList(stmtlist); // StmtList

    return concatenate(2, c1, c2);
}

CodeList translate_DefList(node_t* deflist) {
    if (deflist == NULL) return NULL;
    if (strcmp(deflist->tokenName, "DefList") != 0) return NULL;

    CodeList c = NULL;
    // DefList -> Def DefList | empty
    while (deflist->childNum > 0) {
        node_t* def = deflist->childs[0];
        deflist = deflist->childs[1];

        // Def -> Specifier DecList SEMI
        node_t* specifier = def->childs[0];
        node_t* declist = def->childs[1];

        // DecList -> Dec | Dec COMMA DecList
        while (declist != NULL) {
            node_t* dec = declist->childs[0];
            // Dec -> VarDec | VarDec ASSIGN Exp
            Operand decVar = getVarByName(dec->decName)->op;
            decVar->type = dec->semanticType;
            c = concatenate(2, c, createCodeDec(decVar, getTypeSize(dec->semanticType)));
            if (dec->childNum > 1) {
                // Dec -> VarDec ASSIGN Exp
                Operand left = getVarByName(dec->decName)->op;
                Operand right = new_temp();
                CodeList c1 = translate_Exp(dec->childs[2], right);
                CodeList c2 = createCodeAssign(left, right);
                // c1 || c2
                if (c == NULL) {
                    c = concatenate(2, c1, c2);
                } else {
                    c = concatenate(3, c, c1, c2);
                }
            }
            if (declist->childNum == 1) break;
            declist = declist->childs[2];
        }
    }
    return c;
}

CodeList translate_StmtList(node_t* stmtlist) {
    if (stmtlist == NULL) return NULL;
    if (strcmp(stmtlist->tokenName, "StmtList") != 0) return NULL;
    CodeList c1 = NULL;
    // StmtList -> Stmt StmtList | empty
    while (stmtlist->childNum > 0) {
        node_t* stmt = stmtlist->childs[0];
        stmtlist = stmtlist->childs[1];

        if (c1 == NULL) {
            c1 = translate_Stmt(stmt);
        } else {
            c1 = concatenate(2, c1, translate_Stmt(stmt));
        }
    }
    return c1;
}

CodeList expChecker(node_t* exp, Operand* op) {
    if (exp == NULL) return NULL;
    if (strcmp(exp->tokenName, "Exp") != 0) return NULL;
    CodeList c = NULL;
    Operand helper = *op;

    if (isConst(exp) == 1) {
        *op = getConst(exp);
    } else if (isVar(exp) == 1) {
        *op = getVar(exp);
    } else {
        *op = new_temp();
        CodeList c = translate_Exp(exp, *op);
    }
    free(helper);
    return c;
}

CodeList translate_Stmt(node_t* stmt) {
    if (stmt == NULL) return NULL;
    if (strcmp(stmt->tokenName, "Stmt") != 0) return NULL;

    if (stmt->childNum == 1) {
        // Stmt -> CompSt
        return translate_CompSt(stmt->childs[0]);
    } else if (stmt->childNum == 2) {
        // Stmt -> Exp SEMI
        Operand tmp = new_temp();
        return translate_Exp(stmt->childs[0], tmp);
    } else if (stmt->childNum == 3) {
        // Stmt -> RETURN Exp SEMI
        Operand t1 = new_temp();
        CodeList c1 = translate_Exp(stmt->childs[1], t1);
        CodeList c2 = createCodeRet(t1);
        return concatenate(2, c1, c2);
    } else if (stmt->childNum == 5) {
        if (strcmp(stmt->childs[0]->tokenName, "IF") == 0) {
            // Stmt -> IF LP Exp RP Stmt
            Operand trueLabel = new_label();
            Operand falseLabel = new_label();
            CodeList c1 = translate_Cond(stmt->childs[2], trueLabel, falseLabel);
            CodeList c2 = translate_Stmt(stmt->childs[4]);
            // c1 || truelabel || c2 || falselabel
            return concatenate(
                4,
                c1, 
                label2CodeList(trueLabel), 
                c2, 
                label2CodeList(falseLabel));
        } else {
            // Stmt -> WHILE LP Exp RP Stmt
            Operand trueLabel = new_label();
            Operand falseLabel = new_label();
            Operand beginLabel = new_label();
            CodeList c1 = translate_Cond(stmt->childs[2], trueLabel, falseLabel);
            CodeList c2 = translate_Stmt(stmt->childs[4]);
            // beginLabel || c1 || trueLabel || c2 || GOTO beginLabel || falseLabel
            return concatenate(
                6,
                label2CodeList(beginLabel),
                c1,
                label2CodeList(trueLabel),
                c2,
                createCodeGOTO(beginLabel),
                label2CodeList(falseLabel)
            );
        }
    } else {
        // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        Operand trueLabel = new_label();
        Operand falseLabel = new_label();
        Operand nextLabel = new_label();
        CodeList c1 = translate_Cond(stmt->childs[2], trueLabel, falseLabel);
        CodeList c2 = translate_Stmt(stmt->childs[4]);
        CodeList c3 = translate_Stmt(stmt->childs[6]);
        // c1 || truelabel || c2 || GOTO label || falselabel || c3 || label
        return concatenate(
            7,
            c1,
            label2CodeList(trueLabel),
            c2,
            createCodeGOTO(nextLabel),
            label2CodeList(falseLabel),
            c3,
            label2CodeList(nextLabel)
        );
    }
}

CodeList translate_Exp(node_t* exp, Operand place) {
    if (exp == NULL) return NULL;
    if (strcmp(exp->tokenName, "Exp") != 0) return NULL;
    assert(place != NULL);

    place->type = exp->semanticType;
    if (exp->childNum == 1) {
        if (exp->childs[0]->type == LEXICAL_TOKEN_ID) {
            // Exp -> ID
            Operand var = getVarByName(exp->childs[0]->tokenName)->op;
            // return createCodeAssign(place, var);
            *place = *var;
            return NULL;
        } else if (exp->childs[0]->type == LEXICAL_TOKEN_NUMBER) {
            // Exp -> INT
            int val = atoi(exp->childs[0]->childs[0]->tokenName);
            return createCodeAssign(place, new_const(val));
        }
    } else if (exp->childNum == 3) {
        if (strcmp(exp->childs[0]->tokenName, "Exp") == 0 && 
            strcmp(exp->childs[2]->tokenName, "Exp") == 0) {
            // Exp Op Exp
            node_t* op = exp->childs[1];
            node_t* exp1 = exp->childs[0];
            node_t* exp2 = exp->childs[2];

            if (strcmp(op->tokenName, "ASSIGNOP") == 0) {
                if (exp1->childNum == 1 && exp1->childs[0]->type == LEXICAL_TOKEN_ID) {
                    // Exp1 -> ID
                    Operand left = getVarByName(exp1->childs[0]->tokenName)->op;
                    left->type = exp1->semanticType;

                    Operand right = new_temp();
                    CodeList c1 = translate_Exp(exp2, right);
                    CodeList c2 = createCodeAssign(left, right);
                    CodeList c3 = createCodeAssign(place, left);
                    return concatenate(3, c1, c2, c3);
                } else {
                    // need use address
                    Operand left = new_addresss();
                    Operand right = new_temp();
                    CodeList c1 = translate_Exp(exp1, left);
                    CodeList c2 = translate_Exp(exp2, right);
                    CodeList c3 = createCodeDeref(left, right);    // *left = right
                    CodeList c4 = createCodeDeref(place, left);    // place = *left
                    return concatenate(4, c1, c2, c3, c4);
                }
            } else if (strcmp(op->tokenName, "RELOP") == 0 
                || strcmp(op->tokenName, "AND") ==0 
                || strcmp(op->tokenName, "OR") == 0) {
                    // Exp -> Exp RELOP Exp | Exp AND Exp | Exp OR Exp
                    Operand trueLabel = new_label();
                    Operand falseLabel = new_label();
                    CodeList c0 = createCodeAssign(place, new_const(0));
                    CodeList c1 = translate_Cond(exp, trueLabel, falseLabel);

                    return concatenate(
                        5,
                        c0,
                        c1,
                        label2CodeList(trueLabel),
                        createCodeAssign(place, new_const(1)),
                        label2CodeList(falseLabel)
                    );
            } else {
                Operand t1 = new_temp();
                Operand t2 = new_temp();
                CodeList c1 = translate_Exp(exp1, t1);
                CodeList c2 = translate_Exp(exp2, t2);
                CodeList c3 = createBinOp(place, t1, t2);

                if (strcmp(op->tokenName, "PLUS") == 0) {
                    c3->code->kind = IC_PLUS;
                } else if (strcmp(op->tokenName, "MINUS") == 0) {
                    c3->code->kind = IC_MINUS;
                } else if (strcmp(op->tokenName, "STAR") == 0) {
                    c3->code->kind = IC_MULTI;
                } else if (strcmp(op->tokenName, "DIV") == 0) {
                    c3->code->kind = IC_DIV;
                }
                return concatenate(3, c1, c2, c3);
            }
        } else if (strcmp(exp->childs[1]->tokenName, "Exp") == 0) {
            // Exp -> (Exp)
            return translate_Exp(exp->childs[1], place);
        } else if (strcmp(exp->childs[0]->tokenName, "Exp") == 0) {
            // Exp -> Exp DOT ID
            // get the address of the structure
            Operand baseAddr = new_addresss();
            CodeList c1 = NULL, c2 = NULL;
            
            if (exp->childs[0]->childNum == 1 && exp->childs[0]->childs[0]->type == LEXICAL_TOKEN_ID) {
                // exp->childs[0] -> ID
                Operand s = getVarByName(exp->childs[0]->childs[0]->tokenName)->op;
                c1 = NULL;
                
                if (s->para == 1) {
                    c2 = createCodeAssign(baseAddr, s); // baseAddr = s;   
                } else {
                    c2 = createCodeRef(baseAddr, s);   // baseAddr = &s;
                }
            } else {
                c1 = translate_Exp(exp->childs[0], baseAddr);
            }
            
            // calculate the address of the field
            Operand addr = new_addresss();
            Operand offset = new_temp();
            CodeList c3 = getOffsetOfField(exp->childs[0]->semanticType, exp->childs[2]->tokenName, offset);
            CodeList c4 = createBinOp(addr, baseAddr, offset); // addr = baseAddr + offset
            c4->code->kind = IC_PLUS;

            CodeList c5;
            if (place->kind == Em_ADDRESS)
                c5 = createCodeAssign(place, addr);     // place = addr
            else
                c5 = createCodeDeref(place, addr);      // place = *addr
            return concatenate(5, c1, c2, c3, c4, c5);
        } else if (exp->childs[0]->type == LEXICAL_TOKEN_ID){
            // Exp -> ID()
            Item funcItem = findItem(funcTable, exp->childs[0]->tokenName);
            if (strcmp(funcItem->name, "read") == 0) return createCodeRead(place);
            return createCodeCall(funcItem->name, place);
        }
    } else if (exp->childNum == 2) {
        node_t* op = exp->childs[0];
        if (strcmp(op->tokenName, "MINUS") == 0) {
            // Exp -> MINUS Exp
            Operand t1 = new_temp();
            CodeList c1 = translate_Exp(exp->childs[1], t1);
            CodeList c2 = createBinOp(place, new_const(0), t1);
            c2->code->kind = IC_MINUS;
            return concatenate(2, c1, c2);
        } else {
            // Exp -> NOT Exp
            Operand trueLabel = new_label();
            Operand falseLabel = new_label();
            CodeList c0 = createCodeAssign(place, new_const(0));
            CodeList c1 = translate_Cond(exp, trueLabel, falseLabel);

            return concatenate(
                5,
                c0,
                c1,
                label2CodeList(trueLabel),
                createCodeAssign(place, new_const(1)),
                label2CodeList(falseLabel)
            );
        }
    } else if (exp->childNum == 4) {
        if (exp->childs[0]->type == LEXICAL_TOKEN_ID) {
            // Exp -> ID(Args)
            Item funcItem = findItem(funcTable, exp->childs[0]->tokenName);
            ArgList argList = NULL;
            CodeList c1 = translate_Args(exp->childs[2], &argList);

            if (strcmp(funcItem->name, "write") == 0)
                return concatenate(
                    3,
                    c1,
                    createCodeWrite(argList->args), 
                    createCodeAssign(place, new_const(0)));

            CodeList c2 = NULL;
            int argCount = 0;
            while (argList != NULL) {
                // c2 = concatenate(2, c2, createCodeArg(argList->args));
                // 逆顺序的args序列
                argCount++;
                c2 = concatenate(2, createCodeArg(argList->args, argCount), c2);
                argList = argList->next;
            }
            CodeList c3 = createCodeCall(funcItem->name, place);
            return concatenate(3, c1, c2, c3);
        } else {
            // Exp -> Exp [Exp]
            // get the address of the array
            Operand baseAddr = new_addresss();
            CodeList c1 = NULL, c2 = NULL;
            
            if (exp->childs[0]->childNum == 1 && exp->childs[0]->childs[0]->type == LEXICAL_TOKEN_ID) {
                // exp->childs[0] -> ID
                Operand s = getVarByName(exp->childs[0]->childs[0]->tokenName)->op;
                c1 = NULL;
                c2 = createCodeRef(baseAddr, s);   // baseAddr = &s;
            } else {
                c1 = translate_Exp(exp->childs[0], baseAddr);
                // exp->childs[0] -> Exp[Exp]
                if (exp->childs[0]->childNum == 4 && strcmp(exp->childs[0]->childs[1]->tokenName, "LB") == 0)
                    irErrorFlag = 1; // multi-dimensional array type
            }
            
            // calculate the offset
            Operand index = new_temp();
            Operand addr = new_addresss();
            Operand offset = new_temp();
            CodeList c3 = concatenate(2, translate_Exp(exp->childs[2], index), getOffsetByIdx(exp->childs[0]->semanticType, index, offset));
            CodeList c4 = createBinOp(addr, baseAddr, offset); // addr = baseAddr + offset
            c4->code->kind = IC_PLUS;

            CodeList c5;
            if (place->kind == Em_ADDRESS)
                c5 = createCodeAssign(place, addr);     // place = addr
            else
                c5 = createCodeDeref(place, addr);      // place = *addr
            return concatenate(5, c1, c2, c3, c4, c5);
        }
    }
}

CodeList getOffsetByIdx(Type type, Operand idx, Operand place) {
    assert(type->kind == ARRAY);
    Operand size = new_const(getTypeSize(type->u.array.elem));
    CodeList c1 = createBinOp(place, idx, size);
    c1->code->kind = IC_MULTI;  // place = idx*size
    return c1;
}

CodeList getOffsetOfField(Type type, char* name, Operand place) {
    assert(type->kind == STRUCTURE);
    int size = 0;
    FieldList field = type->u.structure;
    while (field != NULL && strcmp(field->name, name) != 0) {
        size += getTypeSize(field->type);
        field = field->tail;
    }
    CodeList c1 =createCodeAssign(place, new_const(size));
    return c1;
}

CodeList translate_Args(node_t* args, ArgList* argList) {
    // Args -> Exp COMMA Args | Exp
    CodeList c = NULL, c1 = NULL;
    ArgList tail = *argList;
    Operand arg = NULL;

    while (args != NULL) {
        node_t* exp = args->childs[0];
        
        if (exp->semanticType->kind == ARRAY) {
            irErrorFlag = 1; //parameters of array type
        } else if (exp->semanticType->kind == STRUCTURE) {
            Operand addr = new_addresss();
            c1 = translate_Exp(exp, addr);
            arg = addr;
        } else {
            Operand tmp = new_temp();
            c1 = translate_Exp(exp, tmp);
            arg = tmp;
        }
        c = concatenate(2, c, c1);
        // add the arg to argList
        if (tail == NULL) {
            *argList = new_arglist();
            tail = *argList;
            tail->args = arg;
        } else {
            tail->next = new_arglist();
            tail = tail->next;
            tail->args = arg;
        }
        if (args->childNum == 1) break;
        if (args->childNum == 3) args = args->childs[2];
    }
    return c;
}

CodeList translate_Cond(node_t* exp, Operand trueLabel, Operand falseLabel) {
    if (exp->childNum == 3) {
        // Exp -> Exp RELOP Exp | Exp OR Exp | Exp AND Exp | others
        node_t* op = exp->childs[1];
        if (strcmp(op->tokenName, "AND") == 0) {
            Operand label1 = new_label();
            CodeList c1 = translate_Cond(exp->childs[0], label1, falseLabel);
            CodeList c2 = translate_Cond(exp->childs[2], trueLabel, falseLabel);
            return concatenate(3, c1, label2CodeList(label1), c2);
        } else if (strcmp(op->tokenName, "OR") == 0) {
            Operand label1 = new_label();
            CodeList c1 = translate_Cond(exp->childs[0], trueLabel, label1);
            CodeList c2 = translate_Cond(exp->childs[2], trueLabel, falseLabel);
            return concatenate(3, c1, label2CodeList(label1), c2);
        } else if (strcmp(op->tokenName, "RELOP") == 0) {
            // RELOP
            Operand t1 = new_temp();
            Operand t2 = new_temp();
            CodeList c1 = translate_Exp(exp->childs[0], t1);
            CodeList c2 = translate_Exp(exp->childs[2], t2);
            CodeList c3 = createCodeCondGOTO(trueLabel, t1, t2, getRELOP(exp));
            return concatenate(4, c1, c2, c3, createCodeGOTO(falseLabel));
        } else {
            // others
            Operand t1 = new_temp();
            CodeList c1 = translate_Exp(exp, t1);
            CodeList c2 = createCodeCondGOTO(trueLabel, t1, new_const(0), "!=");
            return concatenate(3, c1, c2, createCodeGOTO(falseLabel));
        }
    } else if (exp->childNum == 2 && strcmp(exp->childs[0]->tokenName, "NOT") == 0) {
        // Exp -> NOT Exp
        return translate_Cond(exp->childs[1], falseLabel, trueLabel);
    } else {
        Operand t1 = new_temp();
        CodeList c1 = translate_Exp(exp, t1);
        CodeList c2 = createCodeCondGOTO(trueLabel, t1, new_const(0), "!=");
        return concatenate(3, c1, c2, createCodeGOTO(falseLabel));
    }
}

CodeList label2CodeList(Operand label) {
    CodeList c = new_CodeList();
    c->code->kind = IC_LABEL;
    c->code->u.label = label;
    return c;
}

Variable getVarByName(char* name) {
    // find varible in varible list
    if (var_head == NULL) {
        var_head = new_var(name);
        var_tail = var_head;
        return var_tail;
    } else {
        Variable walk = var_head;
        while (walk != NULL) {
            if (strcmp(walk->name, name) == 0) {
                return walk;
            }
            if (walk == var_tail) break;
            walk = walk->next;
        }
        var_tail->next = new_var(name);
        var_tail = var_tail->next;
        return var_tail;
    }
}

Operand new_temp() {
    Operand temp = new_operand();
    temp->kind = Em_TEMP;
    temp->u.tempno = temp_num++;
    temp->type = BASIC;
    return temp;
}

Operand new_label() {
    Operand label = new_operand();
    label->kind = Em_LABEL;
    label->u.labelno = label_num++;
    return label;
}

Operand new_const(int value) {
    // -1, 0, 1
    if (value == 0) return const0;
    if (value == 1) return const1p;
    if (value == -1) return const1m;

    Operand cst = new_operand();
    cst->kind = Em_CONSTANT;
    cst->u.val = value;
    cst->type = BASIC;
    return cst;
}

Operand new_addresss() {
    Operand addr = new_operand();
    addr->kind = Em_ADDRESS;
    addr->u.tempno = temp_num++;
    addr->type = BASIC;
    return addr;
}

void connect(CodeList c1, CodeList c2) {
    if (c1 == NULL) return;
    // get the tail of c1
    CodeList walk = c1;
    while (walk != NULL && walk->next != NULL) walk = walk->next;
    walk->next = c2;
    c2->prev = walk;
}

CodeList concatenate(int num, ...) {
    va_list args;
    va_start(args, num);

    CodeList head, tail;
    head = tail = va_arg(args, CodeList);
    for (int i = 1; i < num; i++) {
        CodeList tmp = va_arg(args, CodeList);
        if (tmp == NULL) continue;
        if (head == NULL) {
            head = tmp;
            tail = tmp;
        } else {
            connect(tail, tmp);
            tail = tmp;
        }
    }
    va_end(args);

    return head;
}

CodeList createBinOp(Operand des, Operand op1, Operand op2) {
    CodeList c = new_CodeList();
    c->code->u.binop.op1 = op1;
    c->code->u.binop.op2 = op2;
    c->code->u.binop.result = des;
    return c;
}

CodeList createCodeRet(Operand op) {
    if (op == NULL) return NULL;

    CodeList c = new_CodeList();
    c->code->kind = IC_RETURN;
    c->code->u.op = op;
    return c;
}

CodeList createCodeGOTO(Operand label) {
    if (label == NULL) return NULL;

    CodeList c = new_CodeList();
    c->code->kind = IC_JUMP;
    c->code->u.jump.label = label;
    return c;
}

CodeList createCodeCondGOTO(Operand label, Operand op1, Operand op2, char* relop) {
    if (label == NULL || op1 == NULL || op2 == NULL) return NULL;

    CodeList c = new_CodeList();
    c->code->kind = IC_CONDJUMP;
    c->code->u.condjump.label = label;
    c->code->u.condjump.op1 = op1;
    c->code->u.condjump.op2 = op2;
    c->code->u.condjump.relop = relop;
    return c;
}

CodeList createCodeAssign(Operand left, Operand right) {
    if (left == NULL || right == NULL) return NULL;
    
    if (left->type != NULL && left->type->kind == ARRAY 
        && right->type != NULL && right->type->kind == ARRAY
        && left->kind == Em_VARIABLE
        && right->kind == Em_VARIABLE) {
        // array assign
        int size = left->type->u.array.size;
        if (size > right->type->u.array.size) {
            size = right->type->u.array.size;
        }
        Operand offset = new_temp();
        Operand addressL = new_addresss(), addressR = new_addresss();
        Operand elemAddrL = new_addresss(), elemAddrR = new_addresss();
        CodeList c = createCodeRef(addressL, left);
        c = concatenate(2, c, createCodeRef(addressR, right));
        Operand mid = new_temp();

        for (int i = 0; i < size; i++) {
            // get the offset
            Operand idx = new_const(i);
            Operand elemSize = new_const(getTypeSize(left->type->u.array.elem));
            CodeList c1 = createBinOp(offset, idx, elemSize);
            c1->code->kind = IC_MULTI;

            // get the address
            CodeList c2 = createBinOp(elemAddrL, addressL, offset);
            c2->code->kind = IC_PLUS;
            CodeList c3 = createBinOp(elemAddrR, addressR, offset);
            c3->code->kind = IC_PLUS;
            CodeList c4 = createCodeDeref(mid, elemAddrR);
            CodeList c5 = createCodeDeref(elemAddrL, mid);
            c = concatenate(6, c, c1, c2, c3, c4, c5);
        }
        return c;
    } else if (left->type != NULL && left->type->kind == STRUCTURE 
            && right->type != NULL && right->type->kind == STRUCTURE
            && left->kind == Em_VARIABLE
            && right->kind == Em_VARIABLE) {
        // structure assign
    } else {
        // basic assign
        CodeList c = new_CodeList();
        c->code->kind = IC_ASSIGN;
        c->code->u.assign.left = left;
        c->code->u.assign.right = right;
        return c;
    }
}

CodeList createCodeDeref(Operand des, Operand src) {
    if (des == NULL || src == NULL) return NULL;

    CodeList c = new_CodeList();
    c->code->kind = IC_DEREF;
    c->code->u.deref.left = des;
    c->code->u.deref.right = src;
    return c;
}

CodeList createCodeRef(Operand des, Operand src) {
    if (des == NULL || src == NULL) return NULL;
    if (des->kind == Em_TEMP || des->kind == Em_ADDRESS) {
        if (src->kind == Em_TEMP || src->kind == Em_ADDRESS)
            assert(0);
    }
    
    CodeList c = new_CodeList();
    c->code->kind = IC_REF;
    c->code->u.ref.left = des;
    c->code->u.ref.right = src;
    return c;
}

CodeList createCodeCall(char *funcName, Operand des) {
    if (des == NULL) return NULL;

    CodeList c = new_CodeList();
    c->code->kind = IC_CALL;
    c->code->u.call.func = funcName;
    c->code->u.call.result = des;
    return c;
}

CodeList createCodeArg(Operand op, int num) {
    if (op == NULL) return NULL;

    CodeList c = new_CodeList();
    c->code->kind = IC_ARG;
    c->code->u.arg.arg = op;
    c->code->u.arg.number = num;
    return c;
}

CodeList createCodeParam(Operand op, int num) {
    if (op == NULL) return NULL;

    CodeList c = new_CodeList();
    c->code->kind = IC_PARAM;
    c->code->u.param.number = num;
    c->code->u.param.param = op;
    return c;
}

CodeList createCodeRead(Operand op) {
    if (op == NULL) return NULL;

    CodeList c = new_CodeList();
    c->code->kind = IC_READ;
    c->code->u.op = op;
    return c;
}

CodeList createCodeWrite(Operand op) {
    if (op == NULL) return NULL;

    CodeList c = new_CodeList();
    c->code->kind = IC_WRITE;
    c->code->u.op = op;
    return c;
}

CodeList createCodeFunc(char* funcName) {
    CodeList c = new_CodeList();
    c->code->kind = IC_FUNC;
    c->code->u.func = funcName;
    return c;
}

CodeList createCodeDec(Operand op, int size) {
    if (op == NULL) return NULL;

    CodeList c = new_CodeList();
    c->code->kind = IC_DEC;
    c->code->u.dec.op1 = op;
    c->code->u.dec.size = size;
    return c;
}

char* getRELOP(node_t* exp) {
    if (exp == NULL) return NULL;
    if (exp->childNum != 3) return NULL;

    // Exp -> Exp RELOP Exp
    node_t* relop = exp->childs[1];
    char* opName = relop->childs[0]->tokenName;
    char* res = (char*)calloc(4, sizeof(char)); 
    memset(res, 0, 4);
    if (strcmp(opName, "EQ") == 0) {
        res[0] = '=';
        res[1] = '=';
    } else if (strcmp(opName, "NE") == 0) {
        res[0] = '!';
        res[1] = '=';
    } else if (strcmp(opName, "LT") == 0) {
        res[0] = '<';
    } else if (strcmp(opName, "BT") == 0) {
        res[0] = '>';
    } else if (strcmp(opName, "BE") == 0) {
        res[0] = '>';
        res[1] = '=';
    } else if (strcmp(opName, "LE") == 0) {
        res[0] = '<';
        res[1] = '=';
    } else {
        free(res);
        return NULL;
    }

    return res;
}

Operand new_operand() {
    Operand op = (Operand)malloc(sizeof(struct _Operand));
    op->para = 0;
    op->name = NULL;
    return op;
}

char* op2String(Operand op) {
    if (op == NULL) return NULL;
    if (op->name != NULL) return op->name;

    char* res = (char*)calloc(8, sizeof(char));
    memset(res, 0, 8);
    switch (op->kind)
    {
    case Em_ADDRESS:
    case Em_TEMP:
        sprintf(res, "t%d", op->u.tempno);
        break;
    
    case Em_CONSTANT:
        sprintf(res, "#%d", op->u.val);
        break;

    case Em_VARIABLE:
        sprintf(res, "v%d", op->u.varno);
        break;

    case Em_LABEL:
        sprintf(res, "label%d", op->u.labelno);
        break;

    default:
        free(res);
        return NULL;
        break;
    }
    
    op->name = res;
    return res;
}

InterCode new_InterCode() {
    InterCode inter = (InterCode)malloc(sizeof(struct _InterCode));
    return inter;
}

CodeList new_CodeList() {
    CodeList c = (CodeList)malloc(sizeof(struct _CodeList));
    c->code = new_InterCode();
    c->next = c->prev = NULL;
    return c;
}

ArgList new_arglist() {
    ArgList arglist = (ArgList)malloc(sizeof(struct _ArgList));
    arglist->next = NULL;
    return arglist;
}

Variable new_var(char* name) {
    Variable var = (Variable)malloc(sizeof(struct _Variable));
    var->name = name;
    var->next = NULL;
    var->op = new_operand();
    var->op->kind = Em_VARIABLE;
    var->op->u.varno = var_num++;
    return var;
}
        
void print_IR(CodeList head, FILE* file) {
    if (irErrorFlag == 1) {
        printf("Cannot translate\n");
        return;
    }
    CodeList walk = head;
    InterCode interCode;
    while (walk != NULL) {
        interCode = walk->code;
        // ic to string
        switch (interCode->kind)
        {
        case IC_ASSIGN: 
            fprintf(file, "%s := %s\n", 
                op2String(interCode->u.assign.left), 
                op2String(interCode->u.assign.right));
            break;
        
        case IC_LABEL:
            fprintf(file, "LABEL %s :\n", op2String(interCode->u.label));
            break;
        
        case IC_PLUS:
            fprintf(file, "%s := %s + %s\n",
                op2String(interCode->u.binop.result),
                op2String(interCode->u.binop.op1),
                op2String(interCode->u.binop.op2));
            break;

        case IC_MINUS:
            fprintf(file, "%s := %s - %s\n",
                op2String(interCode->u.binop.result),
                op2String(interCode->u.binop.op1),
                op2String(interCode->u.binop.op2));
            break;
        
        case IC_MULTI:
            fprintf(file, "%s := %s * %s\n",
                op2String(interCode->u.binop.result),
                op2String(interCode->u.binop.op1),
                op2String(interCode->u.binop.op2));
            break;
        
        case IC_DIV:
            fprintf(file, "%s := %s / %s\n",
                op2String(interCode->u.binop.result),
                op2String(interCode->u.binop.op1),
                op2String(interCode->u.binop.op2));
            break;

        case IC_CALL:
            fprintf(file, "%s := CALL %s\n",
                op2String(interCode->u.call.result),
                interCode->u.call.func);
            break;

        case IC_ARG:
            fprintf(file, "ARG %s\n",
                op2String(interCode->u.arg.arg));
            break;
        
        case IC_PARAM:
            fprintf(file, "PARAM %s\n",
                op2String(interCode->u.param.param));
            break;
        
        case IC_READ:
            fprintf(file, "READ %s\n",
                op2String(interCode->u.op));
            break;
        
        case IC_WRITE:
            fprintf(file, "WRITE %s\n",
                op2String(interCode->u.op));
            break;

        case IC_RETURN:
            fprintf(file, "RETURN %s\n",
                op2String(interCode->u.op));
            break;
        
        case IC_FUNC:
            fprintf(file, "FUNCTION %s :\n", interCode->u.func);
            break;
        
        case IC_JUMP:
            fprintf(file, "GOTO %s\n", op2String(interCode->u.jump.label));
            break;

        case IC_CONDJUMP:
            fprintf(file, "IF %s %s %s GOTO %s\n", 
                op2String(interCode->u.condjump.op1),
                interCode->u.condjump.relop,
                op2String(interCode->u.condjump.op2),
                op2String(interCode->u.condjump.label)
                );
            break;

        case IC_DEREF:
            if (interCode->u.deref.left->kind == Em_ADDRESS) {
                fprintf(file, "*%s := %s\n",
                    op2String(interCode->u.deref.left),
                    op2String(interCode->u.deref.right));
            } else {
                fprintf(file, "%s := *%s\n",
                    op2String(interCode->u.deref.left),
                    op2String(interCode->u.deref.right));
            }
            break;

        case IC_REF:
            fprintf(file, "%s := &%s\n",
                op2String(interCode->u.ref.left),
                op2String(interCode->u.ref.right));
            break;
        
        case IC_DEC:
            fprintf(file, "DEC %s %d\n",
                op2String(interCode->u.dec.op1),
                interCode->u.dec.size);
            break;

        default:
            break;
        }
        walk = walk->next;
    }
}

int isConst(node_t* exp) {
    if (exp == NULL) return 0;
    if (strcmp(exp->tokenName, "Exp") != 0) return 0;
    if (exp->childNum != 1) return 0;
    if (exp->childs[0]->type != LEXICAL_TOKEN_NUMBER) return 0;
    return 1;
}

Operand getConst(node_t* exp) {
    assert(isConst(exp) == 1);
    return new_const(atoi(exp->childs[0]->childs[0]->tokenName));
}

int isVar(node_t* exp) {
    if (exp == NULL) return 0;
    if (strcmp(exp->tokenName, "Exp") != 0) return 0;
    if (exp->childNum != 1) return 0;
    if (exp->childs[0]->type != LEXICAL_TOKEN_ID) return 0;
    return 1;
}

Operand getVar(node_t* exp) {
    assert(isVar(exp) == 1);
    return getVarByName(exp->childs[0]->tokenName)->op;
}