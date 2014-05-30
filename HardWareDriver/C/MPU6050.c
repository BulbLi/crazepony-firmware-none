
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
#include "MPU6050.h"
#include "IIC.h"
#include "extern_variable.h"


//MPU6050����������ؼ�����IIC�ӿ�Ҫ�����ã������������Ҫ��
//�������òο������ֲ�
//����޸ģ�2014-01-29

uint8_t 	mpu6050_buffer[14];					
S_INT16_XYZ 	GYRO_OFFSET,ACC_OFFSET;			
uint8_t GYRO_OFFSET_OK = 1;
uint8_t ACC_OFFSET_OK = 1;
S_INT16_XYZ 	MPU6050_ACC_LAST,MPU6050_GYRO_LAST;		


//�ڲ���ʱ
static void MPU6050_Delay(unsigned long time)
{
   long i;
   for(i=0; i<time; i++)
   {
     
   }
}


//��iic��ȡ�������ݷֲ�,������Ӧ����
void MPU6050_Dataanl(void)
{
    MPU6050_ACC_LAST.X=((((int16_t)mpu6050_buffer[0]) << 8) | mpu6050_buffer[1]) - ACC_OFFSET.X;
    MPU6050_ACC_LAST.Y=((((int16_t)mpu6050_buffer[2]) << 8) | mpu6050_buffer[3]) - ACC_OFFSET.Y;
    MPU6050_ACC_LAST.Z=((((int16_t)mpu6050_buffer[4]) << 8) | mpu6050_buffer[5]) - ACC_OFFSET.Z;
    //���������¶�ADC����ϸ���ο��ֲ�
    MPU6050_GYRO_LAST.X=((((int16_t)mpu6050_buffer[8]) << 8) | mpu6050_buffer[9]) - GYRO_OFFSET.X;
    MPU6050_GYRO_LAST.Y=((((int16_t)mpu6050_buffer[10]) << 8) | mpu6050_buffer[11]) - GYRO_OFFSET.Y;
    MPU6050_GYRO_LAST.Z=((((int16_t)mpu6050_buffer[12]) << 8) | mpu6050_buffer[13]) - GYRO_OFFSET.Z;

    if(!GYRO_OFFSET_OK)
    {
        static int32_t	tempgx=0,tempgy=0,tempgz=0;
        static uint8_t cnt_g=0;

        if(cnt_g==0)
        {
            GYRO_OFFSET.X=0;
            GYRO_OFFSET.Y=0;
            GYRO_OFFSET.Z=0;
            tempgx = 0;
            tempgy = 0;
            tempgz = 0;
            cnt_g = 1;
            
            return;
        }
        tempgx+= MPU6050_GYRO_LAST.X;
        tempgy+= MPU6050_GYRO_LAST.Y;
        tempgz+= MPU6050_GYRO_LAST.Z;
        if(cnt_g==200)
        {
            GYRO_OFFSET.X=tempgx/cnt_g;
            GYRO_OFFSET.Y=tempgy/cnt_g;
            GYRO_OFFSET.Z=tempgz/cnt_g;
            cnt_g = 0;
            GYRO_OFFSET_OK = 1;
            
            return;
        }
        
        cnt_g++;
    }
    if(!ACC_OFFSET_OK)
    {
        static int32_t	tempax=0,tempay=0,tempaz=0;
        static uint8_t cnt_a=0;

        if(cnt_a==0)
        {
            ACC_OFFSET.X = 0;
            ACC_OFFSET.Y = 0;
            ACC_OFFSET.Z = 0;
            tempax = 0;
            tempay = 0;
            tempaz = 0;
            cnt_a = 1;
            
            return;
        }
        tempax+= MPU6050_ACC_LAST.X;
        tempay+= MPU6050_ACC_LAST.Y;
        tempaz+= MPU6050_ACC_LAST.Z;
        if(cnt_a==200)
        {
            ACC_OFFSET.X=tempax/cnt_a;
            ACC_OFFSET.Y=tempay/cnt_a;
            ACC_OFFSET.Z=tempaz/cnt_a;
            cnt_a = 0;
            ACC_OFFSET_OK = 1;
            
            return;
        }
        cnt_a++;		
    }
}


//�������µ�MPU����
void MPU6050_READ(void)
{
    i2cRead(devAddr,MPU6050_RA_ACCEL_XOUT_H,14,mpu6050_buffer);
}


//�޸�дָ���豸ָ���Ĵ���һ���ֽ��е�1��λ
void IICwriteBit(u8 dev, u8 reg, u8 bitNum, u8 data)
{
    u8 b;
    
    i2cRead(dev, reg, 1, &b);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    i2cWrite(dev, reg, b);
}


