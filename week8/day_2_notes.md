
4.1 USB初始化过程
	USB驱动作为一个系统，集成了众多的驱动模块，注册过程非常复杂。从USB系统角度
	来说，USB主机驱动主要包含：
	1）USB核驱动
	2）主机控制器驱动
	3）集线器驱动

	驱动的加载流程如下图所示:
	 
 	message.c  usb_control_msg()

#---------------------------------------------------------------------#
	 usb_get_device_descriptor()
	 usb_hcd_pool_rh_status()

	Hcd.c
	 hcd_buffer_create
	 usb_register_bus
	 usb_alloc_dev
	  
	 usb_hcd_poll_rh_status
	 usb_get_device_descriptor
	 register_root_hub

	otgip_ehci 
	 otg_ip_irq()
	 usb_add_hcd()
	 usb_create_hcd()


	platform_get_resource();
	platform_get_irq();

	otg_probe()

											usb_get_device_state()					
											usb_new_device()
											device_add()
											hub_port_init()
											usb_hub_port_init()
											usb_hub_port_change()
											hub_port_change()
											hub_port_status()
											hub_prereset()
											usb_autopm_get_interface()
											hub_event()
											kthread_run()
#---------------------------------------------------------------------#
	usb/core/usb.c
	usb_init()       usb_host_init()  usb_hub_init() usb_dev_init()

#---------------------------------------------------------------------#
4.1.1 USB Core的初始化
	USB驱动从USB子系统的初始化开始，USB子系统的初始化在文件driver/usb/core/usb.c
	1.subsys_initcall(usb_init);
	2.module_exit(usb_exit);
	subsys_initcall()是一个宏，可以理解为module_init().由于此部分代码非常重要
	开发者把它看作一个子系统,而不仅仅是一个模块。USB Core这个模块代表的不是某
	一个设备，而是所有USB设备赖以生存的模块。在Linux中，像这样一个类别的设备
	驱动被归结为一个子系统。

	subsys_initcall(usb_init)告诉我们，usb_init才是真正的初始化函数，而usb_exit
	将整个USB子系统结束时的清理函数。

4.1.2 主机控制器的初始化及驱动执行(以EHCI为例)
	module_init(otg_init);											模块注册
	static init __init otg_init(void);
	platform_driver_register();										平台注册. 
	static  init __init otg_probe(struct platform_device *pdev);    探测处理函数

	reg = platform_get_resource(pdev, IORESOURCE_MEM, 0);           获取寄存器信息
	data = platform_get_resource(pdev, IORESOURCE_MEM,1);           获取内存信息

	irq = platform_get_irq(pdev, 0);                                获取中断号
	usb_create_hcd(&otg_hc_driver, &pdej据空间进行分配，初始化计数,总线，定时器，
	HCD结构体各成员值 

	ret = usb_add_hcd(hcd, irq, SA_INTERRUPT);
	完成HCD结构体的初始化和注册，申请buffer，注册总线，分配设备端内存空间，向
	中断向量表中申请中断，注册根集线器，对根集线器状态进行轮询


4.1.3 注册集线器
	register_root_hub(hcd);
	在USB系统驱动加载过程中,创建了集线器的线程(khubd),并且一直查询相应的线程
	事务。HCD驱动中，将集线器作为一个设备添加到主机控制器驱动中，然后进行集线器
	端口的初始化。在USB主机看来，根集线器本身也是USB主机的设备。USB主机驱动加载
	完成之后，即开始注册根集线器，并且作为一个设备加载到主机驱动之中。

	USB主机和USB设备之间进行数据交互，USB设备本身并没有总线控制权，u盘被动地
	接收USB主机发送过来的信息并做响应。USB主机控制器与根集线器构成了主机系统，
	然后外接其它的USB设备。

	为了更好地探测到根集线器的状态变化，USB主机控制器驱动增加了状态轮询函数。
	以一定的时间间隔轮询根集线器状态是否发生变化。一旦根集线器状态发生变化
	主机控制器就会产生相应的响应。

	USB和USB设备之间的数据传输以URB(USB Request Block)的形式进行。

4.2 URB传输过程
	usb初始化过程中，无论是主机控制器驱动还是根集线器驱动，都是通过URB传输设备
	信息。

4.2.1  申请URB
	struct urb *usb_alloc_urb(int iso_packets, gfp_t mem_flags)
	为urb分配内存并执行初始化。

4.2.2 初始化URB
	
 static inline void_fill_bulk_urb(struct urb *urb,
		 struct usb_device *dev,
		 unsigned int pipe,
		 void *transfer_buffer,
		 int buffer_length,
		 usb_complete_t complete_fn,
		 void *context)


 static inline void usb_fill_control_urb(struct urb *urb,
		 struct usb_device *dev,
		 unsigned int pipe,
		 unsigned char *setup_packet,
		 void *transfer_buffer,
		 int buffer_length,
		 usb_complete_t complete_fn,
		 void *context)

 static inline void usb_fill_int_urb(struct urb *urb,)
		 struct usb_device *dev,
		 unsigned int pipe,
		 void *transfer_buffer,
		 int buffer_length,
		 usb_complete_t complete_fn,
		 void *context,
		 int interval
		 )

		 不同的传输模式下，驱动为之申请不同URB。其中，Linux内核只支持同步传输外
