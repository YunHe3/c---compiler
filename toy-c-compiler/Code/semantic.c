/** 
 * error type
 *  1. 变量在使用时未经定义
 *  2. 函数在调用时未经定义
 *  3. 变量出现重复定义，或变量与前面定义过的结构体名字重复
 *  4. 函数出现重复定义(即同样的函数名出现了不止一次定义)
 *  5. 赋值号两边的表达式类型不匹配
 *  6. 赋值号左边出现一个只有右值的表达式
 *  7. 操作数类型不匹配或操作数类型与操作符不匹配(例如整型变量与数组变量相加减，或数组(或结构体)变量与数组(或结构体)变量相加减)
 *  8. return语句的返回类型与函数定义的返回类型不匹配
 *  9. 函数调用时实参与形参的数目或类型不匹配
 *  10. 对非数组型变量使用“[...]”(数组访问)操作符
 *  11. 对普通变量使用“(...)”或“()”(函数调用)操作符
 *  12. 数组访问操作符“[...]”中出现非整数(例如a[1.5])
 *  13. 对非结构体型变量使用“.”操作符
 *  14. 访问结构体中未定义过的域
 *  15. 结构体中域名重复定义(指同一结构体中)，或在定义时对域进行初始化(例如struct A { int a = 0; })
 *  16. 结构体的名字与前面定义过的结构体或变量的名字重复
 *  17. 直接使用未定义过的结构体来定义变量
 * 
 *  18. 函数声明但未定义
 *  19. 函数两次声明之间冲突或声明与定义冲突
*/ 

/*-------------------------------------------------------------------*/
/*------------------------ Macro and Include ------------------------*/
/*-------------------------------------------------------------------*/
#include "semantic.h"

/*-------------------------------------------------------------------*/
/*------------------------- Type System -----------------------------*/
/*-------------------------------------------------------------------*/

Type createFloatType() {
    Type type = (Type)malloc(sizeof(struct Type_));
    type->kind = BASIC;
    type->u.basic = FLOAT;
    type->parType = NULL;
    return type;
}

Type createIntType() {
    Type type = (Type)malloc(sizeof(struct Type_));
    type->kind = BASIC;
    type->u.basic = INT;
    type->parType = NULL;
    return type;
}

Type createStructureType() {
    Type type = (Type)malloc(sizeof(struct Type_));
    type->kind = STRUCTURE;
    type->parType = NULL;
    return type;
}

Type createArrayType(Type ele, int size) {
    Type type = (Type)malloc(sizeof(struct Type_));
    type->kind = ARRAY;
    type->u.array.elem = ele;
    type->u.array.size = size;
    type->parType = NULL;
    return type;
}

int typecmp(Type t1, Type t2) {
    if (t1 == NULL || t2 == NULL) return 1;
    if (t1->kind == t2->kind) {
        if (t1->kind == STRUCTURE) {
            return fieldcmp(t1->u.structure, t2->u.structure);
            // return (t1 == t2) ? 0 : 1; // only name equal
        } else if (t1->kind == ARRAY) {
            return typecmp(t1->u.array.elem, t2->u.array.elem);
        } else if (t1->kind == BASIC) {
            return (t1->u.basic == t2->u.basic) ? 0 : 1;
        }
    } else {
        return 1;
    }
}

FieldList createFieldList() {
    FieldList fieldList = (FieldList)malloc(sizeof(struct FieldList_));
    fieldList->tail = NULL;
    fieldList->name = (char*)calloc(MAX_NAME_LENGTH, sizeof(char));
    fieldList->lineNo = 0;
    return fieldList;
}

// return 0 if two field have the same type list, otherwise 1
int fieldcmp(FieldList f1, FieldList f2) {
    FieldList walk1 = f1;
    FieldList walk2 = f2;
    while (walk1 != NULL && walk2 != NULL) {
        if (typecmp(walk1->type, walk2->type) != 0) return 1;
        walk1 = walk1->tail;
        walk2 = walk2->tail;
    }
    if (walk1 != NULL || walk2 != NULL) return 1;
    return 0;
}

/*-------------------------------------------------------------------*/
/*------------------------- Symbol Table ----------------------------*/
/*-------------------------------------------------------------------*/

