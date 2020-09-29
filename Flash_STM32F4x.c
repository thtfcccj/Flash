/***************************************************************************

                     Flash��׼�ӿ�-��STM32F4X�е�ʵ��
�ٶ�Ϊ�ⲿ2.7~3.6V���磬160MHZʱ����״̬
****************************************************************************/

#include "Flash.h"
#include "Delay.h"
#include "stm32f4xx.h"
#include "string.h" //memcpy
#include "IoCtrl.h" //WDT_Week()

unsigned long Flash_ErrCount = 0; //ʧ�ܼ�����

/***************************************************************************
                           �������
****************************************************************************/

#ifndef FLASH_VOLTAGE_RANGE
#define FLASH_VOLTAGE_RANGE  2   //�����ѹ��Χ: 0:<2.1V   1:< 2.1~2.7V:
//                                             2:2.7~3.6V   3:�ⲿ����Vpp2.7~3.6V����
#endif // FLASH_VOLTAGE_RANGE

/***************************************************************************
                        �ڲ�����ʵ��
****************************************************************************/
#define _ERR_MASK (FLASH_SR_WRPERR | FLASH_SR_WRPERR |  FLASH_SR_PGPERR | FLASH_SR_PGSERR)

//---------------------------Flash����ʵ��---------------------------------
static void _Unlock(void)
{
  FLASH->KEYR = 0X45670123;
  FLASH->KEYR = 0XCDEF89AB;
}

//---------------------------Flash����ʵ��---------------------------------
static void _Lock(void)
{
  FLASH->CR |= FLASH_CR_LOCK;
}

//-----------------------------Flashд��������--------------------------
//����ֵͬFlash_ErasePage()
static void _WaitDone(unsigned short Time) //�ȴ�usʱ��
{
  volatile unsigned long State; //ǿ�ƶ�ȡ
  for(; Time > 0; Time--){
    State = FLASH->SR;
    if(!(State & FLASH_SR_BSY)){//��æʱ
      break;
    }
    DelayUs(1);
  }
  //ι��
  WDT_Week();
  
  if(!(State  & _ERR_MASK)) return;
  
  //�쳣ʱ���������й����־λ��ֹ����  
  Flash_ErrCount++;
  FLASH->SR |= _ERR_MASK;//��λ�����
  if(FLASH->SR &  _ERR_MASK){//�Բ������ʱ
    //Flash_ErrCount++;
  }
}
//------------------------˫��(8Byte)����д�뵽Flashָ����ַ����------------------
#if FLASH_VOLTAGE_RANGE == 3
static void _Write2Word(unsigned long long *pAdr,    //Flash��ַ
                         unsigned long long data)     //Ҫд�������
{
  _Unlock();//����
  //��ʼ
  FLASH->CR |= FLASH_CR_PSIZE;      //64bitд��ģʽ
  FLASH->CR |= FLASH_CR_PG;        //���ʹ��
  *pAdr = data;                    //д������
  _WaitDone(10000);                //�ȴ��������
  FLASH->CR &= ~FLASH_CR_PG;      //������������ʹ��λ.
  
  _Lock();//����
}
#endif

//------------------------����(32Byte)����д�뵽Flashָ����ַ����----------------
#if FLASH_VOLTAGE_RANGE == 2
static void _WriteWord(unsigned long *pAdr,    //Flash��ַ
                         unsigned long data)     //Ҫд�������
{
  _Unlock();//����
  //��ʼ
  FLASH->CR &= ~FLASH_CR_PSIZE_0;  //32bitд��ģʽ
  FLASH->CR |= FLASH_CR_PSIZE_1;
  FLASH->CR |= FLASH_CR_PG;        //���ʹ��
  *pAdr = data;                    //д������
  _WaitDone(10000);                //�ȴ��������
  FLASH->CR &= ~FLASH_CR_PG;      //������������ʹ��λ.
  
  _Lock();//����
}
#endif

