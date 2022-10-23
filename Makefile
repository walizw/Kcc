CC=gcc
ECHO=echo

OBJS=build/compiler.o build/cprocess.o build/helpers/buffer.o \
	build/helpers/vector.o
INCLUDES=-I./

all: $(OBJS)
	$(CC) main.c $(OBJS) $(INCLUDES) -g -o main

build/compiler.o: compiler.c
	@$(ECHO) "CC\t\t"$<
	@$(CC) $(INCLUDES) $< -o $@ -g -c

build/cprocess.o: cprocess.c
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