SymTable createTable() {
    SymTable table = (SymTable)malloc(sizeof(struct SymTable_)); 
    for (int i = 0; i < TABLESIZE; i++) {
        table->items[i] = NULL;
    }
    return table;
}

void cleanTable(SymTable table) {
    for (int i = 0; i < TABLESIZE; i++) {
        Item item = table->items[i];
        Item freeHelper;
        while (item != NULL) {
            freeHelper = item;
            item = item->tail;
            free(freeHelper);
        }
    }
}

unsigned int hash_pjw(char* name) {
    unsigned int val = 0, i;
    for (; *name; ++name)
        {
           val = (val << 2) + *name;
           if (i = val & ~TABLESIZE) val = (val ^ (i >> 12)) & 0x3fff;
        }
    return val;
}

// return 1 if insert successfully, otherwise 0 
// -1 if in the same structure
// 19 for error type 19: Inconsistent declaration of function
int insertItem(SymTable table, Item item) {
    int slot = hash_pjw(item->name);
    Item des = table->items[slot];
    Item tail = NULL;

    while (des != NULL) {
        if (strcmp(des->name, item->name) == 0) {
            // two same name
            // casa 1: function def and dec
            if (table->kind == FUNC) {
                if (des->funcStatus == item->funcStatus && des->funcStatus == DEF) {
                    // both DEF, func redefine error
                    return 0;
                } else {
                    // 1. one DEC and another DEF or 2. both DEC
                    // check the params
                    FieldList params1 = des->func.params;
                    FieldList params2 = item->func.params;
                    if (fieldcmp(params1, params2) == 0) {
                        // if both DEC, set the des's status to DEC
                        // otherwise, set to DEF
                        if (des->funcStatus == item->funcStatus) {
                            des->funcStatus = DEC;
                        } else {
                            des->funcStatus = DEF;
                        }
                        return 1;
                    } else {
                        return 19;
                    }
                }
            } 

            // case 2: nested type
            if (des->type->parType != NULL
                && item->type->parType != NULL
                && des->type->parType == item->type->parType) return -1;

            // case 3: function's params
            if (des->func.funcItem != NULL
                && item->func.funcItem != NULL
                && strcmp(des->func.funcItem->name, item->func.funcItem->name) == 0) return 1;
                
            // case 4: name conflict
            return 0;
        }
        tail = des;
        des = des->tail;
    }

    if (des == NULL) {
        table->items[slot] = item;
    } else {
        tail->tail = item;
    }
    return 1;
}

Item findItem(SymTable table, char* name) {
    int slot = hash_pjw(name);
    Item des = table->items[slot];

    while (des != NULL && strcmp(des->name, name) == 1) {
        des = des->tail;
    }
    return des;
}

Item createItem() { 
    Item item = (Item)malloc(sizeof(struct Item_));
    item->name = (char*)calloc(MAX_NAME_LENGTH, sizeof(char));
    item->tail = NULL;
    item->funcStatus = DEC;
    item->func.funcItem = NULL;
    return item; 
}

/*-------------------------------------------------------------------*/
/*----------------------- Semantic Analyze --------------------------*/
/*-------------------------------------------------------------------*/

SymTable structureTable;
SymTable varTable;
SymTable funcTable;
int errTable[MAX_LINE];
int seed;

void semantic(node_t* program) {

    /* Initialze */
    structureTable = createTable();
    varTable = createTable();
    funcTable = createTable();

    structureTable->kind = STRUCTURE;
    varTable->kind = VAR;
    funcTable->kind = FUNC;

    for (int i = 0; i < MAX_LINE; i++) {
        errTable[i] = 0;
    }

    seed = 1;

    // add write and read function to symbol table
    Item funcWrite = createItem();
    Item funcRead = createItem();
    sprintf(funcRead->name, "read");
    sprintf(funcWrite->name, "write");
    funcRead->kind = funcWrite->kind = FUNC;
    funcRead->funcStatus = funcWrite->funcStatus = DEF;
    funcRead->type = funcWrite->type = createIntType();
    funcRead->func.params = NULL;
    FieldList p = createFieldList();
    p->name = "out";
    p->type = createIntType();
    p->tail = NULL;
    funcWrite->func.params = p;
    insertItem(funcTable, funcWrite);
    insertItem(funcTable, funcRead);

    /* Scan */
    Program(program);

    /* check if there are dec-but-undef function */
    dec_undef_check(funcTable);

    // // free resurose
    // cleanTable(structureTable);
    // cleanTable(funcTable);
    // cleanTable(varTable);
}