//------------------------˫�ֽ�����д�뵽Flashָ����ַ����------------------
#if FLASH_VOLTAGE_RANGE == 1
static void _Write2Byte(unsigned short *pAdr,    //Flash��ַ
                        unsigned short data) //Ҫд�������
{
  _Unlock();//����
  //��ʼ
  FLASH->CR &= ~FLASH_CR_PSIZE_1;      //16bitд��ģʽ
  FLASH->CR |= FLASH_CR_PSIZE_0;
  FLASH->CR |= FLASH_CR_PG;          //���ʹ��
  *pAdr = data;                     //д������
  _WaitDone(10000);                 //�ȴ��������
  FLASH->CR &= ~FLASH_CR_PG;        //������������ʹ��λ.
  
  _Lock();//����
}
#endif

//------------------------�ֽ�����д�뵽Flashָ����ַ����------------------
static void _WriteByte(unsigned char *pAdr,    //Flash��ַ
                        unsigned char data) //Ҫд�������
{
   _Unlock();//����
  //��ʼ
  FLASH->CR &= ~FLASH_CR_PSIZE;      //8bitд��ģʽ
  FLASH->CR |= FLASH_CR_PG;          //���ʹ��
  *pAdr = data;                     //д������
  _WaitDone(10000);                 //�ȴ��������
  FLASH->CR &= ~FLASH_CR_PG;        //������������ʹ��λ.
  
  _Lock();//����
} 

//------------------------�ɻ�ַ��SNBת��-----------------------
//����SNBֵ(��λFLASH_CR_SNB_SHIFT��)
unsigned long _Adr2SNB(unsigned long Adr)
{
  #ifdef FLASH_1M_BANK_S//1M��BANKʱ
    if(Adr < 0x08004000) return 0 << FLASH_CR_SNB_SHIFT;
    if(Adr < 0x08008000) return 1 << FLASH_CR_SNB_SHIFT;  
    if(Adr < 0x0800C000) return 2 << FLASH_CR_SNB_SHIFT;
    if(Adr < 0x08010000) return 3 << FLASH_CR_SNB_SHIFT;
    if(Adr < 0x08020000) return 4 << FLASH_CR_SNB_SHIFT;
    if(Adr < 0x08040000) return 5 << FLASH_CR_SNB_SHIFT; 
    if(Adr < 0x08060000) return 6 << FLASH_CR_SNB_SHIFT;
    if(Adr < 0x08080000) return 7 << FLASH_CR_SNB_SHIFT; 
    if(Adr < 0x080A0000) return 8 << FLASH_CR_SNB_SHIFT; 
    if(Adr < 0x080C0000) return 9 << FLASH_CR_SNB_SHIFT;
    if(Adr < 0x080E0000) return 10 << FLASH_CR_SNB_SHIFT;
    return 11 << FLASH_CR_SNB_SHIFT;
  #endif
}

/***************************************************************************
                        ��׼�ӿں���ʵ��
****************************************************************************/

//------------------------��ʼ������------------------------------
void Flash_Init(void)
{
  _Lock();//��ֹ�쳣����δ��������
}

//---------------------------------Flashҳ��������ʵ��-----------------------
//�˺���������ҳ������������ӽ�����
//ͳһ����ֵ����: 0��� 1æ 2��̴��� 3д��������
void Flash_ErasePage(unsigned long Adr)
{
  Flash_cbErasePageStartNotify(); //����ǰͨ��
  _Unlock();//����
  
  FLASH->CR &= ~FLASH_CR_PSIZE;  //ǿ������͵�ѹ��8Byteģʽ������
  FLASH->CR |= FLASH_CR_SER;  //ҳ����ģʽ
  FLASH->CR &= ~FLASH_CR_SNB; //���λ��
  FLASH->CR |= _Adr2SNB(Adr); //ָ��λ��
  
  //�����ڼ�Ҫִ�д���ʱ���ڷ���RAM��ִ��
  #ifndef SUPPORT_FLASH_ERASE_IN_RAM
    //��ʼ����(ʵ���ڲ���FLASH���ڼ䣬��ȡ��ȡָ��FLASH���ᱻ��ͣ)
    FLASH->CR |= FLASH_CR_STRT; 
    //��ʱ��ι��(
    for(unsigned char i = 210; i > 0; i--){
      _WaitDone(1000);   //�ȴ���������
    }
  #else
    Flash_cbEraseInRam(); //����RAM��ִ�еĴ���
  #endif
  FLASH->CR &= ~(1 << 1);      //���������ҳ������־
  _Lock();//����
}

