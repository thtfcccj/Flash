/***************************************************************************

                     Flash��׼�ӿ�-��Hc32�е�ʵ��
�� FLASH����ҳ�����Ĵ������ڵ�ַ����С�� 32768
****************************************************************************/

#include "Flash.h"
#include "Delay.h"
#include "hc32f030e8pa.h" //�Դ�Ϊ��
#include "string.h" //memcpy
unsigned long Flash_ErrCount = 0; //ʧ�ܼ�����


/***************************************************************************
                              ����˵��
�����ڴ��޸�����Ӱ��һ���ԣ�
****************************************************************************/
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
//��ô�����ƴ˺�����32k�ڣ�����

//ֻ��ָ��Ԥ�в���дFLASH�ṹ�Ĵ���������ô˺���
static void _WrProtReg(__IO uint32_t *pReg,        //��д����ܱ����Ĵ���
                       unsigned long Data)      //�����������
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

//-----------------------------Flashд��������--------------------------
//��ô�����ƴ˺�����32k�ڣ�����

//����ֵͬFlash_ErasePage()
static void _WaitDone(unsigned short Time) //�ȴ�usʱ��
{
  for(; Time > 0; Time--){
    if(!(M0P_FLASH->CR_f.BUSY)) return; //��æʱд����
    DelayUs(1);
  }
  Flash_ErrCount++; //д�������
}

/***************************************************************************
                        ��׼�ӿں���ʵ��
****************************************************************************/

//------------------------��ʼ������------------------------------
void Flash_Init(void)
{
  //�޸�Ĭ�϶�ʱֵ��ϵͳ��ͬ
  //����оƬ���ϣ�Ĭ��ֵ����4MΪ����������Ӧ����ϵͳ���ӳɱ�����
  if(_DELAY_MUTI > 1){
    __IO uint32_t *pDelayReg = &FLASH->TNVS; //��ʱֵ��ʼ����8��
    __IO uint32_t *pEndDelayReg = pDelayReg + 8;
    for(; pDelayReg < pEndDelayReg; pDelayReg++)
      _WrProtReg(pDelayReg, *pDelayReg * _DELAY_MUTI);
  }
  
  //�����ö��ȴ�����
  //����оƬ����,��24Mʱÿ���24M��һֵ
  if((_DELAY_MUTI / 6) > 0){
    unsigned long Data = FLASH->CR & ~0xC0;
      _WrProtReg(&FLASH->CR, Data | ((_DELAY_MUTI / 6) << 2));
  }
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
//��ô�����ƴ˺�����32k�ڣ�����

//�˺���������ҳ������������ӽ�����
//ͳһ����ֵ����: 0��� 1æ 2��̴��� 3д��������
void Flash_ErasePage(unsigned long Adr)
{
  _WrProtReg(&FLASH->CR, FLASH->CR | 2);  //����ҳ����ģʽ
  _WrProtReg(&FLASH->SLOCK, 1 << ((Adr - FLASH_BASE) / FLASH_SLOCK_SIZE));//ȡ��ҳ����
  *(volatile unsigned char*)Adr = 0xff; //д��������������
  _WaitDone(21000);   //�ȴ���������,>20msΪ����ֵ
  _WrProtReg(&FLASH->CR, FLASH->CR & 0x03);  //�˳�ҳ����ģʽ
  _WrProtReg(&FLASH->SLOCK, 0);//����ҳ����
}

//-------------------------д���ݵ�Flash��ʵ��----------------------------
//��ô�����ƴ˺�����32k�ڣ�����

//�˺�����������Flashд���ݣ�������ӽ���������

void Flash_Write(unsigned long Adr,   //Flash��ַ
                 const void *pVoid,  //Ҫд�������
                 unsigned long Len)   //д���ݳ���  
{
  //дǰ׼����
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
    _WrProtReg(&FLASH->CR, FLASH->CR & 0x03);  //ȡ��дģʽ
    _WrProtReg(&FLASH->SLOCK, 0);//����ҳ����
}

//----------------------------��Flash�ж�ȡ����ʵ��----------------------
//�˺���Ϊ��ѡ����,�ɲ�ʵ��
void Flash_Read(unsigned long Adr,   //Flash��ַ
                void *pVoid,        //Ҫ����������
                unsigned long Len)   //�����ݳ���
{
  memcpy(pVoid, (unsigned char*)Adr, Len);
}