Type Program(node_t* program) {
    assert(strcmp(program->tokenName, "Program") == 0);

    // Program -> ExtDefList
    node_t* extdeflist = program->childs[0];
    ExtDefList(extdeflist);
    return NULL;
}

Type ExtDefList(node_t* extdeflist) {
    assert(strcmp(extdeflist->tokenName, "ExtDefList") == 0);

    // ExtDefList -> ExtDef ExtDefList | empty
    while (extdeflist->childNum > 0) {
        node_t* extdef = extdeflist->childs[0];
        ExtDef(extdef);
        extdeflist = extdeflist->childs[1];
    }

    return NULL;
}

// get type from the node 'Specifier'
Type Specifier(node_t* specifier) {
    assert(strcmp(specifier->tokenName, "Specifier") == 0);
    
    if (strcmp(specifier->childs[0]->tokenName, "TYPE") == 0) {
        // Specifier -> TYPE
        node_t* type = specifier->childs[0];
        if (strcmp(type->childs[0]->tokenName, "int") == 0) {
            // TYPE -> int
            Type t = createIntType();
            specifier->semanticType = t;
            return t;
        } else {
            // TYPE -> float
            Type t = createFloatType();
            specifier->semanticType = t;
            return t;
        }
    } else {
        // Specifier -> StructSpecifier
        node_t* structSpecifier = specifier->childs[0];
        if (structSpecifier->childNum == 2) {
            // StructSpecifier -> STRUCT Tag
            // look up for the struct symbol
            node_t* tag = structSpecifier->childs[1];
            Item structureItem = findItem(structureTable, tag->childs[0]->tokenName);
            if (structureItem == NULL) {
                // error type 17
                err("Undefine structure type", 17, specifier->lineNo);
            } else {
                specifier->semanticType = structureItem->type;
                return structureItem->type;
            }
        } else if (structSpecifier->childNum == 5) {
            // StructSpecifier -> STRUCT OptTag {DefList}
            node_t* deflist = structSpecifier->childs[3];
            node_t* opttag = structSpecifier->childs[1];
            Item item = createItem();
            if (opttag->childNum > 0) {
                node_t* id = opttag->childs[0];
                strcpy(item->name, id->tokenName);
            } else {
                randomNameGenerator(item->name);
            }
            item->kind = STRUCTURE;
            Type structType = createStructureType();
            FieldList fieldlist = NULL;
            FieldList tail = fieldlist;
            while (deflist->childNum > 0) {
                // DefList -> Def DefList | empty
                node_t* def = deflist->childs[0];

                // Def -> Specifier DecList SEMI
                node_t* spec = def->childs[0];
                node_t* declist = def->childs[1];
                Type type = Specifier(spec);

                // add each type to the field of struct item 
                // DecList -> Dec | Dec COMMA DecList
                while (declist != NULL) {
                    node_t* dec = declist->childs[0];
                    FieldList tmp = createFieldList();
                    type->parType = structType;
                    tmp->type = Dec(dec, type);
                    tmp->type->parType = structType;
                    // tmp->name = dec->decName;
                    strcpy(tmp->name, dec->decName);
                    if (fieldlist == NULL) {
                        fieldlist = tmp;
                        tail = fieldlist;
                    } else {
                        tail->tail = tmp;
                        tail = tail->tail;
                    }
                    if (declist->childNum == 1) break;
                    if (declist->childNum == 3) declist = declist->childs[2];
                }

                deflist = deflist->childs[1];
            }
            structType->u.structure = fieldlist;
            item->type = structType;
            item->lineNo = specifier->lineNo;
            // add struct item to struct symbol table
            // check the varTable and funcTable
            if (findItem(varTable, item->name) != NULL 
                || findItem(funcTable, item->name) != NULL
                || insertItem(structureTable, item) == 0) {
                // item conflict
                // error type 16
                err("Structure redefine", 16 ,specifier->lineNo);
                free(item);
                free(structType);
                structType = NULL;
            }
            specifier->semanticType = structType;
            return structType;
        }
    }
}

