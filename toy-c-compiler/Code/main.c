#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"

int main(int argc, char** argv) {
    FILE* irFile; // 输出的中间代码文件
    FILE* asmFile; // 输出的MIPS代码文件

    if (argc <= 1) return 1;
    FILE* cmmFile = fopen(argv[1], "r");
    if (!cmmFile) { perror(argv[1]); return 1; } 

    //语法分析
    yyrestart(cmmFile);
    yyparse(); 

    // 语义分析
    if (!syntaxErrorFlag)
        semantic(syntaxTreeRoot); 

    // 中间代码生成
    CodeList codelisthead = interCode(syntaxTreeRoot);  
    irFile = fopen("output.ir", "w");
    print_IR(codelisthead, irFile); // 写入生成的中间代码 
    fclose(irFile); // 关闭文件
    remove("output.ir"); // 删除文件

    // MIPS代码生成
    InstrList instrListHead = translateIR2MIPS(codelisthead);
    if(argv[2] == NULL) { asmFile = fopen("output.s", "w"); }
    else
        asmFile =fopen(argv[2], "w"); // 构造输出中间代码文件
    print_Instr(instrListHead, asmFile);
    fclose(asmFile);

    return 0;
}