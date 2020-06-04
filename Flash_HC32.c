/***************************************************************************

                     Flash标准接口-在Hc32中的实现
对 FLASH进行页擦除的代码所在地址必须小于 32768
****************************************************************************/

#include "Flash.h"
#include "Delay.h"
#include "HC32.h"
#include "string.h" //memcpy
unsigned long Flash_ErrCount = 0; //失败计数器


/***************************************************************************
                              配置说明
不可在此修改以免影响一致性！
****************************************************************************/

//#define SUPPORT_MDK   //Keil MDK环境时，需定义

//注：SYS_MHZ为全局定义
#define _DELAY_MUTI        ((SYS_MHZ + 3) / 4)   //4M为基准插入延时
#define FLASH              M0P_FLASH       //别名

#ifndef FLASH_BASE //FALSH基址
  #define FLASH_BASE   0x00000000 //默认为0
#endif

#ifndef FLASH_SLOCK_SIZE //每个SLOCK保护的范围
  #ifdef FLASH_COUNT
    #define FLASH_SLOCK_SIZE   （FLASH_COUNT / 32） //分为32份
  #else
    #define FLASH_SLOCK_SIZE   2048    //默认2K一份
  #endif
#endif

/***************************************************************************
                             内部函数实现
****************************************************************************/

//-----------------------------写保护寄存器函数----------------------------
//只有指定预列才能写FLASH结构寄存器里需调用此函数
#ifdef SUPPORT_MDK    //需限制此函数在32k内, Keil MDK环境定义
  void _WrProtReg(__IO uint32_t *pReg,
                  unsigned long Data) __attribute__((section(".ARM.__at_0x400")));
  static void _WrProtReg(__IO uint32_t *pReg,        //待写入的受保护寄存器
                          unsigned long Data)
#else // IAR环境定义, Flash容量<32k时可不在icf文件中指定
  static void _WrProtReg(__IO uint32_t *pReg,        //待写入的受保护寄存器
                          unsigned long Data)@".Flash_WrProtReg" //IAR环境定义
#endif
{
  unsigned short Count = 255 * _DELAY_MUTI; //例程中的值
  for(; Count > 0; Count--){
    FLASH->BYPASS = 0x5A5A;
    FLASH->BYPASS = 0xA5A5;
    *pReg = Data;
    if(*pReg == Data) return; //写成功了
  }
  Flash_ErrCount++; //写错误计数
}

//-----------------------------Flash写操作完成等待函数--------------------------
//返回值同Flash_ErasePage()
static void _WaitDone(unsigned short Time)//IAR环境定义
{
  for(; Time > 0; Time--){
    if(!(M0P_FLASH->CR_f.BUSY)) return; //非忙时写完了
    DelayUs(1);
  }
  Flash_ErrCount++; //写错误计数
}

//------------------------------寄存器默认值------------------------------------
#if (_DELAY_MUTI > 1)
  static const unsigned long _DelayRegDefault[] ={
    0x20,      //Tnvs 时间参数
    0x17,      //Tpgs 时间参数
    0x1b,      //Tprog 时间参数
    0x4650,    //Tserase 时间参数
    0x222E0,  //Tmerase 时间参数
    0x18,     //Tprcv 时间参数
    0xF0,     //Tsrcv 时间参数
    0x03E8,   //Tmrcv时间参数
  };

#endif //_DELAY_MUTI


/***************************************************************************
                        标准接口函数实现
****************************************************************************/

//------------------------------初始化函数------------------------------------
void Flash_Init(void)
{
  //修改默认定时值与系统相同
  //根据芯片资料，默认值是以4M为基础，否则应根据系统进钟成倍调整
  #if (_DELAY_MUTI > 1)
    __IO uint32_t *pDelayReg = &FLASH->TNVS; //定时值起始，共8个
    for(unsigned char i = 0; i < 8; i++, pDelayReg++)
      _WrProtReg(pDelayReg, _DelayRegDefault[i] * _DELAY_MUTI);
  #endif
  
  //重配置读等待周期
  //根据芯片资料,超24M时每起过24M多一值
  #if ((_DELAY_MUTI / 6) > 1)
    unsigned long Data = FLASH->CR & ~0x0C;
      _WrProtReg(&FLASH->CR, Data | ((_DELAY_MUTI / 6) << 2));
  #endif
}

//---------------------------Flash解锁实现---------------------------------
void Flash_Unlock(void)
{
  //这里实现为空
}

