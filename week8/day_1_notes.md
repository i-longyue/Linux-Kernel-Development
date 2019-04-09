

2.OTG 协议
OTG设备采用Mini-AB插线，相对于传统的usb数据线，Mini-AB接口多了一根数据ID，ID线
是否接入将Mini-A和Mini-B两种类型。
在OTG设备之间数据 连接的过程中，通过OTG数据线Mini-A和Mini-B接口来确定OTG设备
的主从：接入Mini-A接口的设备默认为A设备(主机设备);
接入Mini-B接口的设备，默认为B设备(从设备)

	A设备和B设备无需交换电缆接口，即可通主机交换(HNP)实现A，B设备之间的角色
互换。同时为节省电源，OTG允许总线空闲时A设备判断电源。此时，若B设备希望使用
总线，可以通过会话请求协议(SRP)请求提供电源。

2.1 HNP(主机交换协议)

	当Mini-A接口接入A设备并确定A设备为主机时；若B设备希望成为主机，则A设备向B
设备发送SetFeature命令，允许B设备进行主机交换。B设备检测到总线挂起5ms后，即
挂起D+并启动HNP，使总线处于SE0状态。此时A设备检测到总线处于SE0状态，即认为B
设备发起主机交换，A设备进行响应。待B设备发现D+线为高电平而D-线为低电平(J
状态表示A设备识别了B设备的HNP请求。B设备开始总线复位并具有总线控制权，
主机交换协议完成。


2.2 SRP(会话请求)协议
	对于主机，要求能响应会话请求，对于设备仅要求能够发起SRP协议。
	OTG设备不仅要求发送SRP，而且还能响应SRP请求。

SRP分为数据线脉冲调制和电压脉冲调制两种方式，B设备发起SRP必须以下两个条件：
	1）B设备检测到A设备低于其有效电压阈值，同时B设备低于有效的电压阈值。
	2) B设备必须检测到D+和D-数据线至少在2ms的时间内低于有效阈值，即处于SE0

	数据线脉冲调制会话请求：B设备必须等到满足以上两个条件后，将数据线接入上
	拉电阻一定时间，以备A设备过滤数据线上的瞬间电压。与此同时，B设备上拉D+
	以便在全速模式下进行初始化操作。A设备在检测到D+变为高电或D-变为低电平时
	产生SRP指示信号。

	Vbus脉冲调制会话请求：B设备同样需等待满足上述两个初始化条件，然后B设备
	通过对电容充电以提高总线电压，待达到总线上的电压阈值，唤醒A设备。在充电
	过程中，一定要保证充电的电压峰值在一定的范围以避免烧坏A设备。

3.USB驱动架构

	主机端
	USB Device Driver           Gadget Driver(Mass Storage)
	(Mass storage/HID)


	USB Core                    Gadget API


	USB HID
	(EHCI/OHCI/UHCI)            USB Device Controller Driver


	Host Controller             Device Controller


3.1 USB主机端驱动

		USBD
			                	urb             hcd  hep
						USB数据传输通道     主机状态信息传输通道
						          |               |
								  |           适的数据结构
								  V               V
		HCD
			platform_device driver  Host          Root Hub
									controller    driver
									driver


									中断服务函数 

						USB数据传输通道     主机状态信息传输通道 
						          |                  |  
								  |                  |
								  V                  V

	硬件     Host controller     缓存区  中断管理  寄存器


	USB核心(USBD)是整个USB驱动的核心部分，从上图可知，一方面USBD对接收到USB
	主机控制器的数据进行处理，并传递给上层的设备驱动软件;同时也接收来自上层
	的非USB格式数据流，进行相应数据后传递给USB主机控制器驱动.

	usb_control_msg ----------------->usb_fill_control
		^                                     |
		|                                     V 
	usb_get_descriptor                usb_start_urb    message.c
		^                                     |
		|                                     V
	usb_get_device_descriptor         usb_submit_urb     urb.c
		^                                     |
		|                        _____________|______________   hcd.c   
		|                       |                            |
		|                       V                            v
        |                   rh_urb_enqueue               wait_for_compet ition
		|                       |                            |
		|                       V                            V