Type* Def(node_t* def) {
    assert(strcmp(def->tokenName, "Def") == 0 && def->childNum == 3);

    // Def -> Specifier DecList SEMI
    node_t* specifier = def->childs[0];
    node_t* declist = def->childs[1];
    Type type = Specifier(specifier);

    Type* types = (Type*)calloc(MAX_DECLIST_SIZE, sizeof(Type));
    int idx = 0;

    while (declist != NULL && declist->childNum == 3) {
        node_t* dec = declist->childs[0];
        types[idx++] = Dec(dec, type);
        declist = declist->childs[2];
    }

    types[idx] = NULL;
    return types;
}  

Type ExtDef(node_t* extdef) {
    assert(strcmp(extdef->tokenName, "ExtDef") == 0);
    node_t* specifier = extdef->childs[0];
    Type type = Specifier(specifier);

    if (extdef->childNum == 2) {
        // pass, struct def
        return NULL;
    }

    assert(extdef->childNum == 3);
    node_t* child2 = extdef->childs[1];
    node_t* child3 = extdef->childs[2];
    if (strcmp(child2->tokenName, "ExtDecList") == 0) {
        // ExtDef -> Specifier ExtDecList SEMI
        node_t* extdeclist = extdef->childs[1];
        ExtDecList(extdeclist, type);
    } else if (strcmp(child3->tokenName, "SEMI") == 0){
        // ExtDef -> Specifier FunDec SEMI
        node_t* fundec = extdef->childs[1];
        Item funcItem = FunDec(fundec, type, DEC);
        paramsHandler(funcItem);
    } else {
        // ExtDef -> Specifier FunDec CompSt
        node_t* fundec = extdef->childs[1];
        node_t* compst = extdef->childs[2];
        Item funcItem = FunDec(fundec, type, DEF);
        paramsHandler(funcItem);
        CompSt(compst, type);
    }

    return type;
}

Type ExtDecList(node_t* extdeclist, Type type) {
    assert(strcmp(extdeclist->tokenName, "ExtDecList") == 0);

    // ExtDecList -> VarDec | VarDec COMMA ExtDecList
    node_t* vardec = extdeclist->childs[0];
    VarDec(vardec, type, 0);
    if (extdeclist->childNum == 3) ExtDecList(extdeclist->childs[2], type);
    return NULL;
}

Type Dec(node_t* dec, Type type) {
    assert(strcmp(dec->tokenName, "Dec") == 0);

    // Dec -> VarDec | VarDec ASSIGNOP Exp
    node_t* vardec = dec->childs[0];
    Type varType = VarDec(vardec, type, 0);
    strcpy(dec->decName, vardec->decName);
    
    if (dec->childNum == 3) {
        // check if under struct, if so, no assign allow
        if (varType->parType != NULL && varType->parType->kind == STRUCTURE) {
            err("Can't initialze the variable in structure", 15, dec->lineNo);
        } else {
            node_t* exp = dec->childs[2];
            Type expType = Exp(exp);
            // check if (varType == expType)
            if (typecmp(expType, varType) != 0) {
                // error type 5
                err("Type mismatch", 5, dec->lineNo);
            }
        }
    }

    dec->semanticType = varType;
    return varType;
}

// return funcItem if the function dec successfully, otherwise NULL
Item FunDec(node_t* fundec, Type retType, int status) {
    assert(strcmp(fundec->tokenName, "FunDec") == 0);
    // FunDec -> ID (VarList) | ID()
    node_t* id = fundec->childs[0];

    Item item = createItem();
    strcpy(item->name, id->tokenName);
    item->kind = FUNC;
    item->type = retType;
    item->funcStatus = status;
    item->lineNo = fundec->lineNo;

    if (fundec->childNum == 4) {
        node_t* varlist = fundec->childs[2];
        VarList(varlist, item);
    }

    int ret = insertItem(funcTable, item);
    if (ret == 0) {
        // error type 4
        err("Function redefine", 4, fundec->lineNo);
        free(item);
        return NULL;
    } else if (ret == 19) {
        // error type 19
        err("Inconsistent declaration of function", 19, fundec->lineNo);
        free(item);
        return NULL;
    }
    return item;
}

