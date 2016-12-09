# [cdiNodeSlave](https://github.com/ArmstrongYang/cdiNodeSlave)
The project of the CDI slave sensor module.

[toc]

## [SlaveBoot](https://github.com/ArmstrongYang/cdiNodeSlave/tree/master/SlaveBoot)
### 路径
/SlaveBoot/MDK-ARM/Platforms.uvprojx
### 功能
1. 该程序是整个程序的起始运行文件，需要在以后的每个模块中下载该程序。
2. 该程序从0x8000000开始，占用空间0x1000bytes，也即4K大小；
3. 其他应用程序从0x8001000开始，占用空间0x3000bytes，也即12K大小；
4. 如果有程序超过规定大小，Keil会提示编译不通过，因此不必担心。
5. SlaveBoot储存空间分配方案图：
![SlaveBoot](https://github.com/ArmstrongYang/cdiNodeSlave/blob/master/2016-12-09-SlaveBoot%E5%AD%98%E5%82%A8%E5%92%8CRAM%E5%88%86%E9%85%8D%E5%9B%BE.png =100x100)

## SlaveApp （Demo文件，仅用于测试，详情参考SlaveModules）
### 路径
/SlaveApp/MDK-ARM/Platforms.uvprojx
### 功能
1. 该程序是整个程序的I2C通信和指令执行文件。
2. 模块启动后，Boot程序会自动执行跳转命令；
3. 该程序从0x8001000开始，占用空间0x3000bytes，也即12K大小；
4. 如果有程序超过规定大小，Keil会提示编译不通过，因此不必担心。
5. SlaveApp储存空间分配方案图：
![SlaveApp](https://github.com/ArmstrongYang/cdiNodeSlave/blob/master/2016-12-09-SlaveApp%E5%AD%98%E5%82%A8%E5%92%8CRAM%E5%88%86%E9%85%8D%E5%9B%BE.png =100x100)

## SlaveModules(每个模块程序)
### 模块列表
#### 0x10-sRGB
三色RGB模块
#### 0x11-sKey
按键模块

## SlaveLibraries
### 功能
1. 存放每个模块对应的单独.c 和.h文件
2. 便于模块文件的集中管理，除了此处每个模块不一样，其他部分基本上一致

## 2016-11-29 16:51:43
+ 增加SlaveBoot程序框架
+ 增加SlaveApp程序框架
	+ 必须要加上NVIC_SRAM()这个程序，意义是：更改中断向量表

## 2016-12-08 18:44:28 
+ Master+APP成功运行，证据：2016-12-08 Master+APP成功运行.txt

## 2016-12-09 00:50:59
+ SlaveBoot配合模块APP使用即可
