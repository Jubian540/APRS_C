#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTROL_FIELD	0x03
#define PROTOCOL_ID	0xf0
#define	FLAGE		0x73

#define	DEBUG		0x1

typedef struct AX25_Message {
	char destination[10];	/*APRS destination*/
	char source[10];		/*callsign and ssid*/
	char digipeaters[255];	/*APRS digipeaters*/
	char info[255];		/*APRS message*/
}AX25_message;

typedef struct AX25_ADDR
{
	char *data;
	int length;
}AX25_address;


/*
 * Function name	:callsign_encode
 * Description		:Encode callsign to AX25 format
 * Parameter
 * @callsign		strig of callsign
 * @ssid			string of ssid
 */
char *callsign_encode(char *callsign_str)
{
	int index = 0;
	char callsign[10] = {0};
	char ssid[10] = {0};
	int callsign_len = 0;
	char *format = malloc(sizeof(char)*7);
	if (format == NULL) {
		fprintf(stderr, "out of memory!\n");
		return NULL;
	}

	for (int i = 0;i < strlen(callsign_str); i++) {
		if (callsign_str[i] == '-') {
			index = i;
			break;
		}
	}

	if (index == 0) {
		strcpy(callsign, callsign_str);
		strcpy(ssid, "0");
	} else {
		memcpy(callsign, callsign_str, index);
		memcpy(ssid, callsign_str + index + 1, 1);
	}

	callsign_len = strlen(callsign);

	if (callsign_len > 6) {
		memcpy(format, callsign, 6);
		memcpy(format + 6, " ", 1);
	} else {
		memcpy(format, callsign, callsign_len);
		for (int i = callsign_len; i < 6; i++) {
			memcpy(format + i, " ", 1);
		}
	}
	memcpy(format + 6, ssid, 1);

#if DEBUG
	for (int i = 0; i <= 7; i++) {
		printf("%c", format[i]);
	}
	printf("\n");
#endif

#if DEBUG
	printf("format: ");
#endif
	for (int i = 0; i <= 7; i++) {
		format[i] = format[i] << 1;
#if DEBUG
		printf("%c", format[i]);
#endif
	}

#if DEBUG
		printf("\n");
#endif
	return format;
}

/*
 * Function name	:encoded_addresses
 * Description		:Encode address of AX25 format 
 * Parameter
 * @msg				AX25 message
 */
AX25_address encoded_addresses(AX25_message msg)
{
	int count = 1;
	int start = 0;
	int next = 0;
	AX25_address res;
	char buf[15];

	for (int i = 0; i < strlen(msg.digipeaters); i++) {
		if (msg.digipeaters[i] == ',') count++;
	}

	res.length = 14 + count*7;
	res.data = (char*)malloc(res.length);
	if (res.data == NULL) {
		fprintf(stderr, "out of memory!\n");
		res.length = 0;
		return res;
	}

	memcpy(res.data, callsign_encode(msg.destination), 7);
	memcpy(res.data + 7, callsign_encode(msg.source), 7);

	for (int i = 0; i < count; i++) {
		for (int j = start; j < strlen(msg.digipeaters); j++) {
			if (msg.digipeaters[j] == ',') {
				next = j - start;
				break;
			}
		}

		memcpy(buf, msg.digipeaters + start, next);
		buf[next] = '\0';
		start += next + 1;
		memcpy(res.data + 14 + i*7, callsign_encode(buf), 7);
	}

	res.data[res.length-1] |= 0x01;
	return res;
}

int main(int argc, char **argv){
	AX25_message msg;
	strcpy(msg.destination, "APRS");
	strcpy(msg.source, "AB252-2");
	strcpy(msg.digipeaters, "WIDE1-1,WIDE2-1,WIDE3-1");
	AX25_address addr = encoded_addresses(msg);

	for (int i = 0; i < addr.length; i++) {
		printf("%c", addr.data[i]);
	}
	printf("\n");
	return 0;
}
