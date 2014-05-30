
 /*    
  *      ____                      _____                  +---+
  *     / ___\                     / __ \                 | R |
  *    / /                        / /_/ /                 +---+
  *   / /   ________  ____  ___  / ____/___  ____  __   __
  *  / /  / ___/ __ `/_  / / _ \/ /   / __ \/ _  \/ /  / /
  * / /__/ /  / /_/ / / /_/  __/ /   / /_/ / / / / /__/ /
  * \___/_/   \__,_/ /___/\___/_/    \___ /_/ /_/____  /
  *                                                 / /
  *                                            ____/ /
  *                                           /_____/
  *                                       
  *  Crazyfile control firmware                                        
  *  Copyright (C) 2011-2014 Crazepony-II                                        
  *
  *  This program is free software: you can redistribute it and/or modify
  *  it under the terms of the GNU General Public License as published by
  *  the Free Software Foundation, in version 3.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  *  GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program. If not, see <http://www.gnu.org/licenses/>.
  *
  *
  * debug.c - Debugging utility functions
  *
  */

#include "NRF24L01.h"
#include "spi.h"
#include "ReceiveData.h"
#include "delay.h"
	 
uint8_t NRF24L01_RXDATA[RX_PLOAD_WIDTH];//nrf24l01���յ�������
uint8_t NRF24L01_TXDATA[RX_PLOAD_WIDTH];//nrf24l01��Ҫ���͵�����



//�޸ĸý��պͷ��͵�ַ�����Թ������������ͬһ������У����ݲ��ܸ���
u8  TX_ADDRESS[TX_ADR_WIDTH]= {0x34,0xc3,0x10,0x10,0x11};	//���ص�ַ
u8  RX_ADDRESS[RX_ADR_WIDTH]= {0x34,0xc3,0x10,0x10,0x11};	//���յ�ַ				



//д�Ĵ���
uint8_t NRF_Write_Reg(uint8_t reg, uint8_t value)
{
    uint8_t status;
    SPI_CSN_L();					  
    status = SPI_RW(reg);  
    SPI_RW(value);		  /* д���� */
    SPI_CSN_H();					  /* ��ֹ������ */
    return 	status;
}


//���Ĵ���
uint8_t NRF_Read_Reg(uint8_t reg)
{
    uint8_t reg_val;
    SPI_CSN_L();					 
    SPI_RW(reg);			  
    reg_val = SPI_RW(0);	  /* ��ȡ�üĴ����������� */
    SPI_CSN_H();	
 
    return 	reg_val;
}


//д������
uint8_t NRF_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t uchars)
{
    uint8_t i;
    uint8_t status;
    SPI_CSN_L();				        /* ѡͨ���� */
    status = SPI_RW(reg);	/* д�Ĵ�����ַ */
    for(i=0; i<uchars; i++)
    {
        SPI_RW(pBuf[i]);		/* д���� */
    }
    SPI_CSN_H();						/* ��ֹ������ */
    return 	status;	
}


//��������
uint8_t NRF_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t uchars)
{
    uint8_t i;
    uint8_t status;
    SPI_CSN_L();						/* ѡͨ���� */
    status = SPI_RW(reg);	/* д�Ĵ�����ַ */
    for(i=0; i<uchars; i++)
    {
        pBuf[i] = SPI_RW(0); /* ��ȡ�������� */ 	
    }
    SPI_CSN_H();						/* ��ֹ������ */
    return 	status;
}


//д���ݰ�
void NRF_TxPacket(uint8_t * tx_buf, uint8_t len)
{	
    SPI_CE_L();		 //StandBy Iģʽ	
    NRF_Write_Buf(WR_TX_PLOAD, tx_buf, len); 			 // װ������	
    SPI_CE_H();		 //�ø�CE���������ݷ���
}

//��ʼ��
char NRF24L01_INIT(void)
{

   SPI1_INIT();
   return NRF24L01_Check();
}