//-------------------------д���ݵ�Flash��ʵ��----------------------------
//�˺�����������Flashд���ݣ�������ӽ���������
void Flash_Write(unsigned long Adr,   //Flash��ַ
                 const void *pVoid,  //Ҫд�������
                 unsigned long Len)   //д���ݳ���  
{
  unsigned char *pOrgData = (unsigned char*)pVoid;
  unsigned char *pWrAdr = (unsigned char*)Adr;
  
  //д���ַû˫�ֶ���ʱ����һ��һ��д���Զ���  
  if(Adr & 0x07){
    unsigned char CurLen;
    if(Len > 7) CurLen = 7;
    else CurLen = Len;
    Len -= CurLen;
    for(; CurLen > 0; CurLen--, pOrgData++, pWrAdr++){
      _WriteByte(pWrAdr, *pOrgData);
    }
    if(Len == 0){//д����
      return;
    }
  }
  
  //�ⲿVPPʱ��һ��д���8Byte
  #if FLASH_VOLTAGE_RANGE == 3
  //���ﱣ֤д���ַ˫�ֶ�����(������ȷ���û����ݶ��룬��ֻ��memcpy)
  unsigned long long *pllWrAdr = (unsigned long long *)pWrAdr;
  for(; Len >= 8; Len -= 8, pOrgData += 8, pllWrAdr++){
    unsigned long long llData;
    memcpy(&llData, pOrgData, 8);
    _Write2Word(pllWrAdr, llData);
    Flash_cbWrIdieNotify(); //д����ͨ��
  }
  pWrAdr = (unsigned char*)pllWrAdr;
  #endif
  
  //�ڲ�2.7~3.6Vʱ��һ��д���4Byte
  #if FLASH_VOLTAGE_RANGE == 2
  //���ﱣ֤д���ַ˫�ֶ�����(������ȷ���û����ݶ��룬��ֻ��memcpy)
  unsigned long *plWrAdr = (unsigned long *)pWrAdr;
  for(; Len >= 4; Len -= 4, pOrgData += 4, plWrAdr++){
    unsigned long lData;
    memcpy(&lData, pOrgData, 4);
    _WriteWord(plWrAdr, lData);
    Flash_cbWrIdieNotify(); //д����ͨ��
  }
  pWrAdr = (unsigned char*)plWrAdr;
  #endif
  
  //�ڲ�2.1~2.7Vʱ��һ��д���2Byte
  #if FLASH_VOLTAGE_RANGE == 1
  //���ﱣ֤д���ַ˫�ֶ�����(������ȷ���û����ݶ��룬��ֻ��memcpy)
  unsigned short *psWrAdr = (unsigned short *)pWrAdr;
  for(; Len >= 2; Len -= 2, pOrgData += 2, psWrAdr++){
    unsigned short sData;
    memcpy(&sData, pOrgData, 4);
    _Write2Byte(psWrAdr, sData);
    Flash_cbWrIdieNotify(); //д����ͨ��
  }
  pWrAdr = (unsigned char*)psWrAdr;
  #endif
  
  //�ڲ�2.1~����ʱʱ��һ��д���1Byte,����
  #if FLASH_VOLTAGE_RANGE == 0
  #endif
  
  //���²���һ��һ��д���Զ���
  for(; Len > 0; Len--, pOrgData++, pWrAdr++){
    _WriteByte(pWrAdr, *pOrgData);
  }
}

//----------------------------��Flash�ж�ȡ����ʵ��----------------------
//�˺���Ϊ��ѡ����,�ɲ�ʵ��
void Flash_Read(unsigned long Adr,   //Flash��ַ
                void *pVoid,        //Ҫ����������
                unsigned long Len)   //�����ݳ���
{
  memcpy(pVoid, (unsigned char*)Adr, Len);
}




