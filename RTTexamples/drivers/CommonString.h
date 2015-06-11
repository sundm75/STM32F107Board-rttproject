#ifndef COMMON_H
#define COMMON_H

#include <rtthread.h>
#include "ctype.h"
#include "string.h"
#if defined(RT_USING_DFS)
#include <dfs_posix.h>
#endif

//将16进制字符串转换成16进制数如："12" 转换成0x12
void  ASCIItoHEX(unsigned char* arrayin, unsigned char *arrayout);

//将16进制数转换成16进制字符串如：12 转换成"12"
void  HEXtoASCII(unsigned char* arrayin, unsigned char *arrayout);
void  HEXtonASCII(unsigned char* arrayin, unsigned char *arrayout, unsigned char len);
//将一个字符串s2接到另一个字符串s1后面，包括0x00
void insertstr(char *s1,char *s2,int l1,int l2);

//检测字符串是否为字母或者数字
char isstring(char* p);

//将字符串中某个字符删除
void strcut(char*p,char c);

//从字符串删除指定长度字符
void DeleteString(unsigned char* array, unsigned int arraylen, unsigned int beginaddr, unsigned int dellen);

//将一个字符串插入到另一个字符串的制定位置
void insert(char *s1,char *s2,int f);

//去除字符串中非字母或者数字的字符
void strcutnochar(char*p);

//清零缓冲
void clrbuf(char*buf,int num);
//将回车换行变成空格

void strrep(char*p);

//按字符打印
void printchar(char *buffer,int len);

//字符串比较，按长度
char  compstr(char *buffer1,char *buffer2,int len);



#endif
