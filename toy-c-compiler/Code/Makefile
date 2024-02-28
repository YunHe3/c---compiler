CC=gcc
CFLAGS=-g
OBJ = syntax-tree.o main.o syntax.tab.o semantic.o ir.o assemble.o
DEPS = 

lex.yy.c: lexical.l
	flex -o lex.yy.c lexical.l

syntax.tab.c: syntax.y lex.yy.c
	bison -o syntax.tab.c -d -v syntax.y

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

parser: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lfl -ly

submit:
	make clean
	zip ../胡纯涛_211220071.zip -r ../Code ../README ../report.pdf

clean:
	rm -f lex.yy.c syntax.tab.c syntax.tab.h parser syntax.output
	rm -f *.o
	rm -f *.out
	rm -f *.s
