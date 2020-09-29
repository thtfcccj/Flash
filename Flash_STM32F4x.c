/***************************************************************************

                     Flash标准接口-在STM32F4X中的实现
假定为外部2.7~3.6V供电，160MHZ时工作状态
****************************************************************************/

#include "Flash.h"
#include "Delay.h"
#include "stm32f4xx.h"
#include "string.h" //memcpy
#include "IoCtrl.h" //WDT_Week()

unsigned long Flash_ErrCount = 0; //失败计数器

/***************************************************************************
                           相关配置
****************************************************************************/

#ifndef FLASH_VOLTAGE_RANGE
#define FLASH_VOLTAGE_RANGE  2   //定义电压范围: 0:<2.1V   1:< 2.1~2.7V:
//                                             2:2.7~3.6V   3:外部单独Vpp2.7~3.6V供电
#endif // FLASH_VOLTAGE_RANGE

/***************************************************************************
                        内部函数实现
****************************************************************************/
#define _ERR_MASK (FLASH_SR_WRPERR | FLASH_SR_WRPERR |  FLASH_SR_PGPERR | FLASH_SR_PGSERR)

//---------------------------Flash解锁实现---------------------------------
static void _Unlock(void)
{
  FLASH->KEYR = 0X45670123;
  FLASH->KEYR = 0XCDEF89AB;
}

//---------------------------Flash加锁实现---------------------------------
static void _Lock(void)
{
  FLASH->CR |= FLASH_CR_LOCK;
}

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
  //喂狗
  WDT_Week();
  
  if(!(State  & _ERR_MASK)) return;
  
  //异常时，清所所有挂起标志位防止出错  
  Flash_ErrCount++;
  FLASH->SR |= _ERR_MASK;//置位以清除
  if(FLASH->SR &  _ERR_MASK){//仍不能清除时
    //Flash_ErrCount++;
  }
}
//------------------------双字(8Byte)数据写入到Flash指定地址函数------------------
#if FLASH_VOLTAGE_RANGE == 3
static void _Write2Word(unsigned long long *pAdr,    //Flash地址
                         unsigned long long data)     //要写入的数据
{
  _Unlock();//解锁
  //开始
  FLASH->CR |= FLASH_CR_PSIZE;      //64bit写入模式
  FLASH->CR |= FLASH_CR_PG;        //编程使能
  *pAdr = data;                    //写入数据
  _WaitDone(10000);                //等待操作完成
  FLASH->CR &= ~FLASH_CR_PG;      //无条件清除编程使能位.
  
  _Lock();//上锁
}
#endif

//------------------------单字(32Byte)数据写入到Flash指定地址函数----------------
#if FLASH_VOLTAGE_RANGE == 2
static void _WriteWord(unsigned long *pAdr,    //Flash地址
                         unsigned long data)     //要写入的数据
{
  _Unlock();//解锁
  //开始
  FLASH->CR &= ~FLASH_CR_PSIZE_0;  //32bit写入模式
  FLASH->CR |= FLASH_CR_PSIZE_1;
  FLASH->CR |= FLASH_CR_PG;        //编程使能
  *pAdr = data;                    //写入数据
  _WaitDone(10000);                //等待操作完成
  FLASH->CR &= ~FLASH_CR_PG;      //无条件清除编程使能位.
  
  _Lock();//上锁
}
#endif

//------------------------双字节数据写入到Flash指定地址函数------------------
#if FLASH_VOLTAGE_RANGE == 1
static void _Write2Byte(unsigned short *pAdr,    //Flash地址
                        unsigned short data) //要写入的数据
{
  _Unlock();//解锁
  //开始
  FLASH->CR &= ~FLASH_CR_PSIZE_1;      //16bit写入模式
  FLASH->CR |= FLASH_CR_PSIZE_0;
  FLASH->CR |= FLASH_CR_PG;          //编程使能
  *pAdr = data;                     //写入数据
  _WaitDone(10000);                 //等待操作完成
  FLASH->CR &= ~FLASH_CR_PG;        //无条件清除编程使能位.
  
  _Lock();//上锁
}
#endif

//------------------------字节数据写入到Flash指定地址函数------------------
static void _WriteByte(unsigned char *pAdr,    //Flash地址
                        unsigned char data) //要写入的数据
{
   _Unlock();//解锁
  //开始
  FLASH->CR &= ~FLASH_CR_PSIZE;      //8bit写入模式
  FLASH->CR |= FLASH_CR_PG;          //编程使能
  *pAdr = data;                     //写入数据
  _WaitDone(10000);                 //等待操作完成
  FLASH->CR &= ~FLASH_CR_PG;        //无条件清除编程使能位.
  
  _Lock();//上锁
} 