//����ģʽ
void SetRX_Mode(void)
{
    SPI_CE_L();
   
	  NRF_Write_Reg(FLUSH_RX,0xff);//���TX FIFO�Ĵ���			 
  	NRF_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS,RX_ADR_WIDTH);//дRX�ڵ��ַ
   	NRF_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);    //ʹ��ͨ��0���Զ�Ӧ��    
  	NRF_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01);//ʹ��ͨ��0�Ľ��յ�ַ  	 
  	NRF_Write_Reg(NRF_WRITE_REG+RF_CH,40);	     //����RFͨ��Ƶ��		  
  	NRF_Write_Reg(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//ѡ��ͨ��0����Ч���ݿ��� 	    
  	NRF_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);//����TX�������,0db����,2Mbps,���������濪��   
  	NRF_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0f);//���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ 
    
    
    SPI_CE_H();
} 


//����ģʽ
void SetTX_Mode(void)
{
    SPI_CE_L();
   
    NRF_Write_Reg(FLUSH_TX,0xff);										//���TX FIFO�Ĵ���		  
  	
    NRF_Write_Buf(NRF_WRITE_REG+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH);		//дTX�ڵ��ַ 
  	NRF_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH); 	//����TX�ڵ��ַ,��ҪΪ��ʹ��ACK	  

  	NRF_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);     //ʹ��ͨ��0���Զ�Ӧ��    
  	NRF_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01); //ʹ��ͨ��0�Ľ��յ�ַ  
  	NRF_Write_Reg(NRF_WRITE_REG+SETUP_RETR,0x1a);//�����Զ��ط����ʱ��:500us + 86us;����Զ��ط�����:10��
  	NRF_Write_Reg(NRF_WRITE_REG+RF_CH,40);       //����RFͨ��Ϊ40
  	NRF_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);  //����TX�������,0db����,2Mbps,���������濪��   
  	NRF_Write_Reg(NRF_WRITE_REG+CONFIG,0x0e);    //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ,���������ж�
    
    
    SPI_CE_H();
} 


//��ѯ�ж�
void Nrf_Irq(void)
{
    uint8_t sta = NRF_Read_Reg(NRF_READ_REG + NRFRegSTATUS);
    if(sta & (1<<RX_DR))//�����ж�
    {
        NRF_Read_Buf(RD_RX_PLOAD,NRF24L01_RXDATA,RX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer
         
        Get_ReceiveData();      //�Լ����޸�

    }
 
    NRF_Write_Reg(0x27, sta);//���nrf���жϱ�־λ
}


//���պ���
u8 NRF24L01_RxPacket(u8 *rxbuf)
{
	u8 sta;		    							   
        //SPI2_SetSpeed(SPI_SPEED_4); //spi�ٶ�Ϊ9Mhz��24L01�����SPIʱ��Ϊ10Mhz��   
	sta=NRF_Read_Reg(NRFRegSTATUS);  //��ȡ״̬�Ĵ�����ֵ    	 
	NRF_Write_Reg(NRF_WRITE_REG+NRFRegSTATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	if(sta&RX_OK)//���յ�����
	{
		NRF_Write_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//��ȡ����
		NRF_Write_Reg(FLUSH_RX,0xff);//���RX FIFO�Ĵ��� 
		return 0; 
	}	   
	return 1;//û�յ��κ�����
}		



//�ж�SPI�ӿ��Ƿ����
u8 NRF24L01_Check(void) 
{ 
   u8 buf[5]={0xC2,0xC2,0xC2,0xC2,0xC2}; 
   u8 buf1[5]; 
   u8 i=0; 
    
   /*д��5 ���ֽڵĵ�ַ.  */ 
   NRF_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5); 
     
   /*����д��ĵ�ַ */ 
   NRF_Read_Buf(TX_ADDR,buf1,5); 
   
    /*�Ƚ�*/ 
   for (i=0;i<5;i++) 
   { 
      if (buf1[i]!=0xC2) 
      break; 
   } 
  
   if (i==5)   return 1 ;        //MCU ��NRF �ɹ����� 
   else        return 0 ;        //MCU��NRF����������    
} 
