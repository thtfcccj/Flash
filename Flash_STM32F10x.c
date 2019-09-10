/***************************************************************************

                     Flash标准接口-在STM32F10X中的实现

****************************************************************************/

#include "Flash.h"
#include "Delay.h"
#include "stm32f10x.h"
#include "string.h" //memcpy

unsigned long Flash_ErrCount = 0; //失败计数器

/***************************************************************************
                        内部函数实现
****************************************************************************/
//-----------------------------Flash写操作函数--------------------------
//返回值同Flash_ErasePage()
static void _WaitDone(unsigned short Time) //等待us时间
{
  volatile unsigned long State; //强制读取
  for(; Time > 0; Time--){
    State = FLASH->SR;
    if(!(State & FLASH_SR_BSY)){//非忙时
      break;
    }
    DelayUs(1);
  }
  if(!(State  & (FLASH_SR_PGERR | FLASH_SR_WRPRTERR))) return;
  
  //异常时，清所所有挂起标志位防止出错  
  Flash_ErrCount++;
  FLASH->SR |= (FLASH_SR_PGERR | FLASH_SR_WRPRTERR);//置位以清除
  if(FLASH->SR &  (FLASH_SR_PGERR | FLASH_SR_WRPRTERR)){//仍不能清除时
    //Flash_ErrCount++;
  }
}

/*/------------------------四字数据写入到Flash指定地址函数------------------
static void _WriteWord(unsigned long Adr,    //Flash地址
                        unsigned long data) //要写入的数据
{
  //开始
  FLASH->CR |= 1 << 0;              //编程使能
  *(unsigned long*)Adr = data;     //写入数据
  _WaitDone(10000);                 //等待操作完成
  FLASH->CR &= ~(1 << 0);           //无条件清除编程使能位.
}*/

//------------------------半字数据写入到Flash指定地址函数------------------
static void _WriteHalfWord(unsigned long Adr,    //Flash地址
                           unsigned short data) //要写入的数据
{
  //开始
  FLASH->CR |= 1 << 0;              //编程使能
  *(unsigned short*)Adr = data;     //写入数据
  _WaitDone(10000);                 //等待操作完成
  FLASH->CR &= ~(1 << 0);           //无条件清除编程使能位.
} 

/***************************************************************************
                        标准接口函数实现
****************************************************************************/

//------------------------初始化函数------------------------------
void Flash_Init(void)
{
}


//---------------------------Flash解锁实现---------------------------------
void Flash_Unlock(void)
{
  FLASH->KEYR = 0X45670123;
  FLASH->KEYR = 0XCDEF89AB;
}

//---------------------------Flash加锁实现---------------------------------
void Flash_Lock(void)
{
  FLASH->CR |= 1<<7;
}

//---------------------------------Flash页擦除函数实现-----------------------
//此函数仅负责页擦除，不负责加解锁等
//统一返回值定义: 0完成 1忙 2编程错误 3写保护错误
void Flash_ErasePage(unsigned long Adr)
{
  //正常结束时->仅用于判断状态
  FLASH->CR |= 1 << 1;         //置页擦除标志
  FLASH->AR = Adr;             //设置页地址 
  FLASH->CR |= 1 << 6;         //开始擦除
  _WaitDone(21000);   //等待操作结束,>20ms就行  
  FLASH->CR &= ~(1 << 1);      //无条件清除页擦除标志
}

//-------------------------写数据到Flash中实现----------------------------
//此函数仅负责向Flash写数据，不负责加解锁及擦除
void Flash_Write(unsigned long Adr,   //Flash地址
                 const void *pVoid,  //要写入的数据
                 unsigned long Len)   //写数据长度  
{
  const unsigned char *p = pVoid;             
  //for(unsigned long i = 0; i < Len; i += 4)//一次4个
  //  _WriteWord(Adr + i, *((unsigned long*)(p + i)));
  for(unsigned long i = 0; i < Len; i += 2)//一次2个
    _WriteHalfWord(Adr + i, *((unsigned short*)(p + i)));  
}

//----------------------------从Flash中读取数据实现----------------------
//此函数为可选功能,可不实现
void Flash_Read(unsigned long Adr,   //Flash地址
                void *pVoid,        //要读出的数据
                unsigned long Len)   //读数据长度
{
  memcpy(pVoid, (unsigned char*)Adr, Len);
}




