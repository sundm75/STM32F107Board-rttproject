/*******************************************
/*********************2011-2012, NJUT, Edu. ********************
  FileName: mbtest.c 
  Author:  孙冬梅       Version :  1.0        Date: 2012.12.9
  Description:     freemodbus四个接口回调函数    
  Version:         1.0 
  Function List:    
    1. eMBRegInputCB eMBRegHoldingCB 
      eMBRegCoilsCB	eMBRegDiscreteCB 四个接口函数完成数据的读写操作
  History:         
      <author>  <time>   <version >   <desc> 
      Sundm    12/12/9     1.0     文件创建   
***********************************************************/ 
#include "stm32f10x.h"
#include "mb.h"

/*进入超临界 关总中断*/
void ENTER_CRITICAL_SECTION(void)
{
	__set_PRIMASK(1);
}
/*退出超临界 开总中断*/
void EXIT_CRITICAL_SECTION(void)
{
	__set_PRIMASK(0);
}

/*测试用 输入寄存器数组及保持寄存器数组 地址0-9*/
u16 usRegInputBuf[10]={0x0000,0xfe02,0xfe03,0xfe04,0xfe05,0xfe06,0xfe07,0xfe08, 0xfe09,0xfe0a};
u16 usRegHoldingBuf[10]={0x0000,0xce20,0xce30,0xce40,0xce50,0xce60,0x7e20,0xce80,0xce90,0xcea0};

u8 REG_INPUT_START=0,REG_HOLDING_START=0;
u8 REG_INPUT_NREGS=8,REG_HOLDING_NREGS=8;
u8 usRegInputStart=0,usRegHoldingStart=0;

/*读输入寄存器命令 功能码0x04*/
eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
	eMBErrorCode    eStatus = MB_ENOERR;
	int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START )&& ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;

}

/*寄存器的读写函数命令 支持的命令为读 0x03 和写0x06*/
eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
	u16 *PRT=(u16*)pucRegBuffer;

    if( ( usAddress >= REG_HOLDING_START ) && ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
                *PRT++ = __REV16(usRegHoldingBuf[iRegIndex++]); /*数据序转 REV16位*/

// 		*pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] >> 8 );
//              *pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] & 0xFF );
//		iRegIndex++;
                usNRegs--;
            }
            break;

        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                usRegHoldingBuf[iRegIndex++] = __REV16(*PRT++); /*数据序转 REV16位*/

//		usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
//              usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
//              iRegIndex++;
                usNRegs--;
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/*读/写开关寄存器命令  0x01  x05*/
eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
    ( void )pucRegBuffer;
    ( void )usAddress;
    ( void )usNCoils;
    ( void )eMode;
    return MB_ENOREG;
}

/*读开关寄存器命令 0x02*/
eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    ( void )pucRegBuffer;
    ( void )usAddress;
    ( void )usNDiscrete;
    return MB_ENOREG;
}
