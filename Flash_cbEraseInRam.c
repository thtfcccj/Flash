/***************************************************************************

                     Flash 在擦除期间要执行的代码示例 
这里以IAR环境为例, 应用代码已测试成功
****************************************************************************/
#ifdef SUPPORT_FLASH_ERASE_IN_RAM1 //这里去掉后缀1后，可直接修改使用

#include "Flash.h"
#include "stm32f4xx.h"

//用户代码包含
#include "IoCtrl.h"

//----------------------在擦除期间要执行的代码-----------------------------
//__ramfunc 表示代码在ram中执行,  为减少RAM占用，代码应尽量精简
//此函数不要调用外部函数，要调用也要将被调用函数声明为 __ramfunc 
__ramfunc static void _InRam(void)
{
  //这里局部变量声明与相关初始化,以周期扫描数码管执行为例
  
  unsigned short Period; //周期计数(不置数以节省代码)
  unsigned char ScanId; //扫描位(不置数以节省代码)
  
  //开始擦除
  FLASH->CR |= FLASH_CR_STRT;
  do{
    if(Period) Period--;//周期未到
    else{
      Period = 10000;//这里重置总计时周期    
      ScanId++;
      if(ScanId > 4) ScanId = 0;//一周期了
      //这里切换扫描数码管(应用代码略)
      
    }//end else
  }while(FLASH->SR & FLASH_SR_BSY);//忙时继续
}

//------------------------回调函数实现-----------------------------
//应尽量多地将不需要在RAM中执行的代码放在此
void Flash_cbEraseInRam(void)
{
  //这前期预处理,
  ;  //如：数码管并口先置为输出
  
  __disable_irq();   // 关闭总中断
  _InRam();      
  __enable_irq();    // 开启总中断
  
  
  //这里后期恢复
  
}



#endif //SUPPORT_FLASH_ERASE_IN_RAM1

