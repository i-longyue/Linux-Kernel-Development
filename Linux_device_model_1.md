Linux设备模型:基础篇
linux提供了新的设备模型:总线(bus),设备(device),驱动driver.其中总线是处理器与设备之间通道
,在设备模型中,所有的设备都通过总线相连;设备是对一个设备的详细信息描述,驱动是设备的相关
驱动.其基本关系如下:bus相当于一个容器,是device和device_driver的管理机构,它包含了一个
device集合和一个driver集合.其中,device集合包含了挂在该总线下的所有设备,这些设备通过
链表链接起来;driver集合包含了挂在该总线下的所有驱动程序,这些驱动程序通过链表链接起来.

sysfs文件系统:
sysfs文件系统是Linux2.6内核引入的,它被看成与proc devfs和devpty等同类别的文件系统,sysfs
文件系统也是一个虚拟文件系统,它可以产生一个包括所有系统硬件的层级视图,与提供进程和
状态信息的proc文件系统系统十分类似;

sysfs文件系统把链接在系统上的所有设备和总线组织成一个分级的文件系统,它们可以由
用户空间存取,并向用户空间导出内核数据结构及它们的属性等信息sysfs的一个目的就是
展示设备驱动模型中各个组件的层次关系,其顶级目录结构包括:
	1.block
	2.devices
	3.bus
	4.drivers
	5.class

从图中可以看出:"Linux设备模型就是总线,设备驱动,类"这四个概念之前的相互关系;这
也是linux2.6内核抽象出来的用于管理系统中所有设备的模型图
简单地描述设备模型的层次关系如下:
	1.驱动核心中可以注册多种类型的总线
	2.每一种类型的总线下面可以挂载许多设备(kset, device)
	3.每一种类型的总线下面可以挂载许多设备驱动(kset, device_driver)
	4.每一个驱动程序可以管理一级设备
	这种基本关系的建立源于实际系统中各种总线,设备,驱动,类结构的抽象;
	Linux设备模型中的总线设备,驱动,类,之间环环相扣的复杂关系可以发展的需要.


	1.关于总线bus
	(1)busgg结构体
	struct bus_type,该结构体中有个函数指针成员:int (*match)(struct device *dev
			,struct device_driver *drv);这个函数指针非常重要,当一个新设备或者
	驱动被添加到一个总线时会被调用,是建立总线上设备与驱动的桥梁.

	(2)总线的操作
	注册,注销

	(3)总线属性bus_attribute

	2.关于device
	(1)在底层,一个设备对于一个设备结构体struct device
	(2)设备操作
	注册设备
	注销设备
	(3)设备属性:
	sysfs中的设备入口可有属性,相关的结构是:


	3.关于driver
	(1) driver结构体为struct device_driver
	(2) 驱动程序的操作
	驱动程序注册注销

	(3)驱动程序的属性
	struct driver_attribute  DRIVER_ATTR
	int driver_create_file
	void driver_remove_file
	


