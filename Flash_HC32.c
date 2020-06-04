/***************************************************************************

                     Flash��׼�ӿ�-��Hc32�е�ʵ��
�� FLASH����ҳ�����Ĵ������ڵ�ַ����С�� 32768
****************************************************************************/

#include "Flash.h"
#include "Delay.h"
#include "HC32.h"
#include "string.h" //memcpy
unsigned long Flash_ErrCount = 0; //ʧ�ܼ�����


/***************************************************************************
                              ����˵��
�����ڴ��޸�����Ӱ��һ���ԣ�
****************************************************************************/

//#define SUPPORT_MDK   //Keil MDK����ʱ���趨��

//ע��SYS_MHZΪȫ�ֶ���
#define _DELAY_MUTI        ((SYS_MHZ + 3) / 4)   //4MΪ��׼������ʱ
#define FLASH              M0P_FLASH       //����

#ifndef FLASH_BASE //FALSH��ַ
  #define FLASH_BASE   0x00000000 //Ĭ��Ϊ0
#endif

#ifndef FLASH_SLOCK_SIZE //ÿ��SLOCK�����ķ�Χ
  #ifdef FLASH_COUNT
    #define FLASH_SLOCK_SIZE   ��FLASH_COUNT / 32�� //��Ϊ32��
  #else
    #define FLASH_SLOCK_SIZE   2048    //Ĭ��2Kһ��
  #endif
#endif

/***************************************************************************
                             �ڲ�����ʵ��
****************************************************************************/

//-----------------------------д�����Ĵ�������----------------------------
//ֻ��ָ��Ԥ�в���дFLASH�ṹ�Ĵ���������ô˺���
#ifdef SUPPORT_MDK    //�����ƴ˺�����32k��, Keil MDK��������
  void _WrProtReg(__IO uint32_t *pReg,
                  unsigned long Data) __attribute__((section(".ARM.__at_0x400")));
  static void _WrProtReg(__IO uint32_t *pReg,        //��д����ܱ����Ĵ���
                          unsigned long Data)
#else // IAR��������, Flash����<32kʱ�ɲ���icf�ļ���ָ��
  static void _WrProtReg(__IO uint32_t *pReg,        //��д����ܱ����Ĵ���
                          unsigned long Data)@".Flash_WrProtReg" //IAR��������
#endif
{
  unsigned short Count = 255 * _DELAY_MUTI; //�����е�ֵ
  for(; Count > 0; Count--){
    FLASH->BYPASS = 0x5A5A;
    FLASH->BYPASS = 0xA5A5;
    *pReg = Data;
    if(*pReg == Data) return; //д�ɹ���
  }
  Flash_ErrCount++; //д�������
}

//-----------------------------Flashд������ɵȴ�����--------------------------
//����ֵͬFlash_ErasePage()
static void _WaitDone(unsigned short Time)//IAR��������
{
  for(; Time > 0; Time--){
    if(!(M0P_FLASH->CR_f.BUSY)) return; //��æʱд����
    DelayUs(1);
  }
  Flash_ErrCount++; //д�������
}

//------------------------------�Ĵ���Ĭ��ֵ------------------------------------
#if (_DELAY_MUTI > 1)
  static const unsigned long _DelayRegDefault[] ={
    0x20,      //Tnvs ʱ�����
    0x17,      //Tpgs ʱ�����
    0x1b,      //Tprog ʱ�����
    0x4650,    //Tserase ʱ�����
    0x222E0,  //Tmerase ʱ�����
    0x18,     //Tprcv ʱ�����
    0xF0,     //Tsrcv ʱ�����
    0x03E8,   //Tmrcvʱ�����
  };

#endif //_DELAY_MUTI


/***************************************************************************
                        ��׼�ӿں���ʵ��
****************************************************************************/

//------------------------------��ʼ������------------------------------------
void Flash_Init(void)
{
  //�޸�Ĭ�϶�ʱֵ��ϵͳ��ͬ
  //����оƬ���ϣ�Ĭ��ֵ����4MΪ����������Ӧ����ϵͳ���ӳɱ�����
  #if (_DELAY_MUTI > 1)
    __IO uint32_t *pDelayReg = &FLASH->TNVS; //��ʱֵ��ʼ����8��
    for(unsigned char i = 0; i < 8; i++, pDelayReg++)
      _WrProtReg(pDelayReg, _DelayRegDefault[i] * _DELAY_MUTI);
  #endif
  
  //�����ö��ȴ�����
  //����оƬ����,��24Mʱÿ���24M��һֵ
  #if ((_DELAY_MUTI / 6) > 1)
    unsigned long Data = FLASH->CR & ~0x0C;
      _WrProtReg(&FLASH->CR, Data | ((_DELAY_MUTI / 6) << 2));
  #endif
}