Type VarDec(node_t* vardec, Type inhType, int isParam) {
    Type type = NULL;

    if (vardec->childNum == 1) {
        // VarDec -> ID
        
        node_t* id = vardec->childs[0];
        strcpy(vardec->decName, id->tokenName);
        type = inhType;
        // check if the vardec is params of function
        // only when the function def or dec successfuly
        // the variable item will be insert to table
        if (isParam) {
            vardec->semanticType = type;
            return type;
        }

        // create new var item
        Item item = createItem();
        item->kind = VAR;
        item->type = type;
        item->lineNo = vardec->lineNo;
        strcpy(item->name, id->tokenName);
        int ret = insertItem(varTable, item);
        
        if (ret == 0) {
            // insert fail
            // error type 3
            err("Variable redefine", 3, vardec->lineNo);
            free(item);
        } else if (ret == -1) {
            // error type 15
            err("Variable redefine in structure", 15, vardec->lineNo);
            free(item);
        }
    } else if (vardec->childNum == 4) {
        // VarDec -> VarDec LB INT RB
        node_t* subvardec = vardec->childs[0];
        int size = atoi(vardec->childs[2]->childs[0]->tokenName);
        Type arrayType = createArrayType(inhType, size);
        arrayType->parType = inhType;
        type = VarDec(subvardec, arrayType, isParam);
        strcpy(vardec->decName, subvardec->decName);
    }

    vardec->semanticType = type;
    return type;
}

void VarList(node_t* varlist, Item funcItem) {
    assert(strcmp(varlist->tokenName, "VarList") == 0);
    
    FieldList field = funcItem->func.params;
    while (varlist != NULL) {
        // VarList -> ParamDec COMMA VarList | ParamDec
        node_t* paramDec = varlist->childs[0];
        // PaarmDec -> Specifier VarDec
        node_t* specifier = paramDec->childs[0];
        node_t* vardec = paramDec->childs[1];

        Type type = Specifier(specifier);
        Type varType = VarDec(vardec, type, 1);
        
        // add vardec to func's params
        FieldList tmp = createFieldList();
        strcpy(tmp->name, vardec->decName);
        tmp->type = varType;
        tmp->lineNo = vardec->lineNo;

        if (field == NULL) {
            funcItem->func.params = tmp;
            field = funcItem->func.params;
        } else {
            field->tail = tmp;
            field = field->tail;
        }

        if (varlist->childNum == 1) break;
        if (varlist->childNum == 3) varlist = varlist->childs[2];
    }
}

Type CompSt(node_t* compst, Type type) {
    assert(strcmp(compst->tokenName, "CompSt") == 0);
    // LC DefList StmtList RC
    node_t* deflist = compst->childs[1];
    node_t* stmtlist = compst->childs[2];
    Type retType = NULL;

    // handle deflist
    while (deflist->childNum != 0) {
        node_t* def = deflist->childs[0];
        deflist = deflist->childs[1];

        assert(strcmp(def->tokenName, "Def") == 0);
        node_t* specifier = def->childs[0];
        node_t* declist = def->childs[1];
        Type type = Specifier(specifier);
        // DecList -> Dec | Dec COMMA DecList
        while (declist != NULL) {
            node_t* dec = declist->childs[0];
            Dec(dec, type);
            if (declist->childNum == 1) break;
            if (declist->childNum == 3) declist = declist->childs[2];
        }
    }

    // handle stmtlist
    while (stmtlist->childNum != 0) {
        node_t* stmt = stmtlist->childs[0];
        stmtlist = stmtlist->childs[1];
        Type tmp = Stmt(stmt, type);
        if (tmp != NULL)
            retType = tmp;
    }

    compst->semanticType = retType;
    return retType;
}

Type Stmt(node_t* stmt, Type type) {
    assert(strcmp(stmt->tokenName, "Stmt") == 0);
    Type retType = NULL;
    // Stmt -> Exp SEMI | CompSt | RETURN Exp SEMI
    //         IF LP Exp RP Stmt | IF LP Exp RP Stmt ELSE Stmt
    //         WHILE LP Exp RP Stmt
    if (stmt->childNum == 1) {
        // Stmt -> CompSt
        CompSt(stmt->childs[0], type);
    } else if (stmt->childNum == 2) {
        // Stmt -> Exp SEMI
        Exp(stmt->childs[0]);
    } else if (stmt->childNum == 3) {
        // Stmt -> RETURN Exp SEMI
        retType = Exp(stmt->childs[1]);
        if (typecmp(retType, type) != 0) {
            // error type 8
            err("Return type mismatch", 8, stmt->lineNo);
        }
    } else if (stmt->childNum == 5) {
        // Stmt -> IF LP Exp RP Stmt | WHILE LP Exp RP Stmt
        Exp(stmt->childs[2]);
        Stmt(stmt->childs[4], type);
    } else {
        // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        Exp(stmt->childs[2]);
        Stmt(stmt->childs[4], type);
        Stmt(stmt->childs[6], type);
    }

    stmt->semanticType =  retType;
    return retType;
}

