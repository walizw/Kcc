PROGRAM_NAME=kcc

CC=gcc
ECHO=echo

OBJS=build/compiler.o build/cprocess.o build/lex_process.o build/lexer.o \
	build/token.o build/helpers/buffer.o build/helpers/vector.o
INCLUDES=-I./

all: $(OBJS)
	$(CC) main.c $(OBJS) $(INCLUDES) -g -o $(PROGRAM_NAME)

build/compiler.o: compiler.c
	@$(ECHO) "CC\t\t"$<
	@$(CC) $(INCLUDES) $< -o $@ -g -c

build/cprocess.o: cprocess.c
	@$(ECHO) "CC\t\t"$<
	@$(CC) $(INCLUDES) $< -o $@ -g -c

build/lex_process.o: lex_process.c
	@$(ECHO) "CC\t\t"$<
	@$(CC) $(INCLUDES) $< -o $@ -g -c

build/lexer.o: lexer.c
	@$(ECHO) "CC\t\t"$<
	@$(CC) $(INCLUDES) $< -o $@ -g -c

build/token.o: token.c
	@$(ECHO) "CC\t\t"$<
	@$(CC) $(INCLUDES) $< -o $@ -g -c

build/helpers/buffer.o: helpers/buffer.c
	@$(ECHO) "CC\t\t"$<
	@$(CC) $(INCLUDES) $< -o $@ -g -c

build/helpers/vector.o: helpers/vector.c
	@$(ECHO) "CC\t\t"$<
	@$(CC) $(INCLUDES) $< -o $@ -g -c

clean:
	rm -rf main $(OBJS)