//---------------------------Flash����ʵ��---------------------------------
void Flash_Unlock(void)
{
  //����ʵ��Ϊ��
}

//---------------------------Flash����ʵ��---------------------------------
void Flash_Lock(void)
{
  //����ʵ��Ϊ��  
}

//---------------------------------Flashҳ��������ʵ��-----------------------
//�˺���������ҳ������������ӽ�����
//ͳһ����ֵ����: 0��� 1æ 2��̴��� 3д��������
#ifdef SUPPORT_MDK    //�����ƴ˺�����32k��, Keil MDK��������
  void Flash_InErasePage(unsigned long Adr) __attribute__((section(".ARM.__at_0x500")));
  void Flash_ErasePage(unsigned long Adr) {Flash_InErasePage(Adr); }
  void Flash_InErasePage(unsigned long Adr)
#else // IAR��������, Flash����<32kʱ�ɲ���icf�ļ���ָ��
void Flash_ErasePage(unsigned long Adr)@".Flash_ErasePage"
#endif
{
  __disable_irq();
  _WrProtReg(&FLASH->CR, FLASH->CR | 2);  //����ҳ����ģʽ
  _WrProtReg(&FLASH->SLOCK, 1 << ((Adr - FLASH_BASE) / FLASH_SLOCK_SIZE));//ȡ��ҳ����
  *(volatile unsigned char*)Adr = 0xff; //д��������������
  _WaitDone(21000);   //�ȴ���������,>20msΪ����ֵ
  _WrProtReg(&FLASH->CR, FLASH->CR & ~0x03);  //�˳�ҳ����ģʽ
  _WrProtReg(&FLASH->SLOCK, 0);//����ҳ����
  __enable_irq();
}

//-------------------------д���ݵ�Flash��ʵ��----------------------------
//�˺�����������Flashд���ݣ�������ӽ���������
#ifdef SUPPORT_MDK    //�����ƴ˺�����32k��, Keil MDK��������
  void Flash_InWrite(unsigned long Adr,const void *pVoid,unsigned long Len) \
                                    __attribute__((section(".ARM.__at_0x600")));
  void Flash_Write(unsigned long Adr,const void *pVoid,unsigned long Len)
  {Flash_InWrite(Adr, pVoid, Len); }
  void Flash_InWrite(unsigned long Adr,const void *pVoid,unsigned long Len)
#else // IAR��������, Flash����<32kʱ�ɲ���icf�ļ���ָ��
void Flash_Write(unsigned long Adr,   //Flash��ַ
                 const void *pVoid,  //Ҫд�������
                 unsigned long Len)@".Flash_Write" //IAR��������
#endif
{
  //дǰ׼����
  __disable_irq();
  _WrProtReg(&FLASH->CR, FLASH->CR | 1);  //����дģʽ
  _WrProtReg(&FLASH->SLOCK, 1 << ((Adr - FLASH_BASE) / FLASH_SLOCK_SIZE));//ȡ��ҳ����
  
  const unsigned char *pData = (const unsigned char *)pVoid;
  //д���ַû4�ֶ���ʱ����һ��һ��д���Զ���
  for( ; (Adr & 0x03) && (Len > 0); Adr++, Len--, pData++){
    *(volatile unsigned char*)Adr = *pData;     //д������
    _WaitDone(10000);                 //�ȴ��������,10msΪ����ֵ
  }
  if(Len == 0) goto _EndWrPro;//д����  
  
  //�м�4�ֽڶ���д��
  for( ; Len > 3; Len -= 4, Adr += 4, pData+= 4){  
    *(volatile unsigned long*)Adr = *(const unsigned long*)pData; //д������
    _WaitDone(10000);                 //�ȴ��������,10msΪ����ֵ
  }
  if(Len == 0) goto _EndWrPro;//д����  
  
  //���δ4�ֽڶ����ַ
  for( ;Len > 0; Adr++, Len--, pData++){
    *(volatile unsigned char*)Adr = *pData;     //д������
    _WaitDone(10000);                 //�ȴ��������,10msΪ����ֵ
  }
  
  _EndWrPro: //����д����
    _WrProtReg(&FLASH->CR, FLASH->CR & ~0x03);  //ȡ��дģʽ
    _WrProtReg(&FLASH->SLOCK, 0);//����ҳ����
    __enable_irq();
}

//----------------------------��Flash�ж�ȡ����ʵ��----------------------
//�˺���Ϊ��ѡ����,�ɲ�ʵ��
void Flash_Read(unsigned long Adr,   //Flash��ַ
                void *pVoid,        //Ҫ����������
                unsigned long Len)   //�����ݳ���
{
  memcpy(pVoid, (unsigned char*)Adr, Len);
}