//---------------------------Flash加锁实现---------------------------------
void Flash_Lock(void)
{
  //这里实现为空  
}

//---------------------------------Flash页擦除函数实现-----------------------
//此函数仅负责页擦除，不负责加解锁等
//统一返回值定义: 0完成 1忙 2编程错误 3写保护错误
#ifdef SUPPORT_MDK    //需限制此函数在32k内, Keil MDK环境定义
  void Flash_InErasePage(unsigned long Adr) __attribute__((section(".ARM.__at_0x500")));
  void Flash_ErasePage(unsigned long Adr) {Flash_InErasePage(Adr); }
  void Flash_InErasePage(unsigned long Adr)
#else // IAR环境定义, Flash容量<32k时可不在icf文件中指定
void Flash_ErasePage(unsigned long Adr)@".Flash_ErasePage"
#endif
{
  __disable_irq();
  _WrProtReg(&FLASH->CR, FLASH->CR | 2);  //进入页擦除模式
  _WrProtReg(&FLASH->SLOCK, 1 << ((Adr - FLASH_BASE) / FLASH_SLOCK_SIZE));//取消页保护
  *(volatile unsigned char*)Adr = 0xff; //写入任意数据启动
  _WaitDone(21000);   //等待操作结束,>20ms为经验值
  _WrProtReg(&FLASH->CR, FLASH->CR & ~0x03);  //退出页擦除模式
  _WrProtReg(&FLASH->SLOCK, 0);//加上页保护
  __enable_irq();
}

//-------------------------写数据到Flash中实现----------------------------
//此函数仅负责向Flash写数据，不负责加解锁及擦除
#ifdef SUPPORT_MDK    //需限制此函数在32k内, Keil MDK环境定义
  void Flash_InWrite(unsigned long Adr,const void *pVoid,unsigned long Len) \
                                    __attribute__((section(".ARM.__at_0x600")));
  void Flash_Write(unsigned long Adr,const void *pVoid,unsigned long Len)
  {Flash_InWrite(Adr, pVoid, Len); }
  void Flash_InWrite(unsigned long Adr,const void *pVoid,unsigned long Len)
#else // IAR环境定义, Flash容量<32k时可不在icf文件中指定
void Flash_Write(unsigned long Adr,   //Flash地址
                 const void *pVoid,  //要写入的数据
                 unsigned long Len)@".Flash_Write" //IAR环境定义
#endif
{
  //写前准备：
  __disable_irq();
  _WrProtReg(&FLASH->CR, FLASH->CR | 1);  //进入写模式
  _WrProtReg(&FLASH->SLOCK, 1 << ((Adr - FLASH_BASE) / FLASH_SLOCK_SIZE));//取消页保护
  
  const unsigned char *pData = (const unsigned char *)pVoid;
  //写入地址没4字对齐时，先一字一字写入以对齐
  for( ; (Adr & 0x03) && (Len > 0); Adr++, Len--, pData++){
    *(volatile unsigned char*)Adr = *pData;     //写入数据
    _WaitDone(10000);                 //等待操作完成,10ms为经验值
  }
  if(Len == 0) goto _EndWrPro;//写完了  
  
  //中间4字节对齐写入
  for( ; Len > 3; Len -= 4, Adr += 4, pData+= 4){  
    *(volatile unsigned long*)Adr = *(const unsigned long*)pData; //写入数据
    _WaitDone(10000);                 //等待操作完成,10ms为经验值
  }
  if(Len == 0) goto _EndWrPro;//写完了  
  
  //最后未4字节对齐地址
  for( ;Len > 0; Adr++, Len--, pData++){
    *(volatile unsigned char*)Adr = *pData;     //写入数据
    _WaitDone(10000);                 //等待操作完成,10ms为经验值
  }
  
  _EndWrPro: //结束写处理
    _WrProtReg(&FLASH->CR, FLASH->CR & ~0x03);  //取消写模式
    _WrProtReg(&FLASH->SLOCK, 0);//加上页保护
    __enable_irq();
}

//----------------------------从Flash中读取数据实现----------------------
//此函数为可选功能,可不实现
void Flash_Read(unsigned long Adr,   //Flash地址
                void *pVoid,        //要读出的数据
                unsigned long Len)   //读数据长度
{
  memcpy(pVoid, (unsigned char*)Adr, Len);
}




