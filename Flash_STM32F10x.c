/***************************************************************************

                     Flash��׼�ӿ�-��STM32F10X�е�ʵ��

****************************************************************************/

#include "Flash.h"
#include "Delay.h"
#include "stm32f10x.h"
#include "string.h" //memcpy

unsigned long Flash_ErrCount = 0; //ʧ�ܼ�����

/***************************************************************************
                        �ڲ�����ʵ��
****************************************************************************/
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
  if(!(State  & (FLASH_SR_PGERR | FLASH_SR_WRPRTERR))) return;
  
  //�쳣ʱ���������й����־λ��ֹ����  
  Flash_ErrCount++;
  FLASH->SR |= (FLASH_SR_PGERR | FLASH_SR_WRPRTERR);//��λ�����
  if(FLASH->SR &  (FLASH_SR_PGERR | FLASH_SR_WRPRTERR)){//�Բ������ʱ
    //Flash_ErrCount++;
  }
}

/*/------------------------��������д�뵽Flashָ����ַ����------------------
static void _WriteWord(unsigned long Adr,    //Flash��ַ
                        unsigned long data) //Ҫд�������
{
  //��ʼ
  FLASH->CR |= 1 << 0;              //���ʹ��
  *(unsigned long*)Adr = data;     //д������
  _WaitDone(10000);                 //�ȴ��������
  FLASH->CR &= ~(1 << 0);           //������������ʹ��λ.
}*/

//------------------------��������д�뵽Flashָ����ַ����------------------
static void _WriteHalfWord(unsigned long Adr,    //Flash��ַ
                           unsigned short data) //Ҫд�������
{
  //��ʼ
  FLASH->CR |= 1 << 0;              //���ʹ��
  *(unsigned short*)Adr = data;     //д������
  _WaitDone(10000);                 //�ȴ��������
  FLASH->CR &= ~(1 << 0);           //������������ʹ��λ.
} 

/***************************************************************************
                        ��׼�ӿں���ʵ��
****************************************************************************/

//------------------------��ʼ������------------------------------
void Flash_Init(void)
{
}


//---------------------------Flash����ʵ��---------------------------------
void Flash_Unlock(void)
{
  FLASH->KEYR = 0X45670123;
  FLASH->KEYR = 0XCDEF89AB;
}

//---------------------------Flash����ʵ��---------------------------------
void Flash_Lock(void)
{
  FLASH->CR |= 1<<7;
}

//---------------------------------Flashҳ��������ʵ��-----------------------
//�˺���������ҳ������������ӽ�����
//ͳһ����ֵ����: 0��� 1æ 2��̴��� 3д��������
void Flash_ErasePage(unsigned long Adr)
{
  //��������ʱ->�������ж�״̬
  FLASH->CR |= 1 << 1;         //��ҳ������־
  FLASH->AR = Adr;             //����ҳ��ַ 
  FLASH->CR |= 1 << 6;         //��ʼ����
  _WaitDone(21000);   //�ȴ���������,>20ms����  
  FLASH->CR &= ~(1 << 1);      //���������ҳ������־
}

//-------------------------д���ݵ�Flash��ʵ��----------------------------
//�˺�����������Flashд���ݣ�������ӽ���������
void Flash_Write(unsigned long Adr,   //Flash��ַ
                 const void *pVoid,  //Ҫд�������
                 unsigned long Len)   //д���ݳ���  
{
  const unsigned char *p = pVoid;             
  //for(unsigned long i = 0; i < Len; i += 4)//һ��4��
  //  _WriteWord(Adr + i, *((unsigned long*)(p + i)));
  for(unsigned long i = 0; i < Len; i += 2)//һ��2��
    _WriteHalfWord(Adr + i, *((unsigned short*)(p + i)));  
}

//----------------------------��Flash�ж�ȡ����ʵ��----------------------
//�˺���Ϊ��ѡ����,�ɲ�ʵ��
void Flash_Read(unsigned long Adr,   //Flash��ַ
                void *pVoid,        //Ҫ����������
                unsigned long Len)   //�����ݳ���
{
  memcpy(pVoid, (unsigned char*)Adr, Len);
}




