
##这个来自于linux设备驱动第3版

USB主控制器负责询问每个USB设备是否它有数据发送。由于这个拓扑关系，一个
USB设备在没有首先被主控制器询问时从不启动发送数据，这个配置允许一个非常
容易即插即用的系统，这样各种设备可自动被主机配置。


13.2 USB和sysfs
由于单个USB物理设备的复杂性，设备在sysfs中表示也非常复杂，物理USB设备
(通过struct usb_device表示)和单个USB接口(由struct usb_interface表示)
都作为单个设备出现在sysfs。

对于一个简单的只包含一个USB接口的USB鼠标，下面的内容给这个设备的sysfs
目录树:


/sys/devices/pci0000:00/0000:00:09.0/usb2/2-1
|-- 2-1:1.0
| |-- bAlternateSetting
| |-- bInterfaceClass
| |-- bInterfaceNumber
| |-- bInterfaceProtocol
| |-- bInterfaceSubClass
| |-- bNumEndpoints
| |-- detach_state
| |-- iInterface
| |-- power
| |-- state
|-- bConfigurationValue
|-- bDeviceClass
|-- bDeviceProtocol
|-- bDeviceSubClass
|-- bMaxPower
|-- bNumConfigurations
|-- bNumInterfaces
|-- bcdDevice
|-- bmAttributes
|-- detach_state
|-- devnum
|-- idProduct
|-- idVendor
|-- maxchild
|-- power
||-- state
|-- speed
|-- version


结构usb_device在树中被表示在：
/sys/devices/pci0000:00/0000:00:09.0/usb2/2-1


而鼠标的usb接口-usb鼠标设备驱动被绑定到接口-位于目录：
/sys/devices/pci0000:00/0000:00:09.0/usb2/2-1/2-1:1.0

为了帮助理解这个长设备路径的含义，我们描述内核如何标记USB设备。
第一个USB设备是一根集线器，这是USB控制.常常包含在一个PCI设备中，
控制器的命名是由于它控制整个连接到它上面的USb总线。
控制器是一个PCI总线和USB总线之间的桥，同时是总线上的第一个设备

所有根集线器被USb核心安排一个唯一的号，在我们的例子中。根集线器
称为usb2，因为它是USb核心的第2个根集线器，可包含在单个系统中在
任何时间的根集线器的数目没有限制。

每个在USB总线上的设备采用根集线器的号作为它的句子的第一个数字。
紧跟着的是-字符和设备插入的端口号。

由于我们例子中的设备在第一个端口,一个1被添加到名字。因为给主USB鼠标
设备的名字是2-1.因为这个USB设备包含一个接口，那使得树中的另一个设备
被添加到sysfs路径。设备名是2-1：1.0是因为它是第一个配置并且有接口号0.

总结，USB sysfs设备命名方法是：
root_hub-hub_port:config.interface

对应一个2层的树，设备名看来象：
root_hub-hub_port-hub_port:config:interface

sysfs没暴露一个usb设备的所有不同部分，因为它停止在接口水平。
具体的细节可以在usbfs文件系统中找到。它加载在系统的/proc/bus/usb/目录
/proc/bus/usb/devices


13.3 USB的Urbs
linux内核中的USB代码和所有的USB设备通讯使用称为urb的东西。

一个urb用来发送或接受数据到一个特定的USb设备上的特定USb端点，
以一种异步的方式。
它用起来非常象一个kiocb结构被用在文件系统 异步I/O代码，或者如同一个
struct skbuff用在网络代码中。一个usb设备驱动可能分配许多urb给一个端点
或者如同一个struct skbuff用在网络代码中，

????
*被一个USb设备驱动创建
*安排一个特定USB设备的特定端点
*提交给USb核心，被usb设备驱动
*提交给特定设备的被USB核心指定的USb主机控制器驱动 
*被USb主机控制器处理，它做一个USB传送到设备
*当urb完成 USB主机控制器驱动通知USb设备驱动 


13.3.1 结构struct urb

	struct usb_devce *dev
	unsigned int pipe;
	unsigned int usb_sndctrlpipe
	unsigned int usb_rcvctrlpipe(struct usb_device *dev, unsigned int endpoint)

	unsigned int usb_sndbulkpipe(struct usb_device *dev, unsigned int endpoint)

	unsigned int usb_rcvbulkpipe(struct usb_device *dev, unsigned int endpoint)

	unsigned int usb_sndintpipe(struct usb_device *dev, unsigned int endpoint)

	unsigned int usb_rcvintpipe(struct usb_device *dev, unsigned int endpoint)

	unsigned int usb_sndisocpipe(struct usb_device *dev, unsigned int endpoint)

	unsigned int usb_rcvisocpipe(struct usb_device *dev, unsigned int endpoint)

	unsigned int transfer_flags

	URB_SHORT_NOT_OK

	URB_ISO_ASAP

	URB_NO_TRANSFER_DMA_MAP

	URB_NO_SETUP_DMA_MAP

	URB_ASYNC_UNLINK

	URB_NO_FSBR

	URB_ZERO_PACKET

	URB_NO_INTERRUPT



