
CC=gcc -Wall -Werror -I. -Wextra -Wno-unused -g
AS=gcc -Wall -Werror -I. -Wextra -g

LIBS = threadlib.o x86.o llist.o test.o

test: $(LIBS)
	$(CC) $(LIBS) -o test

%.o: %.c
	$(CC) $< -c -o $@

x86.o: x86.S
	$(AS) $< -c -o -static $@

clean:
	rm *.o