Type Exp(node_t* exp) {
    assert(strcmp(exp->tokenName, "Exp") == 0);
    Type retType = NULL;
    if (exp->childNum == 3) {
        if (strcmp(exp->childs[0]->tokenName, "Exp") == 0 && 
            strcmp(exp->childs[2]->tokenName, "Exp") == 0) {
            // Exp -> Exp Op Exp
            node_t* exp1 = exp->childs[0];
            node_t* exp2 = exp->childs[2];
            node_t* op   = exp->childs[1];
            Type type1 = Exp(exp1);
            Type type2 = Exp(exp2);
            
            if (typecmp(type1, type2) == 1) { // check if two exp type equal
                if (strcmp(op->tokenName, "ASSIGNOP") == 0) {
                    // error type 5
                    err("Operations's type mismatch", 5, exp->lineNo);
                } else {
                    // error type 7
                    err("Operations's type mismatch", 7, exp->lineNo);
                }  
            } else if (strcmp(op->tokenName, "ASSIGNOP") != 0 && type1->kind != BASIC){ // check if two exp can be calculate
                err("Invaild operator", 7, exp->lineNo);
            } else if (strcmp(op->tokenName, "ASSIGNOP") == 0 && isLHS(exp1) == 0) { // check if a rhs exp
                // error type 6
                err("Can't assign to a non-left-hand side variable", 6, exp->lineNo);
            } else {
                exp->isLHS = 1;
                retType = type1;
            }
        } else if (strcmp(exp->childs[1]->tokenName, "Exp") == 0) {
            // Exp -> (Exp)
            retType = Exp(exp->childs[1]);
            exp->isLHS = exp->isLHS;
        } else if (exp->childs[0]->type == LEXICAL_TOKEN_ID) {
            // Exp -> ID()
            // check if the func call is vaild
            node_t* id = exp->childs[0];
            Item funcItem = findItem(funcTable, id->tokenName);
            if (funcItem == NULL) {
                // error type 2
                err("Undefine function", 2, exp->lineNo);
            } else {
                retType = funcItem->type;
            }
        } else if (strcmp(exp->childs[0]->tokenName, "Exp") == 0 &&
                   strcmp(exp->childs[1]->tokenName, "DOT") == 0) {
            // Exp -> Exp.ID
            node_t* exp1 = exp->childs[0];
            node_t* id = exp->childs[2];
            // check if the Exp is a struct(vaild member call)
            Type type1 = Exp(exp1);
            if (type1 == NULL) {
                // error type 1
                err("Undefine variable", 1, exp->lineNo);
            } else if (type1->kind != STRUCTURE) {
                // error type 13
                err("Invaild operation '.' for non-structure", 13, exp->lineNo);
            } else {
                // get the field type
                FieldList fieldlist = type1->u.structure;
                while (fieldlist != NULL && strcmp(fieldlist->name, id->tokenName) != 0) {
                    fieldlist = fieldlist->tail;
                }
                if (fieldlist == NULL) {
                    // error type 14
                    err("Undefine field in structure", 14, exp->lineNo);
                } else {
                    exp->isLHS = 1;
                    retType = fieldlist->type;
                }
            }
        }
    } else if (exp->childNum == 2) {
        node_t* op = exp->childs[0];
        node_t* exp1 = exp->childs[1];

        retType = Exp(exp1);
    } else if (exp->childNum == 1) {
        if (exp->childs[0]->type == LEXICAL_TOKEN_ID) {
            // Exp -> ID
            node_t* id = exp->childs[0];
            Item varItem = findItem(varTable, id->tokenName);
            if (varItem == NULL) {
                // error type 1
                err("Undefine variable", 1, exp->lineNo);
            } else {
                exp->isLHS = 1;
                retType = varItem->type;
            }
        } else if (exp->childs[0]->type == LEXICAL_TOKEN_NUMBER) {
            // Exp -> INT | FLOAT
            node_t* num = exp->childs[0];
            if (strcmp(num->tokenName, "FLOAT") == 0) {
                retType = createFloatType();
            } else {
                retType = createIntType();
            }
        }
    } else if (exp->childNum == 4) {
        if (exp->childs[0]->type == LEXICAL_TOKEN_ID) {
            // Exp -> ID(Args)
            // check if the func call is vaild
            Item funcItem = findItem(funcTable, exp->childs[0]->tokenName);
            if (funcItem == NULL) {
                Item varItem = findItem(varTable, exp->childs[0]->tokenName);
                if (varItem == NULL) {
                    // error type 2
                    err("Undefine function", 2, exp->lineNo);
                } else {
                    // error type 11
                    err("Invaild operation '()'", 11, exp->lineNo);
                }
            } else {
                FieldList params = funcItem->func.params;
                FieldList walk = params;
                node_t* args = exp->childs[2];
                // Args -> Exp COMMA Args | Exp

                node_t* exp1 = args->childs[0];
                Type type1 = Exp(exp1);
                while (args->childNum > 1 && walk != NULL) {
                    if (typecmp(type1, walk->type) != 0) break;
                    args = args->childs[2];
                    exp1 = args->childs[0];
                    type1 = Exp(exp1);
                    walk = walk->tail;
                }
                if (walk != NULL && walk->tail == NULL && typecmp(type1, walk->type) == 0) {
                    retType = funcItem->type;
                } else {
                    // error type 9
                    err("Params mismatch", 9, exp->lineNo);
                }
            }
        } else {
            // Exp -> Exp1[Exp2]
            // check if 1. Exp1 is a array, 2. Exp2 is a INT
            node_t* exp1 = exp->childs[0];
            node_t* exp2 = exp->childs[2];
            Type type1 = Exp(exp1);
            Type type2 = Exp(exp2);
            if (type1->kind != ARRAY) {
                // error type 10
                err("Invaild operator '[]'", 10, exp->lineNo);
            } else if (type2->kind != BASIC || type2->u.basic != INT) {
                // error type 12
                err("Index must have a INT type", 12, exp->lineNo);
            } else {
                exp->isLHS = 1;
                retType = type1->u.array.elem;
            }

        }
    }
    
    exp->semanticType = retType;
    return retType;
}

