#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AX25.h"

int main(int argc, char **argv)
{
    char info[256];
    char buffer[512], file_buffer[512];
    int start = -1;
    int end = -1;

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
        memcpy(buffer, file_buffer + start, end - start);
        AX25_message res = decodec(buffer, end - start, info);
        if (res.checked) {
            printf("callsign ");
            for (int i = 0; i < 7; i++) printf("%c", res.source[i]);
            printf(":%s\n", info);
        }
    } else {
        fprintf(stderr, "format error!\n");
        return -1;
    }
    return 0;
}