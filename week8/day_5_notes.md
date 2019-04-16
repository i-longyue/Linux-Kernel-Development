
其中usb目录包含了所有USB设备的驱动，而usb目录下面又有它自己的子目录，
进去看看：



2.
3.变态的模块机制
4.想到达明天现在就要启程

5.外面的世界很精彩
	很不幸，作为U盘,需要USB Core, SCSI Core,内存管理单元，还有内核中许多其他模块
	打交道。

	USB通信的灵魂就是USB协议。USB协议将所有USB设备和USB主机所必须遵循游戏规则。
	
6.未曾开始却似结束
	
	那就只有一个函数调用了usb_register.这个函数有什么用。首先是注册，从而让USB
	Core知道有这个设备。


#7.狂欢是一群人的孤单
	Linux设备模型中三个很重要的概念就是总线，设备和驱动，即bus、device和driver
	而实际内核中也定义了这么一些数据结构，它们是
	struct bus_type
	struct device 
	struct device_driver
	这三个重要的数据结构都来自同一个地方，即include/linux/device.h 

	我们知道的总线有很多种，如PCI总线，SCSI总线，USB总线，所以我们会看到Linux
	内核代码中出现
	pci_bus_type
	scsi_bus_type
	usb_bus_type
	它们都是struct bus_type类型的变量，而struct bus_type结构中非常重要的成员就
	是struct kset drivers和struct set devices.kset和另一个叫做kobject正是2.6
	内核中设备模型的基本元素。


	这里我们只需要知道，driver和devices的存在，让struct bus_type与两个链表联系了
	起来:分别是devices和drivers的链表。知道一条总线所对应的数据结构，就可以找到
	这条总线所关联的设备，及支持这类设备的驱动程序。

	而要实现这些目的，就要求每次出现一个设备就要向总线汇报，或者说注册。每次出现
	一个驱动也要向总线汇报。比如系统初始化时，会扫描连接了那些设备。
	并为每一个设备建立起一个struct device的变量，每一次有一个驱动程序，就要准备
	一个struct device_driver结构的变量。把这些变量统统加入相应的链表，设备加入
	devices链表，驱动插入drivers链表。这样通过总线就能找到每一个设备，每一个驱动。



#8.总线、设备和驱动(上)
	struct bus_type中为设备和驱动准备了两个链表，

	而代表设备的结构体struct device
	中又有两个成员，
	struct bus_type *bus 
	struct device_driver *driver


	面代表驱动的结构体struct device_driver
	中也有两个成员 
	struct bus_type *bus
	struct list_head devices

	struct device和struct device_driver的定义和struct bus_type一样

	struct device中bus记录的是这个设备连成哪条总线上，
	driver记录的是这个设备用的是哪个驱动。


	反过来，struct device_driver中bus代表的也是这个驱动属于哪条总线
	devices记录的是这个驱动支持哪些设备。没错，是devices，而不是device
	因为一个驱动程序可以支持一个或多个设备。

	于是我们想知道，关于bus，关于device，关于driver,它们是如何建立联系的。
	换而言之，这三个数据结构中的指针是如何被赋值的？绝对不可能发生的事情是
	一旦为一条总线申请了一个东西。这些东西一定不会是先天就有的，只能是后来
	填进来的。
		而具体到USB系统，完成这个工作就是USB Core。USB Core的代码会进行整个USB
		系统的初始化 ，比如申请 bus_type usb_bus_type,然后会扫描usb总线，看线
	上连接了哪些USB设备。或者Root Hub上连了哪些USB设备，比如连接了一个USB键盘
	那么就为它准备一个struct device 根据它的实际情况，为这个struct device赋值
	并插入devices链表中来，又比如Root Hub上连了一个普通的Hub，那么除了要这个
	Hub本身准备一个struct device以外，还得继续扫描看这个Hub是否又连了别的设备
	，最终usb_bus_type中的devices链表给建立了起来

	drivers链表。每一个USB设备的驱动程序都会struct usb_driver结构
	


#9.总线，设备和驱动(下)
	每当一个struct device诞生，它就会去bus的drivers链表中寻找自己的另一半，
	反之每当一个个struct device_driver，它就会去bus deivces链表中寻找它的
	那些设备。如果找到了合适的那么和之前那种情况一样。调用device_bind_driver
	绑定好.
	

	可以看到这定义了一个struct usb_driver的结构体变量，usb_storage_driver
	关于usb_driver我们上节说过了，当时主要说的是其中的成员driver.
	

#10.我是谁



