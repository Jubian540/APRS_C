AX25: AX25.c AX25.h
	gcc AX25.c -o AX25

all: AX25

clean:
	rm AX25
	rm -rf *.o