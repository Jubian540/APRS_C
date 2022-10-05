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

/*
* APRS协议封装，先要封装成AX25格式，校验包含在AX25包里面
* AX25封包 FLAGE + destination + source + digipeaters + CONTROL_FIELD + PROTOCOL_ID + APRS消息 + 校验 + FLAGE
*/
int main(int argc, char **argv){
	FILE *fp;
	AX25_message msg;
	strcpy(msg.destination, "APRS");			/*这里是七个字符的描述，显示在地图上的名称*/
	strcpy(msg.source, "BA6QH-2");				/*呼号-ssid ssid决定了站点类型，站台，汽车，船舶，气象站之类的，详细对应的数字参考手册*/
	strcpy(msg.digipeaters, "WIDE1-1,WIDE2-1");	/*这里是填8位呼号用的，上一个标准呼号有填的话会被覆盖，详细看手册*/
	AX25_address addr = encoded_addresses(msg);	
	AX25_header hea = header(addr);
	AX25_frame frame = dump_AX25_frame(hea, "!2453.48N/10251.05E"); /*第二个参数包含经纬度信息，还可以包含更多的信息，气压，风向之类的，手册里有说明*/

	fp = fopen("ax25.out", "w");
	if (fp == NULL) {
		fprintf(stderr, "open ax25.out failed!\n");
		return -1;
	}

	fwrite(frame.data, 1, frame.length, fp);
	fclose(fp);

	for (int i = 0; i < frame.length; i++) {
		printf("%c", frame.data[i]);
	}
	printf("\n");

	for (int i = 0; i < frame.length; i++) {
		for(int j = 0; j < 8; j++) {
			char c;
			if(frame.data[i]&(1<<j)) c = '1';
			else c = '0';
			printf("%c", c);
		}
	}
	printf("\n");
	return 0;
}
