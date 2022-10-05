#ifndef AX25_H
#define AX25_H

#define CONTROL_FIELD	0x03
#define PROTOCOL_ID	0xf0
#define	FLAGE		0x7e

#define	DEBUG		0x0

typedef struct AX25_Message {
	char destination[7];	/*APRS destination*/
	char source[7];		/*callsign and ssid*/
	char digipeaters[56];	/*APRS digipeaters*/
	int checked;
}AX25_message;

typedef struct AX25_ADDR
{
	char *data;
	int length;
}AX25_address;

typedef struct Header
{
	char *data;
	int length;
}AX25_header;

typedef struct AX25_Frame
{
	char *data;
	int length;
}AX25_frame;

int fcs(AX25_header hed, char *info);

char *callsign_encode(char *callsign_str);
AX25_address encoded_addresses(AX25_message msg);
AX25_header header(AX25_address addr);
AX25_frame dump_AX25_frame(AX25_header hed, char *info);
AX25_message decodec(char *data, int length, char *info);
#endif