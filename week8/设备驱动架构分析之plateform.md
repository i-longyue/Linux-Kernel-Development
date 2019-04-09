Linux设备驱动架构分析之platform
1.什么是platform(平台)总线？
相对于USB，PCI，i2C,SPI等物理总线来说，platform总线是一种虚拟，抽象出来的总线
实际中并不存在这样的总线。

那为什么需要platform总线呢？其实是Linux设备驱动模型为了保持设备驱动的统一性而
虚拟出来的总线。



2.platform总线线管理的2员大将
(1)两个结构结platform_device和platform_driver
对于任何一种Linux设备驱动模型下的总线由都两个部分组成：描述设备相关的结构体和
描述驱动相关的结构体

在platform总线下就是platform_device和platform_driver,下面是对两个结构体的各个
元素进行分析：

platform_device()
	struct platform_device {
		conest
	