的三种传输事件，ISO事务需要手工进行初始化工作。控制传输事务，批量传输事务，中断
传输事务API如上所示。
	三种事务传输模式下的URB初始化函数有很多相似之处，主要参数含义如下：
	*urb：事务传输中的urb
	*dev：事务传输的目的设备
	*pipe：USB主机与USB设备之间数据传输的通道
	*transfer_buffer: 发送数据所申请的内存缓冲的设备数据包
	*usb_fill_int_urb()的interval:中断传输中两个URB调度的时间间隔


4.2.3 提交URB
	URB初始化完成之后，USBD开始通过usb_start_wait_urb()提交urb请求
	(它调用usb_submit_urb来真正发送URB请求),添加completition函数。

	接下来，从message.c传到主机控制器hcd.c，开始真正的usb_hcd_submit_urb()。
	此时，根据是否为根集线器，进入不同的工作队列。
	usb_start_wait_urb->
	usb_submit_urb    ->
	usb_hcd_submit_urb_>

	a)root_hub传输
	若为root hub，将调用rh_urb_enqueue(),共有两种传输事务(控制传输和中断传输)


	static int rh_urb_enqueue(struct usb_hcd *hcd, struct urb *urb)
	{
		if(usb_endpoint_xfer_int(&urb->ep->desc))
			return rh_queue_status(hcd, urb);

		if(usb_endpoint_xfer_control(&urb->ep->desc))
			return rh_call_control(hcd, urb);
	}





	b)非root_hub传输
	对于非常root_hub传输，它调用：
	status = hcd->driver->urb_enqueue(hcd, urb, mem_flags)



	C)批量传输
	root_hub本身没有批量传输流程，按照控制传输流程，控制传输最终要通过switch
	语句跳转到bulk-only传输流程中。


1.1 USB子系统结构
	用户

	Block Net Char
	USB设备驱动
	USB Core
	USB主机控制器

	协议里说，HCD提供主控制器驱动硬件抽象，它只对USB Core一个负责，USB Core将
	用户的请求映射到相关HCD，用户不能直接访问HCD。换句话说，USB Core就HCD与USB
	设备唯一的桥梁。


1.2 USB子系统的初始化
	usbcore这个模块代表的不是某一个设备，而是所有USB设备赖以生存的模块,它就是
	USB子系统。

	
	./driver/usb/core/usb.c里实现了初始化，伪代码如下

static int __init usb_init(void)
{
	usb_debugfs_init();
	bus_register(&usb_bus_type);
	bus_register_notifier(&usb_bus_type, &usb_bus_nb);
	usb_major_init();
	usb_register(&usbfs_driver);
	usb_devio_init();
	usbfs_init();
	usb_hub_init();
	usb_register_device_driver(&usb_generic_driver, THIS_MODULE);
}

	usbcore注册了USB总线，USB文件系统，USB Hub以及USB的设备驱动usb generic 
	driver等。


