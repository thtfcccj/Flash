﻿##嵌入式系统通用驱动程序接口及其实现-Flash操作及存取位置

* 此接口用于具体项目中: 对Flash操作时，提供统一的存取操作函数，实现了调用层与的Flash硬件/存储空间的分离。
* 相关软件模块需要Flash存储数据时，统一通过包含"Flash.h"文件,实现操作的标准化；

####软件结构说明:
  + **Flash通用操作接口:**  即对外接口调用文件:**Flash.h** ，其它软件部分操作Flash时,**包含此文件即可**。
  + **Flash操作接口的各种实现:** 以*Flash(下横线_)Flash硬件名称(或载体)_专用编译环境(可选,严重不建议代码与编译环境相关连)*命名, 与具体使用的Flash硬件有关，即有各种不同的实现。但一种硬件仅实现一次，项目中需要那个加入那个即可，具有通用性(*实现时需尽量排除编译器影响，使一个芯片的实现能在各种编译器里运行*)。

####使用说明：
 + 1.根据项目嵌入式硬件不同，将Flash.h和**与项目对应的**的Flash具体实现文件。增加到开发环境中。

####目录结构组织：
* **小型项目时**: 即不区分组件层，放在“项目源文件目录\Flash”下，内部不再有子目录
* **大中型项目时**: 区分组件层，放在“项目源文件目录\components\Flash”下，若项目很多，且同一项目也有较多实现时，可将具体实现文件放在此目录“项目名称”目录下，以实现分类存放。

####现支持的Flash：
  *  STM32F4x系列
  *  STM32F10x系列


-------------------------------------------------------------------------------

##开源项目说明
* 为各类单片机提供模板支持,**欢迎大家增加对各类嵌入式硬件的操作模板**,以让更多人使用
* 版权声明: ...ch这世道，说了也等于白说，总之以下点：
 + **源代码部分：** 可以自由使用，源代码中，也不需做任何版权声明。
 + **分享时：** 为防止碎化片，请注明出处，以利于开源项目的推广。
 + **关于fork：**  这个欢迎(但为防止碎化片化，请不要分支或单独推广)。更欢迎为此开源项目直接贡献代码。 

##此开源项目对应的教程
* 视频在分享平台：http://thtfcccj.56.com
* 与视频同步输入的文字,在http://blog.csdn.net/thtfcccj
* 同步的开源项目，则在代码托管平台：https://github.com/thtfcccj

##此开源项目对应的两个件教学视频：
* 嵌入式系统通用驱动程序接口及其实现2-Flash操作标准化
* 嵌入式系统通用驱动程序接口及其实现n-Flash存取位置标准化