//�޸�дָ���豸 ָ���Ĵ���һ���ֽ��еĶ��λ
void IICwriteBits(u8 dev,u8 reg,u8 bitStart,u8 length,u8 data)
{
    u8 mask;
    u8 b;
    
    i2cRead(dev, reg, 1, &b);
    mask = (0xFF << (bitStart + 1)) | 0xFF >> ((8 - bitStart) + length - 1);
    data <<= (8 - length);
    data >>= (7 - bitStart);
    b &= mask;
    b |= data;
    i2cWrite(dev, reg, b);
}


//����MPU6050��ʱ��Դ
/**********************************************************************
* 0       | Internal oscillator
* 1       | PLL with X Gyro reference
* 2       | PLL with Y Gyro reference
* 3       | PLL with Z Gyro reference
* 4       | PLL with external 32.768kHz reference
* 5       | PLL with external 19.2MHz reference
* 6       | Reserved
* 7       | Stops the clock and keeps the timing generator in reset
********************************************************************/
void MPU6050_setClockSource(uint8_t source)
{
    IICwriteBits(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, source);
}


//���� MPU6050 �����ǵ��������
void MPU6050_setFullScaleGyroRange(uint8_t range)
{
    IICwriteBits(devAddr, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range);
}


//���� MPU6050 ���ٶȼƵ��������
void MPU6050_setFullScaleAccelRange(uint8_t range) 
{
    IICwriteBits(devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range);
}

//���� MPU6050 �Ƿ����˯��ģʽ, enabled =1˯��,enabled =0   ����
void MPU6050_setSleepEnabled(uint8_t enabled) 
{
    IICwriteBit(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled);
}

//���� MPU6050 �Ƿ�ΪAUX I2C�ߵ�����
void MPU6050_setI2CMasterModeEnabled(uint8_t enabled) 
{
    IICwriteBit(devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_EN_BIT, enabled);
}



//����MPU6050�Ƿ���ƴ�IIC�豸
void MPU6050_setI2CBypassEnabled(uint8_t enabled) 
{
    IICwriteBit(devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_I2C_BYPASS_EN_BIT, enabled);
}


void MPU6050_setDLPF(uint8_t mode)
{
    IICwriteBits(devAddr, MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, mode);
}



//��ʼ�� MPU6050
void MPU6050_INIT(void)
{
    MPU6050_Delay(2000);   //��ʱ����Ҫ
    MPU6050_setClockSource(MPU6050_CLOCK_PLL_XGYRO); //����ʱ��  0x6b   0x01
    MPU6050_Delay(2000);
    MPU6050_setFullScaleGyroRange(MPU6050_GYRO_FS_500);//������������� +-500��ÿ��
    MPU6050_Delay(2000);
    MPU6050_setFullScaleAccelRange(MPU6050_ACCEL_FS_4);	//���ٶȶ�������� +-4G
    MPU6050_Delay(2000);
    MPU6050_setDLPF(MPU6050_DLPF_BW_42);
    MPU6050_Delay(2000);
    MPU6050_setSleepEnabled(0); //���빤��״̬
    MPU6050_Delay(2000);
    MPU6050_setI2CMasterModeEnabled(0);	 //����MPU6050 ����AUXI2C
    MPU6050_Delay(2000);
    MPU6050_setI2CBypassEnabled(1);  //����������I2C��MPU6050��AUXI2Cֱͨ������������ֱ�ӷ���HMC5883L
    MPU6050_Delay(2000);
}

//��ʼ�� MPU6050
//mpu6050��ʼ��˳��
//1.��ʼ��IIC����
//2.���øú���MPU6050_Check()����豸�������
//3.���豸���ڣ����ʼ�����豸�����򲻳�ʼ��
char MPU6050_Check(void)
{
      uint8_t buf1[5]={0xaa,0xaa,0xaa,0xaa,0xaa};
      uint8_t buf2[5];
      char i;
      for(i=0;i<5;i++)
      IICwriteBits(devAddr, 0x00+i, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, buf1[i]);
      for(i=0;i<5;i++)
      i2cRead(devAddr,0x00+i,MPU6050_CFG_DLPF_CFG_LENGTH,buf2);//��0x00����Ĵ���д��5�������ٴ�����Ĵ�������5����
      
       /*�Ƚ�*/ 
       for (i=0;i<5;i++) 
       { 
          if (buf1[i]!=0xaa) //�����������Ϊ�����趨��������˵���ⲿ�������豸������û�����ӳɹ�
          break; 
       } 
      
       if (i==5)   return 1 ;        //MCU ��MPU6050 �ɹ����� 
       else        return 0 ;        //MCU��MPU6050����������  
}
