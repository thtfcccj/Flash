/***************************************************************************

                     Flash �ڲ����ڼ�Ҫִ�еĴ���ʾ�� 
������IAR����Ϊ��, Ӧ�ô����Ѳ��Գɹ�
****************************************************************************/
#ifdef SUPPORT_FLASH_ERASE_IN_RAM1 //����ȥ����׺1�󣬿�ֱ���޸�ʹ��

#include "Flash.h"
#include "stm32f4xx.h"

//�û��������
#include "IoCtrl.h"

//----------------------�ڲ����ڼ�Ҫִ�еĴ���-----------------------------
//__ramfunc ��ʾ������ram��ִ��,  Ϊ����RAMռ�ã�����Ӧ��������
//�˺�����Ҫ�����ⲿ������Ҫ����ҲҪ�������ú�������Ϊ __ramfunc 
__ramfunc static void _InRam(void)
{
  //����ֲ�������������س�ʼ��,������ɨ�������ִ��Ϊ��
  
  unsigned short Period; //���ڼ���(�������Խ�ʡ����)
  unsigned char ScanId; //ɨ��λ(�������Խ�ʡ����)
  
  //��ʼ����
  FLASH->CR |= FLASH_CR_STRT;
  do{
    if(Period) Period--;//����δ��
    else{
      Period = 10000;//���������ܼ�ʱ����    
      ScanId++;
      if(ScanId > 4) ScanId = 0;//һ������
      //�����л�ɨ�������(Ӧ�ô�����)
      
    }//end else
  }while(FLASH->SR & FLASH_SR_BSY);//æʱ����
}

//------------------------�ص�����ʵ��-----------------------------
//Ӧ������ؽ�����Ҫ��RAM��ִ�еĴ�����ڴ�
void Flash_cbEraseInRam(void)
{
  //��ǰ��Ԥ����,
  ;  //�磺����ܲ�������Ϊ���
  
  __disable_irq();   // �ر����ж�
  _InRam();      
  __enable_irq();    // �������ж�
  
  
  //������ڻָ�
  
}



#endif //SUPPORT_FLASH_ERASE_IN_RAM1