1.3 USB总线
	注册总线通过bus_register(&usb_bus_type);
	下面总结USB设备和驱动匹配的过程，
	-> step 1 - usb device driver
						USB总线

						|
						|
						|-----------usb_generic_driver(usb device driver)
						|
						|
						|
						|


	USB子系统初始化的时候就会注册usb_generic_driver,它的结构体类型是
	usb_device_driver,它是usb世界里唯一的一个USB设备驱动，区别于struct
	usb_driver USB驱动。
	*USB设备驱动(usb device driver)就只有一个,usb_generice_driver这个
	对象，所有USB设备都要绑定到usb_generic_driver上，它的使命可以概括
	为：为USB设备选择一个合适的配置，让设备进入configured状态。

	USB驱动(usb driver)就是USb设备的接口驱动程序，比如adb驱动程序，u盘
	驱动程序，鼠标驱动程序等等。


	-> step 2 - usb driver


						USB总线

						|
						|
						|-----------usb_generic_driver)
						|              (usb device driver)
						|
						|-----------usb驱动链表
						|              (usb driver)
						|           adb
						|           mouse
						|           keyboard
						|           usb mass storage
						|           ... ...

	Linux启动时注册USB驱动,在xxx_init()里通过usb_register()将USB驱动提交
	个设备模型，添加到USB总线驱动链表里。

	-> step 3 - usb device

						USB总线

						|
						|
						|-----------usb_generic_driver)
		usb设备链表		|              (usb device driver)
			手机     	|
		    U盘			|-----------usb驱动链表
						|              (usb driver)
						|           adb
						|           mouse
						|           keyboard
						|           usb mass storage
						|           ... ...


	USb设备连接在Hub上，HUb检测到有设备连接进来，为设备分配一个struct usb_device
	结构体对象，并将设备添加到USb总线的设备列表里。

	-> step 4 - usb interface

						USB总线

						|
						|
						|-----------usb_generic_driver)
		usb设备链表-----|              (usb device driver)
	手机 (手机u盘口)	|
	U盘(u盘合适的接口)  |-----------usb驱动链表
						|              (usb driver)
						|           adb
						|           mouse
						|           keyboard
						|           usb mass storage

	USB设备各个配置的详细信息在USB core里的漫漫旅游中已经被获取并存放在相关的
	几个成员里。
	usb_generic_driver得到了USB设备的详细信息，然后把准备好的接口送给设备模型
	Linux设备模型将接口添加到设备链表里，然后去轮询USB产，总线另外一条驱动链
	表，针对每个找到的驱动去调用USB总线的match函数，完成匹配。

1.4 USB Request Block (urb) 
	USB主机与设备间的通信以数据包(packet)的形式传递，Linux的思想就是把这些遵循
	协议的数据都封装成数据块(block)作统一调度，USB的数据块就是urb，结构体
	struct urb定义在，其中的成员unsigned char *setup_packet指针指向SETUP数据包。
	下面使用urb完成一次完整的USB通信需要经历的过程。

	-> step 1 -usb_alloc_urb()
	创建urb，并指定设备的目的端点。
	-> step 2 -usb_control_msg()

	将urb提交给USB core, USB core将它交给HCD主机控制器驱动

	-> step 3 -usb_parse_configuration()
	HCD解析urb，拿到数据与USB设备通信

	-> step 4
	HCD把urb的所有权交还给驱动程序
	协议层里最重要的函数就是usb_control/bulk/interrupt_msg(),这里就简单的一条
	线索，

	usb_control_msg() => usb_internal_control_msg() => usb_start_wit_urt()
	=> usb_submit_urb() => usb_hcd_submit_urb => hcd->driver->urb_enqueue()
	HCD主控制器驱动根据具体平台实现USB数据通信.