###13.3.2 创建和销毁urb
	13.3.2.1 中断urb
	13.3.2.2 控制urb
	13.3.2.3 同步urb


###13.3.3 提交urb
###13.3.4 完成urb回调处理
	只有三个方法，一个urb可被结束并且使完成函数被调用:
	*urb 被成功发送给设备，并且设备返回正确的确认对于一个OUT urb，数据被成功发送，
	对于一个IN urb,请求的数据被成功收到。urb的状态变量被设置为0.

	*一些错误连续发生，当发送或者接受数据从设备中，被urb结构中status变量中的错误
	值所记录。

	*这个urb被从usb核心去链，这发生在要么当驱动告知USB核心取消一个已提交的urb
	通过调用usb_unlink_urb或者usb_kill_urb,要么当设备从系统中去除，以及一个urb
	已经urb被提交 

###13.3.5 取消urb
	为停止一个已经提交给USB核心的urb，函数usb_kill_urb或者usb_unlink_urb应当被调用 


#13.4 编写一个USB驱动
	编写一个USB设备驱动的方法类似于一个pci驱动：驱动注册它的驱动对象到USB子系统
	并且之后使用供应商和设备标识来告知是否它的硬件已经安装。

#13.4.1 驱动支持什么设备
#13.4.2 注册一个USB驱动
	所有的USB驱动必须是struct usb_driver这个结构必须是被USB驱动填充并且包含多个
	函数回调和变量,来向USb核心代码描述USb驱动: 


	struct module *owner 
	指向这个驱动的模块拥有者的指针。USB核心使用它正确地引用计数这个USB驱动,以便
	它不被在不合适的时刻卸载，这个变量应当设置到THISMODULE宏。


	const char *name
	指向驱动名字的指针，它必须在内核USB驱动中是唯一的并且通常被设置为和驱动模块
	名相同。它出现在const struct usb_device_id *id_table

	指向struct usb_device_id表的指针，包含这个驱动可以接受的所有不同类型USb设备
	的列表，如果这个变量没有被设置，USB驱动中探测回调函数不会被调用。如果你需要
	你的驱动给系统中每个USB设备一直被调用，创建一个只设置这个driver_info成员的
	入口项：

	static struct usb_device_id_usb_ids[] = {
		{.driver_info = 42},
		{}
	};

	int (*probe)(struct usb_interface *intf, const struct usb_device_id *id)
	指向USB驱动的去连接函数的指针，这个函数被USB核心调用，当struct_usb_interface
	已被从系统中清除或者当驱动被从USB核心卸载。

	为创建一个值struct usb_driver结构，只有5个成员需要被初始化：
	static struct usb_driver = {
		.owner 
		.name
		.probe
		.disconnect
	}


   static int __init usb_skel_init(void)
    {
    	int result;
    	/* register this driver with the USB subsystem */
    	result = usb_register(&skel_driver);
    	if (result)
    		err("usb_register failed. Error number %d", result);
    	return result;
    }



   static void __exit usb_skel_exit(void)
   {
   	/* deregister this driver with the USB subsystem */
   	usb_deregister(&skel_driver);
   }

  为注册struct usb_driver到USB核心，一个调用usb_register_driver带一个指向struct
  usb_driver的指针。传统上的USb驱动模块初始化做这个：
  当USB模块被卸载，struct usb_driver需要从内核注销。

  
  13.4.2.1 探测和去连接的细节
  在之前章节描述的struct usb_driver结构中，驱动指定2个USB核心在合适的时候调用
  函数，探测函数被调用，当设备被安装时，USB核心驱动认为这个驱动应当处理;探测
  函数应当进行检查传递给它的关于设备的信息，并且决定是否驱动真正适合的那个设备
  去连接函数被调用当驱动应当不再控制设备。

  在探测函数回调中，USB驱动应当初始化任何它可能使用来管理USB设备的本地结构。它
  还应当保存任何它需要的关于设备的信息到本地结构，因为在此时做这些通常更容易
  作为一个例子，USB驱动常常想为设备探测端点地址和缓冲大小是什么



##13.4.3 提交和控制一个urb
  当驱动有数据发送到USB驱动设备(如同在驱动的write函数中发生的),一个urb必须被
  分配来传送数据到设备。

  当驱动有数据发送到USB设备（如同在驱动write函数中发生的),一个urb必须被分配
  来传送数据到设备。

  在urb被成功分配后，一个DMA缓冲也应当被创建来发送数据到设备以最有效的方式。并且
  被传递到驱动的数据 应当被拷贝到缓冲：


  应当数据被正确地从用户空间拷贝到本地缓冲，urb在它可被提交给USB核心之前必须被
  正确初始化：
  

  现在urb被正确分配，数据被正确拷贝，并且urb被正确初始化，它可被提交给USB核心
  来传递给设备

  在urb被成功传递到USB设备，urb回调被USB核心调用，在我们的例子，我们初始化urb
  来指向函数skel_write_bulk_callback,并且那就是被调用的函数：

  回调函数做的第一件事是检查urb的状态来决定是否这个urb成功完成或没有。




























