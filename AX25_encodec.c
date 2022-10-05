#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AX25.h"

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

#if DEBUG
	for (int i = 0; i < addr.length; i++) {
		printf("0x%x ", addr.data[i]);
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
#endif
	return 0;
}