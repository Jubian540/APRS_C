#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AX25.h"
#include "mic_e.h"

unsigned char decodec_message(char *message, char token)
{
    unsigned char *to;
    unsigned char *i;
    char *chpointer, *enchpointer, *timepointer;
    char icon;
    char ob1[512], ob2[512];
    char dimension_buff[10] = {0};
    char longitude_buff[10] = {0};
    int ol1, ol2;
    int index;

    switch(token) {
    case '`':
        //mic_e
        to = strchr(message, '>');
        i = strchr(message, ':');
        ++to;
        ++i;
        if (fmt_mic_e(to,i,strlen(i),ob1,&ol1,ob2,&ol2)) {
            if (ol1) {
                //posit
                ob1[ol1] = '\0';
                decodec_message(ob1, '!');
            }
        } else return -1; //不符合格式
        break;
    case '=':
    case '!':
        //不带时间戳
        if (message != strstr(message, "!")) message = strstr(message, "!");
        chpointer = strstr(message, "/");
        if (!chpointer) return -1; //不符合格式
        memcpy(dimension_buff, message + 1, chpointer - message - 1);
        enchpointer = strstr(chpointer, "W");
        if ((strstr(chpointer, "E") != 0 && strstr(chpointer, "E") < enchpointer) || enchpointer == 0) enchpointer = strstr(chpointer, "E");
        if (!enchpointer) return -1; //不符合格式
        memcpy(longitude_buff, chpointer + 1, enchpointer - chpointer);
        icon = enchpointer[1];
        printf("d: %s\nl: %s\ni: %c\n", dimension_buff, longitude_buff, icon);
        break;
    case '@':
        //带时间戳
        if (message != strstr(message, "@")) message = strstr(message, "@");
        chpointer = strstr(message, "/");
        if (!chpointer) return -1; //不符合格式
        timepointer = strstr(message, "z");
        memcpy(dimension_buff, timepointer + 1, chpointer - timepointer - 1);
        printf("d: %s\n", dimension_buff);
        enchpointer = strstr(chpointer, "W");
        if ((strstr(chpointer, "E") != 0 && strstr(chpointer, "E") < enchpointer) || enchpointer == 0) enchpointer = strstr(chpointer, "E");
        if (!enchpointer) return -1; //不符合格式
        printf("str: %s\n", enchpointer);
        memcpy(longitude_buff, chpointer + 1, enchpointer - chpointer);
        icon = enchpointer[1];
        printf("d: %s\nl: %s\ni: %c\n", dimension_buff, longitude_buff, icon);
        break;
    }

    return 0;
}

int main(int argc, char **argv)
{
    char info[256];
    char buffer[512], file_buffer[512];
    char callsign[8] = {0};
    int start = -1;
    int end = -1;
    char APRS_buffer[512] = {0};

    FILE* fp = fopen("ax25.out", "r");
    if (fp == NULL) {
        fprintf(stderr, "open ax25.out failed!\n");
        return -1;
    }

    int ret = fread(file_buffer, 1, 512, fp);
    for (int i = 0; i < ret; i++) {
        if (file_buffer[i] == FLAGE) {
            if (start == -1) start = i;
            else {
                end = i;
                break;
            }
        }
    }

    if (end > start) {
        printf("decodecing %d byte(s)....\n", end - start);
        memcpy(buffer, file_buffer + start, end - start);
        AX25_message res = decodec(buffer, end - start, info);
        if (res.checked) {

            for (int i = 0; i < 7; i++) {
                if (res.source[i] == ' ') res.source[i] = '-';
                if (res.destination[i] == ' ') res.destination[i] = '\0';
            }

            for (int i = 0; i < 56; i++) {
                if (res.digipeaters[i] == ' ') res.digipeaters[i] = '-';
                if (res.digipeaters[i] == '\0') break;
            }

            for (int i = 0; i < 7; i++) callsign[i] = res.source[i];
            sprintf(APRS_buffer, "%s>%s,%s:%s\n", callsign, res.destination, res.digipeaters, info);
            //strcpy(APRS_buffer, "FW4272>APRS,TCPXX*,qAX,CWOP-4:@040250z4613.89N/06118.53W_200/004g012t046r000p000P000h72b10225L....DsIP");
            //strcpy(info, "@040250z4613.89N/06118.53W_200/004g012t046r000p000P000h72b10225L....DsIP");
            //strcpy(APRS_buffer, "JA1BPP-14>SUQYQ8,JA2YQQ-3,WIDE1*,qAR,JG1FVF:`C4Um xk/`\"4V}_\%");
            //strcpy(info, "`C4Um xk/`\"4V}_\%");
            decodec_message(APRS_buffer, info[0]);
        } else {
            printf("check error!\n");
        }
    } else {
        fprintf(stderr, "format error!\n");
        return -1;
    }
    return 0;
}