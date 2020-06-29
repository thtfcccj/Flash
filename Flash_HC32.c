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

//#define SUPPORT_MDK   //Keil MDK����ʱ����ȫ�ֶ���

//ע��SYS_MHZΪȫ�ֶ���
#define _DELAY_MUTI        ((SYS_MHZ + 3) / 4)   //4MΪ��׼������ʱ
#define FLASH              M0P_FLASH       //����

#ifndef FLASH_BASE //FALSH��ַ
  #define FLASH_BASE   0x00000000 //Ĭ��Ϊ0
#endif

#ifndef FLASH_CAPABILITY //FALSH����
  #define FLASH_CAPABILITY   0x00010000 //Ĭ��Ϊ64K
#endif

#ifndef FLASH_SLOCK_SIZE //ÿ��SLOCK�����ķ�Χ
  #define FLASH_SLOCK_SIZE   2048    //Ĭ��2Kһ��
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

//-----------------------------Sector��������-------------------------------------
static void _ProtSector(unsigned long Adr,     //Flash��ַ
                         signed char IsCancel)  //0������1ȡ��  
{
  unsigned short Offset = (Adr - FLASH_BASE) / FLASH_SLOCK_SIZE;
  unsigned long ProtFlag;
  if(IsCancel) ProtFlag = 0; //�����Ĵ���ȫ��д0
  else ProtFlag  = 1 << (Offset & 0x1f);//ȡ��ʱ��Ӧλд1
  
  #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) <= 32)//һҳ��д����
    _WrProtReg(&FLASH->SLOCK, ProtFlag);
  #else //��ҳʱ,���2KΪ��λʱ������1M
    switch(Offset >> 5){ //�ҵ���Ӧ��Ĵ���(���в�����)
      case 0: _WrProtReg(&FLASH->SLOCK0, ProtFlag); break;
      case 1: _WrProtReg(&FLASH->SLOCK1, ProtFlag); break;
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (2 * 32))//��2��
      case 2: _WrProtReg(&FLASH->SLOCK2, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (3 * 32))//��3��
      case 3: _WrProtReg(&FLASH->SLOCK3, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (4 * 32))//��4��
      case 4: _WrProtReg(&FLASH->SLOCK4, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (5 * 32))//��5��
      case 5: _WrProtReg(&FLASH->SLOCK5, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (6 * 32))//��6��
      case 6: _WrProtReg(&FLASH->SLOCK6, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (7 * 32))//��7��
      case 7: _WrProtReg(&FLASH->SLOCK7, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (8 * 32))//��8��
      case 8: _WrProtReg(&FLASH->SLOCK8, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (9 * 32))//��9��
      case 9: _WrProtReg(&FLASH->SLOCK9, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (10 * 32))//��10��
      case 10: _WrProtReg(&FLASH->SLOCK10, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (11 * 32))//��11��
      case 11: _WrProtReg(&FLASH->SLOCK11, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (12 * 32))//��12��
      case 12: _WrProtReg(&FLASH->SLOCK12, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (13 * 32))//��13��
      case 13: _WrProtReg(&FLASH->SLOCK13, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (14 * 32))//��14��
      case 14: _WrProtReg(&FLASH->SLOCK14, ProtFlag); break;
      #endif
      #if ((FLASH_CAPABILITY / FLASH_SLOCK_SIZE) > (15 * 32))//��15��
      case 15: _WrProtReg(&FLASH->SLOCK15, ProtFlag); break;
      #endif
      default: break;
    }
  #endif//��ҳʱ
}
                     
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
  _ProtSector(Adr, 1);//ȡ����������
  *(volatile unsigned char*)Adr = 0xff; //д��������������
  _WaitDone(21000);   //�ȴ���������,>20msΪ����ֵ
  _WrProtReg(&FLASH->CR, FLASH->CR & ~0x03);  //�˳�ҳ����ģʽ
  _ProtSector(Adr, 0);  //������������
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
  _ProtSector(Adr, 1);//ȡ����������
  
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
  _ProtSector(Adr, 0);  //������������
    __enable_irq();
}

//----------------------------��Flash�ж�ȡ����ʵ��----------------------
//�˺���Ϊ��ѡ����,�ɲ�ʵ��
void Flash_Read(unsigned long Adr,   //Flash��ַ
                void *pVoid,          //Ҫ����������
                unsigned long Len)   //�����ݳ���
{
  memcpy(pVoid, (unsigned char*)Adr, Len);
}




