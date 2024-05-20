#include <stdint.h>
#include <stddef.h>
#include "header/stdlib/string.h"

void* memset(void *s, int c, size_t n) {
    uint8_t *buf = (uint8_t*) s;
    for (size_t i = 0; i < n; i++)
        buf[i] = (uint8_t) c;
    return s;
}

void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
    uint8_t *dstbuf       = (uint8_t*) dest;
    const uint8_t *srcbuf = (const uint8_t*) src;
    for (size_t i = 0; i < n; i++)
        dstbuf[i] = srcbuf[i];
    return dstbuf;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *buf1 = (const uint8_t*) s1;
    const uint8_t *buf2 = (const uint8_t*) s2;
    for (size_t i = 0; i < n; i++) {
        if (buf1[i] < buf2[i])
            return -1;
        else if (buf1[i] > buf2[i])
            return 1;
    }

    return 0;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *dstbuf       = (uint8_t*) dest;
    const uint8_t *srcbuf = (const uint8_t*) src;
    if (dstbuf < srcbuf) {
        for (size_t i = 0; i < n; i++)
            dstbuf[i]   = srcbuf[i];
    } else {
        for (size_t i = n; i != 0; i--)
            dstbuf[i-1] = srcbuf[i-1];
    }

    return dest;
}

uint8_t strlen(char* str){
    int i = 0;
    while(str[i] != '\0'){
        i++;
    }
    return i;
}

void strcat(char* str1, char* str2){
    int i = 0;
    int j = 0;
    while(str1[i] != '\0'){
        i++;
    }
    while(str2[j] != '\0'){
        str1[i] = str2[j];
        i++;
        j++;
    }
    str1[i] = '\0';
}

int isStrEqual(void* str1, void* str2){
    if (strlen(str1) != strlen(str2)) {return 0;}
    return memcmp(str1, str2, strlen(str1)) == 0;
}

int strToInt(char* str){
    int i = 0;
    int res = 0;
    while (str[i] != 0x0){
        res *= 10;
        res += str[i] - '0';
        i++;
    }
    return res;
}

void intToStr(int num, char* str) {
  int i = 0;

  if (num == 0) {
    str[i++] = '0';
  }

  while (num > 0) {
    int digit = num % 10;
    str[i++] = digit + '0';
    num /= 10;
  }

  str[i] = '\0';

  for (int j = 0; j < i / 2; j++) {
    char temp = str[j];
    str[j] = str[i - 1 - j];
    str[i - 1 - j] = temp;
  }
}

void intToStrPad(int num, char* str, int bufferLength) {
  int i = 0;

  if (num == 0) {
    str[i++] = '0';
  }

  for(int j = 0; j < bufferLength; j++) {
    int digit = num % 10;
    str[i++] = digit + '0';
    num /= 10;
  }

  for (int j = 0; j < i / 2; j++) {
    char temp = str[j];
    str[j] = str[i - 1 - j];
    str[i - 1 - j] = temp;
  }
}