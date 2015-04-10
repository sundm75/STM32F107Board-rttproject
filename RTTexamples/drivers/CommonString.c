/*********************2012-2013, NJUT, Edu.********************* 
FileName: CommonString.c 
Author:  孙冬梅       Version :  1.0        Date: 2013.06.10
Description:    通用字符串函数     
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    13/04/10     1.0     文件创建   
***************************************************************/ 
#include "CommonString.h"


/*******************************************************************************
* Function Name  : strcut
* Description    : 删除字符串中的某个字符
* Input          : p 输入字符串 c 欲删除的字符
* Output         : p 
* Return         : None
*******************************************************************************/
void strcut(char*p,char c)
{
    	while(*p!=0)    //没有结束，循环
	{
		if(*p==c)  //遇到字符c处理
		{
			char *q=p;  //从字符c处开始
			while(*q!=0) //直到末尾的所有字符
			{
				*q=*(q+1); //逐次前移
				q++;      //每移一个字符，指针加1，准备移下一个字符
			}
		}  
		else        //当前字符不是空格
		{
			p++;  //指针后移，指向待检查的新字符
		}
	}
}

/*******************************************************************************
* Function Name  : DeleteString
* Description    : 从字符串删除指定长度字符
* Input          : beginaddr 开始删除的位置，dellen 删除字符串长度。
* Output         : p 
* Return         : None
*******************************************************************************/
void DeleteString(unsigned char* array, unsigned int arraylen, unsigned int beginaddr, unsigned int dellen)
{
  int length;
	int i;
  length = arraylen - beginaddr - dellen;
  array = array + beginaddr;
  if( beginaddr < arraylen && length >=0 )
  {
    for( i = 0; i < length; i++)
    {
      *array = *(array + dellen ); 
      array++;
    }
    for( i = length; i < arraylen; i++)
    {
      *array = 0x00; 
      array++;
    }
  }
}

/*******************************************************************************
* Function Name  : insert
* Description    : 将一个字符串插入到另一个字符串的制定位置
* Input          : s1目的字符串 s2源字符串 f待插入位置 
* Output         : None 
* Return         : None
*******************************************************************************/
void insert(char *s1,char *s2,int f)
{
 int i,l1=0,l2=0;
 char *p;
 for(p=s1;*p!=0;p++)
  l1++;
 for(p=s2;*p!=0;p++)
  l2++;
 for(i=l1+l2;i>=f+l2;i--)
 {
  *(s1+i)=*(s1+l1);
  *(s1+l1)=*(s2+l2-1);
  l1--;
  l2--;
 }
}
//将一个字符串s2接到另一个字符串s1后面，包括0x00
void insertstr(char *s1,char *s2,int l1,int l2)
{
 int i;
 for (i=0;i<l2;i++)
 {
   *(s1+l1+i)=*(s2+i);
 }
}

void clrbuf(char*buf,int num)
{
  memset(buf,0,num);
}
//将回车和换行都换成空格
void strrep(char*p)
{
    	while(*p!=0)    //没有结束，循环
	{
		if((*p==0x0a)||(*p==0x0d))  //遇到空格处理
		{
                  *p = 0x20;
		}
		p++;  //指针后移，指向待检查的新字符
	}
}


//检测字符串是否为字母或者数字
char isstring(char*p)
{
    	while(*p!=0)    //没有结束，循环
	{
		if( ! isalnum (*p) )  //如果字符为非字母或者数字，返回0
		{
                  return RT_FALSE;
		}
		p++;  //指针后移，指向待检查的新字符
	}
        return RT_TRUE;
}
//去除字符串中非字母或者数字的字符
void strcutnochar(char* p)
{
    	while(*p!=0)    //没有结束，循环
	{
		if( ! isalnum (*p) )  //如果字符为非字母或者数字，返回0
		{
                  strcut( p,*p);
                  p--;
		}
		p++;  //指针后移，指向待检查的新字符
	}
}
//将16进制字符串转换成16进制数如："12" 转换成0x12 不足双字符的补0
void ASCIItoHEX(unsigned char* arrayin, unsigned char *arrayout)
{
    unsigned char  temp = 0,n = 0;
    while(*arrayin!=0)    //没有结束，循环
    {
      if(*arrayin >= 'A'&& *arrayin <= 'F') //十六进制还要判断他是不是在A-F或者a-f之间a=10。。
         n = *arrayin - 'A' + 10;
       else if(*arrayin >= 'a'&& *arrayin <= 'f')
         n = *arrayin - 'a' + 10;
       else n = *arrayin - '0';
        arrayin++;
      if(*arrayin!=0)  
      {
        if(*arrayin >= 'A'&& *arrayin <= 'F') //十六进制还要判断他是不是在A-F或者a-f之间a=10。。
           temp = *arrayin - 'A' + 10;
         else if(*arrayin >= 'a'&& *arrayin <= 'f')
           temp = *arrayin - 'a' + 10;
         else temp = *arrayin - '0';
      }
         
        temp = temp + n*16;
        *arrayout = temp;
        arrayin++;
        arrayout++;
        temp = 0;n = 0;
    }
}
//将16进制数转换成16进制字符串如：12 转换成"12"
void  HEXtoASCII(unsigned char* arrayin, unsigned char *arrayout)
{
    unsigned char  temph = 0,templ = 0;
    while(*arrayin!=0)    //没有结束，循环
    {
      temph = (*arrayin &0xF0) >>4;
      templ = *arrayin & 0x0F;
      
      if(temph <=0x09)
      *arrayout = temph + 0x30;
      else if(temph >= 0x0A && temph <=0x0F)
      *arrayout = temph -0x0A + 'A';
      else 
      *arrayout = 0x30;
      arrayout++;
      
      if(templ <=0x09)
      *arrayout = templ + 0x30;
      else if(templ >= 0x0A && templ <=0x0F)
      *arrayout = templ -0x0A + 'A';
      else 
      *arrayout = 0x30;
      arrayout++;
      arrayin++;
    }
}
//将16进制数转换成16进制字符串如：0x12 转换成"12" len为输入16进制数据长度
void  HEXtonASCII(unsigned char* arrayin, unsigned char *arrayout, unsigned char len)
{
    unsigned char  temph = 0,templ = 0;
    while(len)    //没有结束，循环
    {
      temph = (*arrayin &0xF0) >>4;
      templ = *arrayin & 0x0F;
      
      if(temph <=0x09)
      *arrayout = temph + 0x30;
      else if(temph >= 0x0A && temph <=0x0F)
      *arrayout = temph -0x0A + 'A';
      else 
      *arrayout = 0x30;
      arrayout++;
      
      if(templ <=0x09)
      *arrayout = templ + 0x30;
      else if(templ >= 0x0A && templ <=0x0F)
      *arrayout = templ -0x0A + 'A';
      else 
      *arrayout = 0x30;
      arrayout++;
      arrayin++;
      len--;
    }
}

//按字符打印
void printchar(char *buffer,int len)
{
  	int i;    
	for( i=0;i<len;i++)
      {
        rt_kprintf("%c",buffer[i]);
      }
}

//字符串比较，按长度
char  compstr(char *buffer1,char *buffer2,int len)
{
	int i;
	if(strlen(buffer1)==len)
      {
          for( i=0;i<len;i++)
          {
            if(*buffer1 != *buffer2)
            {
              return 0;
            }
            buffer1++;buffer2++;
          }
      }     
      else
        return 0;
      return 1;
}



