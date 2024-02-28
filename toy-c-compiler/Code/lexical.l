/* structrue of the flex source code: */
/* 1. definitions */
/* 2. rules */
/* 3. user subroutines */

/* includes and macros */
%{
    #include <stdio.h>
    #include "syntax.tab.h"
    int lexicalErrorFlag = 0;
    extern int errorMarks[];
    int yycolumn = 1;
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;

%}

/*----------------------definitions----------------------*/

digits [0-9]
letters [a-zA-Z]
%option yylineno

/*-------------------------rules-------------------------*/

%%

if {
    return IF;
}

else {
    return ELSE;
}

while {
    return WHILE;
}

struct {
    return STRUCT;
}

return {
    return RETURN;
}

int {
    return TYPE_INT;
}

float {
    return TYPE_FLOAT;
}

([1-9][0-9]{0,9}|0) { 
    yylval.type_string = strdup(yytext);
    // 32bits decimal value -2147483648~2147483648
    return INTD; 
}



([0-9]*\.[0-9]+|[0-9]+\.) {
    yylval.type_string = strdup(yytext);
    return FLOAT;
}

([0-9]*\.[0-9]+|[0-9]+\.)[Ee][-+]?[0-9]+ {
    yylval.type_string = strdup(yytext);
    return FLOAT;
}

([0-9]+\.[0-9]+)e-? {
    // error type of floate
    if (errorMarks[yylineno] == 0) {
        printf("Error type A at Line %d: Illegal floating point number \"%s\"\n", yylineno, yytext);
        lexicalErrorFlag = 1;
        errorMarks[yylineno] = 1;
    }
    
}

[_a-zA-Z][_a-zA-Z0-9]{0,31} {
    // id length no more than 32 
    int checkUnderScore = 0;
    for (int i = 0; i < yyleng; i++) {
        if (yytext[i] == '_') {
            ++checkUnderScore;
        }
    }
    if (checkUnderScore == yyleng) {
        REJECT;
    }

    yylval.type_string = strdup(yytext);
    return ID; 
}

\"[^\"]*\" {
    if (yytext[yyleng - 2] == '\\') {
        yyless(yyleng - 1);
        yymore();
    } else {
        return STRING;
    }
}


";" { return SEMI; }

"," { return COMMA; }

"=" { return ASSIGNOP; }

">" { return BT; }

"<" { return LT; }

>= { return BE; }

"<=" { return LE; }

"==" { return EQ; }

"!=" { return NE; }

"+" { return PLUS; }

"-" { return MINUS; }

"*" { return STAR; }

"/" { return DIV; }

"&&" { return AND; }

\|\| { return OR; }

\. { return DOT; }

"!" { return NOT; }

"(" { return LP; }

")" { return RP; }

"[" { return LB; }

"]" { return RB; }

"{" { return LC; }

"}" { return RC; }

[ \t\r]+ {  }

\n { yycolumn = 1; }

. {
    if (errorMarks[yylineno] == 0) {
        printf("Error type A at Line %d: Mysterious characters \'%s\'\n",
            yylineno, yytext);
        lexicalErrorFlag = 1;
        errorMarks[yylineno] = 1;
    }
}

%%

/*--------------------user subroutines--------------------*/
// "//" {
//     // eat the comments
//     char c = input();
//     while (c != '\n') { c = input(); }
// }

// "/*" {
//     // eat the comments
//     char prev = input();
//     char c = input();
//     while (!(prev == '*' && c == '/')) {
//         prev = c;
//         c = input();
//     }
// }

// 0[0-7]{1,11} {
//     yylval.type_string = strdup(yytext);
//     // 32-bits octal value start with '0', -020000000000~020000000000
//     return INTO;
// }

// 0[xX][a-fA-F0-9]{1,8} {
//     yylval.type_string = strdup(yytext);
//     // 32 bits hexadecimal value start with '0x' or '0X', -0xffffffff~0xffffffff
//     return INTH;
// }