2.USB Hub
	Hub集线器用来连接更多的USB设备，硬件上实现了USB设备的总线枚举过程，软件上
	实现了USB设备与接口在USB总线上的匹配。

	下面总结下USB Hub在Linux USB核心层里的实现机制，USB子系统初始化时，
	usb_hub_init()开启一个名为"khubd"的内核线程.

	内核线程khubd从Linux启动后就自始至终为USB Hub服务，没有Hub事件时Khubd进入
	睡眠，有USB Hub事件触发时将会经由hub_irq() => hub_activate() => kick_khubd()
	最终唤醒Khubd，将事件加入hub_event_list列表，并执行hub_events().
	hub_events()会不停地轮询hub_events_list列表去完成hub触发的事件，直到这个列表
	为空时退出结束，回到wait_event_xxx继续等待。

	处理hub事件的全过程大致可分主两步

	第一步 判断端口状态的变化
	通过hub_port_status()得到hub端口的状态。

	源码里类似像hub_port_status();hub_hub_status()等功能函数，都调用了核心层的
	usb_control_msg()去实现主控制器与usb设备间的通信。


	第二步 处理端口的变化
	hub_port_connect_change()是核心函数，以端口发现有新的USB设备插入为例，
	USB Hub为USB设备做了以下几步重要的工作，注意这里所谓的USB设备是指插入USB
	Hub的外接USB设备(包括Hub和Functions),接下来Hub都在为USB设备服务。
	
	(1)usb_alloc_dev()为USB设备申请一个struct usb_device结构。
	(2)usb_set_device_state()设置了USB设备状态为上电状态。
	(3)choose_address()为USB设备选择一个地址，利用一个轮询算法为设备
	从0-127里选择一个地址号。
	(4)hub_port_init()端口初始化，实质就是获取设备描述符device descriptor
	(5)usb_get_status()这个有点特殊，它是专门给Hub又外接Hub而准备的。
	(6)usb_new_device()这里USB设备已经进入了Configured状态，调用device_add()
	在USB总线上寻找驱动，若匹配成功，则加载对应的驱动程序。


3.USB_OTG
	引入OTG的概念是为了让设备可以充当主从两个角色，主设备即HCD，从设备即UDC

3.1 协议
	(1)Protocol
	OTG的传输协议有三类-ADP SRP HNP
	ADP(Attach Detection Protocol)当usb总线上没有供电时，ADP允许OTG设备
	或USB设备连接状态。

	SRP(session Request Protocol)允许从设备也可以控制主设备。

	HNP(Host Negotiation Protocol)允许两个设备互换主从角色

	(2)Device role
	协议定义两种角色，OTG A device OTG B devic
	A为电源提供者，B为电源消费者。


	(3)OTg micro plug

	以Freecale平台为例，当有OTG线插入OTG设备产生中断，中断处理上半部通过读取OTG
	控制器寄存器相应值判断OTG设备属于HCD还UDC，下半部分通过工作队列由回调函数
	host->resume() 或者gadget->resume重启host或者gadget控制器。

4.USB Host
	USB主控制器HCD同样集成在CPU内，由开发平台厂商提供驱动。
	主控制器主要有四类：
	EHCI
	FHCI
	OHCI
	UHCI
	嵌入式设备多为EHCI(usb2.0 标准主机控制器)

		
	该驱动的结构体类型为struct hc_driver,其中的成员(*urb_enqueue)最为重要，它是
	主控制器HCD将数据包urb传输usb设备的核心实现函数 ，之前已经提到过，协议层里
	最主要的函数usb_control_msg()最终就会回调主控制器(*urb_enqueue).

	usb_control_msg() => usb_internal_control_msg() => usb_start_wit_urt()
	=> usb_submit_urb() => usb_hcd_submit_urb => hcd->driver->urb_enqueue()


5.USB gadget
	adb驱动f_adb.c
	f_mass_storage.c等一些复用的USB驱动
	数据结构主要有，
	.struct usb_gadget里面主要有(*ops)和struct usb_ep *ep0

	 struct usb_gadget_driver 其中的(*bind)绑定复用设备驱动，
	 (*setup)完成USB枚举操作。
	 · struct usb_compostie_driver 其中的(*bind)绑定比如android复用设备驱动。
	 · struct usb_request USB数据请求包，类似urb。
	 · struct usb_configuration 就是这个gadget设备具有的配置，其中的
	 struct usb_function *interface[]数组记录着它所拥有的USB接口/功能/驱动。
	 · struct usb_function 其中的(*bind)绑定相关的USB接口
	 ，(*setup)完成USB枚举操作。

	 整体框架可概括为，(mv_gadget为gadget控制器的数据)  

6.USB Mass Storage
全世界只有一个Linux U盘驱动，在usb/storage/usb.c中，在进行u盘驱动的初始化probe
之前，USB core和hub已经对这个U盘做了两大工作，即
（1）完成了usb设备的枚举，此时u盘进入configured状态。u盘数据存放在struct 
	usb_interface.

（2）完成了USB总线上设备和驱动的匹配，这时总线上已经找到了接口对应的驱动即u
盘驱动

· usb_stor_scan_thread 扫描U盘的线程，等待5秒，
如果5秒内不拔出就由SCSI进行全盘扫描，
· usb_stor_contro_thread 一个核心的线程，具体参看《USB那些事》...







	




	

