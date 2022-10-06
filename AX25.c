#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AX25.h"

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

/*
 * Function name	:header
 * Description		:Encode header of AX25 format 
 * Parameter
 * @addr			AX25 address
 */
AX25_header header(AX25_address addr)
{
	AX25_header res;
	res.data = (char*)malloc(addr.length + 2);
	if (res.data == NULL) {
		fprintf(stderr, "out of memory!\n");
		res.length = 0;
		return res;
	}

	memcpy(res.data, addr.data, addr.length);

	res.length = addr.length + 2;
	res.data[addr.length] = CONTROL_FIELD;
	res.data[addr.length + 1] = PROTOCOL_ID;
	return res;
}

/*
 * Function name	:dump_AX25_frame
 * Description		:dump AX25 frame of APRS 
 * Parameter
 * @hed				AX25 header
 * @ifnot			APRS message
 */
AX25_frame dump_AX25_frame(AX25_header hed, char *info)
{
	AX25_frame res;
	int info_len = strlen(info);
	res.data = malloc(hed.length + 4 + info_len);
	if (res.data == NULL) {
		fprintf(stderr, "out of memory!\n");
		res.length = 0;
		return res;
	}
	res.length = hed.length + 4 + info_len;
	memset(res.data, 0, res.length);
	res.data[0] = FLAGE;
	
	memcpy(res.data + 1, hed.data, hed.length);
	memcpy(res.data + 1 + hed.length, info, info_len);

	int sfcs = fcs(hed, info);
	printf("fcs: %d\n", sfcs);
	res.data[hed.length + info_len + 1] = ~(sfcs & 0xff);
	res.data[hed.length + info_len + 2] = ~((sfcs>>8) & 0xff);
	res.data[hed.length + info_len + 3] = FLAGE;
	return res;
}

int fcs(AX25_header hed, char *info)
{
	int res_fcs = 0xffff;
	int check;

	for (int i = 0; i < hed.length; i++) {
		for(int j = 0; j < 8; j++) {
			short bit;
			if(hed.data[i]&(1<<j)) bit = 1;
			else bit = 0;

			check = (res_fcs & 0x1 == 1);
			res_fcs >>= 1;
			if (check != bit) res_fcs ^= 0x8408;
		}
	}

	for (int i = 0; i < strlen(info); i++) {
		for(int j = 0; j < 8; j++) {
			short bit;
			if(info[i]&(1<<j)) bit = 1;
			else bit = 0;

			check = (res_fcs & 0x1 == 1);
			res_fcs >>= 1;
			if (check != bit) res_fcs ^= 0x8408;
		}
	}

	return res_fcs;
}

AX25_message decodec(char *data, int length, char *info)
{
    AX25_message res;
    char address_buf[70];
    int address_index = -1;
    for (int i = 0; i < length; i++) {
        if (data[i] & 0x1) {
            address_index = i;
            break;
        }
    }

    //APRS Message
    memcpy(info, data + address_index + 3, length - 5 - address_index);
    memcpy(info + length - 5 - address_index, "\0", 1);

    memcpy(address_buf, data + 1, address_index);
    for (int i = 0; i < address_index; i++) {
        address_buf[i] = (address_buf[i]&0xff)>>1;
    }

    memcpy(res.destination, address_buf, 7);
    memcpy(res.source, address_buf + 7, 7);
    memcpy(res.digipeaters, address_buf + 14, address_index - 14);
    memcpy(res.digipeaters + address_index - 14, "\0", 1);

	//decodec header
	AX25_header hed;
	hed.data = malloc(address_index + 2);
	if (hed.data == NULL) {
		fprintf(stderr, "out of memory");
		return res;
	}
	memcpy(hed.data, data + 1, address_index + 2);
	hed.length = address_index + 2;

#if DEBUG
	for (int i = 0; i < hed.length; i++) {
		printf("0x%x ", hed.data[i]&0xff);
	}
	printf("\n");
#endif

	//check fcs
	int sfcs = fcs(hed, info);
	printf(" fcs[0]: 0x%x  fcs[1]: 0x%x\ncfcs[0]: 0x%x cfcs[1]: 0x%x\n", data[length - 2]&0xff, data[length - 1]&0xff, (~(sfcs&0xff)&0xff), (~((sfcs>>8)&0xff)&0xff));
	if (((~(sfcs&0xff)&0xff) == (data[length - 2]&0xff)) && ((~((sfcs>>8)&0xff)&0xff) == (data[length - 1]&0xff))) res.checked = 1;
	else res.checked = 0;

    return res;
}