//------------------------由基址到SNB转换-----------------------
//返回SNB值(移位FLASH_CR_SNB_SHIFT后)
unsigned long _Adr2SNB(unsigned long Adr)
{
  #ifdef FLASH_1M_BANK_S//1M单BANK时
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
                        标准接口函数实现
****************************************************************************/

//------------------------初始化函数------------------------------
void Flash_Init(void)
{
  _Lock();//防止异常重启未加锁死掉
}

//---------------------------------Flash页擦除函数实现-----------------------
//此函数仅负责页擦除，不负责加解锁等
//统一返回值定义: 0完成 1忙 2编程错误 3写保护错误
void Flash_ErasePage(unsigned long Adr)
{
  Flash_cbErasePageStartNotify(); //擦除前通报
  _Unlock();//解锁
  
  FLASH->CR &= ~FLASH_CR_PSIZE;  //强制用最低电压档8Byte模式擦除掉
  FLASH->CR |= FLASH_CR_SER;  //页擦除模式
  FLASH->CR &= ~FLASH_CR_SNB; //清除位置
  FLASH->CR |= _Adr2SNB(Adr); //指定位置
  
  //擦除期间要执行代码时，在放在RAM中执行
  #ifndef SUPPORT_FLASH_ERASE_IN_RAM
    //开始擦除(实际在擦除FLASH的期间，读取（取指）FLASH，会被暂停)
    FLASH->CR |= FLASH_CR_STRT; 
    //分时以喂狗(
    for(unsigned char i = 210; i > 0; i--){
      _WaitDone(1000);   //等待操作结束
    }
  #else
    Flash_cbEraseInRam(); //放在RAM中执行的代码
  #endif
  FLASH->CR &= ~(1 << 1);      //无条件清除页擦除标志
  _Lock();//上锁
}

//-------------------------写数据到Flash中实现----------------------------
//此函数仅负责向Flash写数据，不负责加解锁及擦除
void Flash_Write(unsigned long Adr,   //Flash地址
                 const void *pVoid,  //要写入的数据
                 unsigned long Len)   //写数据长度  
{
  unsigned char *pOrgData = (unsigned char*)pVoid;
  unsigned char *pWrAdr = (unsigned char*)Adr;
  
  //写入地址没双字对齐时，先一字一字写入以对齐  
  if(Adr & 0x07){
    unsigned char CurLen;
    if(Len > 7) CurLen = 7;
    else CurLen = Len;
    Len -= CurLen;
    for(; CurLen > 0; CurLen--, pOrgData++, pWrAdr++){
      _WriteByte(pWrAdr, *pOrgData);
    }
    if(Len == 0){//写完了
      return;
    }
  }
  
  //外部VPP时，一次写最多8Byte
  #if FLASH_VOLTAGE_RANGE == 3
  //这里保证写入地址双字对齐了(但不能确保用户数据对齐，故只能memcpy)
  unsigned long long *pllWrAdr = (unsigned long long *)pWrAdr;
  for(; Len >= 8; Len -= 8, pOrgData += 8, pllWrAdr++){
    unsigned long long llData;
    memcpy(&llData, pOrgData, 8);
    _Write2Word(pllWrAdr, llData);
    Flash_cbWrIdieNotify(); //写空闲通报
  }
  pWrAdr = (unsigned char*)pllWrAdr;
  #endif
  
  //内部2.7~3.6V时，一次写最多4Byte
  #if FLASH_VOLTAGE_RANGE == 2
  //这里保证写入地址双字对齐了(但不能确保用户数据对齐，故只能memcpy)
  unsigned long *plWrAdr = (unsigned long *)pWrAdr;
  for(; Len >= 4; Len -= 4, pOrgData += 4, plWrAdr++){
    unsigned long lData;
    memcpy(&lData, pOrgData, 4);
    _WriteWord(plWrAdr, lData);
    Flash_cbWrIdieNotify(); //写空闲通报
  }
  pWrAdr = (unsigned char*)plWrAdr;
  #endif
  
  //内部2.1~2.7V时，一次写最多2Byte
  #if FLASH_VOLTAGE_RANGE == 1
  //这里保证写入地址双字对齐了(但不能确保用户数据对齐，故只能memcpy)
  unsigned short *psWrAdr = (unsigned short *)pWrAdr;
  for(; Len >= 2; Len -= 2, pOrgData += 2, psWrAdr++){
    unsigned short sData;
    memcpy(&sData, pOrgData, 4);
    _Write2Byte(psWrAdr, sData);
    Flash_cbWrIdieNotify(); //写空闲通报
  }
  pWrAdr = (unsigned char*)psWrAdr;
  #endif
  
  //内部2.1~以下时时，一次写最多1Byte,继续
  #if FLASH_VOLTAGE_RANGE == 0
  #endif
  
  //余下部分一字一字写入以对齐
  for(; Len > 0; Len--, pOrgData++, pWrAdr++){
    _WriteByte(pWrAdr, *pOrgData);
  }
}

//----------------------------从Flash中读取数据实现----------------------
//此函数为可选功能,可不实现
void Flash_Read(unsigned long Adr,   //Flash地址
                void *pVoid,        //要读出的数据
                unsigned long Len)   //读数据长度
{
  memcpy(pVoid, (unsigned char*)Adr, Len);
}