register_root_hub   mod_timer hcd->driver->hub_control Hcd->driver->urb_enqueue
		^                       |                            |
		|                       v                            v 控制器驱动
	usb_add_hcd                 hub_control               urb_enqueue

	USB数据传输都以URB(USB Request Block)请求，URB生成生成，URB递交，URB释放
	为主线。从上图可知，当加载控制器驱动之后，注册根据集线器，hub和hcd驱动成
	为了一个整体。接着，主机通过控制 传输获取设备的控制描述符信息，接着详述
	整个控制传输的流程。
	usb_submit_urb依据是否连接到根集线器来决定调用urb_enqueue或rh_urb_enqueue
	函数。

	USB从设备通过集线器或根集线器连接到主机上。比如：主机通过根集线器与外界进行
	数据交互，根集线器通过探测数据线状态的变化来通知USB主机是否有USb外围设备接入。


	在主机端控制器驱动加载过程中，注册了根集线器，然后匹配了相应的hub驱动程序，
	同时完成对Hub的轮询函数和状态处理函数的设置。这样，一旦hub集线器的状态发生
	变化，就会产生相应的中断，主机端控制器就会执行相应的中断处理函数，下图为hub
	驱动程序的流程图。
	

	otgip_ehci_probe
	otg_ohci_probe

	usb_create_hcd
		创建HCD
	|
	V
	usb_add_hcd
	|
	v
	register_root_hub
	|
	v
	hub_probe-------> otg_hub_status_data
		              otg_hub_control
	^                       |
	|                       V
	usb_init usb.c        rh_timer_func
							|
							V
						usb_hcd_giveback_urb
						    |
							v
					    hub_irq回调函数
							|	
							v
						唤起线程
							|	
							v
						hub_event
							|	
							v
						创建，执行urb，调用
						hub control，获取
						root hub状态并复位
							|	
							v
						hub_port_connect_change检测到
						端口状态变化

		USB Core中的usb_init()函数中完成了对hub线程（khubd,在usb_hub_init函数
		中真正地创建)创建，然后完成相应设备的探测。主机端控制器驱动进行探测时，
		将hub驱动和主机端控制器驱动结合在一起，相互之间完成调用。
		相对于大容量存储设备与主机之间通过控制/批量传输，集线器与主机之间通过中断
		/控制方式完成数据交互。
		


3.2 USB设备端驱动

	ARM侧
	ARM处理器                IP侧
	Flash                    FIFO Buffer
	控制器                   Device Controller

Flash  System RAM            Device SIE                <------->PHY


FLASH

	用户应用程序            上层应用程序



	文件系统管理            大容量传输协议



Flash底层设备驱动           设备控制器驱动
从上图可知，设备端驱动包含两部分：
1）底层设备控制器驱动
2）上层大容量存储驱动



3.2.1 设备控制器驱动
	USB设备控制器驱动主要实现Gadget API定义的函数和中断服务函数，可按功能划分
	为：API函数实现模块和中断处理模块。
	API函数主要实现Gadget API定义的函数功能，如结构体usb_ep_ops和usb_gadget_ops
	中的函数，usb_gadget_register_driver函数。这些函数是Gadget Driver调用。

	中断处理模块主要处理设备控制器产生的各种中断，包括端点中断，复位挂起等中断。

	Gadget驱动

	usb_gadget_register_driver                              全局变量
	platform_driver_register(struct platform_driver *udc)

	udc->probe()     注册设备device_register()
	初始化usb_ep()
	初始化usb_gadget()
	申请中断request_irq()                                   中断服务程序udc_irq


	上图为设备端控制器基本架构，主要完成了Gadget驱动和控制驱动绑定
	usb_gadget_register_driver注册。


3.3 OTG驱动

               应用层
 

OS FS   USBD    OTG     GADGET  OS FS

		HCD              UDC
		        HAL

OS_FS：文件系统
USBD：USB核心
HCD：主机控制器驱动

	OTG设备支持HNP和SRP协议。OTG设备通过USB OTG电缆连接到一起，其中接Mini-A接口
	的设备为A设备，默认为主机端，Mini-B接口的设备默认为B设备。当A，B设备完成数据
	交互之后，A，B设备之间的USB OTG电缆进入挂起状态。

	当B设备写入b_bus_req,向A设备发起HNP请求，待A设备响应之后，A
	设备发送a_set_b_hnp_en,B设备响应之后即进入主机状态，同时发送请求使用A设备
	set_device,这样A，B设备完成主从交换。



4.USB 传输流程

	控制传输流程

4.1 USB初始化
	usb驱动作为一个系统，集成了众多的驱动模块，注册过程非常复杂。从USB系统
	的角度来说，USB主机驱动主要包含：

	1）USB核驱动
	2）主机控制器驱动
	3）集线器驱动
	驱动的加载执行流程如下图所示：


4.1.1 USB Core
	1.subsys_initcall(usb_init);
	2.module_exit(usb_exit);

	subsys_initcall()是一个宏，可以理解为module_init().
	由于此部分代码 非常重要，开发者把它看作一个子系统，而不仅仅是一个模块。
	USB Core这个模块代表的不是某一个设备，而是所有USB设备赖以生存的模块。
	在Linux中，像这样一个类别的设备驱动被归结为一个子系统。
	subsys_initcall(usb_init)告诉我们，usb_init才是真正的初始化函数，而
	usb_exit将是整个usb子系统结束时的清理函数

4.1.2 主机控制器驱动的初始化及驱动执行(以EHCI为例子)









