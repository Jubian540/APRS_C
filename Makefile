all: AX25_decodec AX25_encodec

AX25.o: AX25.c AX25.h
	gcc -c AX25.c

AX25_decodec.o: AX25_decodec.c AX25.h AX25.c
	gcc -c AX25_decodec.c

AX25_decodec: AX25.o AX25_decodec.o
	gcc AX25.o AX25_decodec.o -o AX25_decodec

AX25_encodec.o: AX25_encodec.c AX25.h AX25.c
	gcc -c AX25_encodec.c

AX25_encodec: AX25.o AX25_encodec.o
	gcc AX25.o AX25_encodec.o -o AX25_encodec

clean:
	rm -rf AX25 AX25_decodec AX25_encodec
	rm -rf *.o