void err(char *msg, int errType, int lineno) {
    if (errTable[lineno] == 1) return;
    errTable[lineno] = 1;
    printf("Error type %d at Line %d: %s\n", errType, lineno, msg);
}

const char* prefix = "__opt_";
const char* suffix = "_opt__";
void randomNameGenerator(char* buf) {
    sprintf(buf, "%s%d%s", prefix, seed, suffix);
    seed += 1; 
}

// return 1 if exp is a lhs exp, otherwise 0
int isLHS(node_t* exp) {
    return exp->isLHS;
}

void dec_undef_check(SymTable func_table) {
    assert(func_table->kind == FUNC);

    for (int idx = 0; idx < TABLESIZE; idx++) {
        Item item = func_table->items[idx];
        while (item != NULL) {
            if (item->funcStatus == DEC) {
                // error type 18
                err("Undefined function", 18, item->lineNo);
            }
            item = item->tail;
        }
    }
}

void paramsHandler(Item funcItem) {
    if (funcItem == NULL) return;

    FieldList paramsWalk = funcItem->func.params;
    while (paramsWalk != NULL) {
        Item item = createItem();
        item->kind = VAR;
        item->type = paramsWalk->type;
        item->lineNo = paramsWalk->lineNo;
        item->func.funcItem = funcItem;
        strcpy(item->name, paramsWalk->name);

        int ret = insertItem(varTable, item);
        if (ret != 1) {
            // error type 1
            err("Variable redefine", 3, item->lineNo);
        }
        paramsWalk = paramsWalk->tail;
    }
}

int getTypeSize(Type type) {
    if (type == NULL) return 4; // temp
    if (type->kind == BASIC) return 4;
    if (type->kind == ARRAY)
        return (type->u.array.size * getTypeSize(type->u.array.elem));
    // structure
    FieldList field = type->u.structure;
    int size = 0;
    while (field != NULL) {
        size += getTypeSize(field->type);
        field = field->tail;
    }
    return size;
}