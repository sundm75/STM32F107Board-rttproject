#include "i2c_ee_dma.h"
#include "finsh.h"

void test_i2cdma(unsigned char addr)
{
	uint8_t EEP_Rx_Buffer[20]={0};
        uint8_t EEP_Tx_Buffer[] = "0123456789abcdef";
       
        rt_kprintf("\n\r I2C Test....");
        rt_kprintf("\n\r 写入数据：[%s]", EEP_Tx_Buffer);

	I2C_EE_Init(); 
        I2C_EE_WriteBuffer(EEP_Tx_Buffer,addr,sizeof(EEP_Tx_Buffer)-1 );
        I2C_EE_WaitOperationIsCompleted();	
        I2C_EE_ReadBuffer(EEP_Rx_Buffer,addr, sizeof(EEP_Tx_Buffer)-1);
        I2C_EE_WaitOperationIsCompleted();
        
        rt_kprintf("\n\r 读出数据：[%s]", EEP_Rx_Buffer);
        rt_kprintf("\n\r I2C Test  Complete! \r\n");
        rt_kprintf("\r\n");
		
}

FINSH_FUNCTION_EXPORT (test_i2cdma,   I2C_DMA_Test 0-240 e.g.:test_i2cdma (240));
   
