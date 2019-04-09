
Linux的设备驱动都遵循一个惯例表征驱动程序（用drivert更贴切一切，应该称为驱动
器比较好吧）的结构体，结构体里面应该包含了驱动程序所需要的所有的资源。
就是这个驱动对象所拥有的属性及成员。所以我们也按照这各结构化的思想来分析代码

鼠标可能有一个叫做mouse_dev的struct。
键盘可能有一个keyboard_dev的struct

Linux内核源码中有一个usb-skeleton(就是usb驱动的骨架咯)
定义的设备结构体就叫做usb-skel:struct usb_skel{
	struct usb_device    *udev;
	struct usb_interface *interface;
	struct semaphore     *limit_sem;
	unsigned char *      bulk_in_buffer;
	size_t               bulk_in_size;
	__u8                 bulk_in_endpointAddr;
	__u8                 bulk_out_endpointAddr;
	struct kref          kref;
}

USB能够自动监测设备，并调用相应驱动程序处理设备，所以以其规范实际上是相当复杂的，
幸好我们不必理会大部分细节问题，因为Linux已经提供相应的解决方案。
USB的驱动分为两块，一块是USB的bus驱动，这个东西，Linux内核已经做好了，我们可以
不管，但我们至少要了解他的功能。形象得说，USB的bus驱动相当于铺出一条路来，让
所有的信息都可以通过这条USB通道到达该到达的地方，这部分工作由usb_core来完成。

当设备接到USB接口,usb_core就检测该设备的一些信息，例如生产厂商ID和产品ID，或者 
是设备所属的class，subclass跟protocol。

以便确定该调用哪一个驱动处理该设备。里面复杂细节我们不用管，我们要做的是另一块
工作__usb的设备驱动。也就是说我们等着usb_core告诉我们要工作了，我们才工作。
对于开发人员来说，他大概如图所示：从开发角度看,每一个usb设备有若干个配置组成，
(configuration)组成,每个配置又可以有多少个接口(interface),每个接口又有多个设置
(setting图中没有给出),而接口本身没有端点或者多个端点（endpint）,USB的数据交换
通过端点来进行，主机与各个端点建立起单向管理来传输数据。而这些接口可以分为四类：

控制(control)
用于配置设备，获取设备信息，发送命令或者获取设备的状态报告。

中断(interrupt)
当USB宿主要求设备传输数据时，中断端点会以一个固定的速率传送。

等时(isochronous)
大量数据的不可靠传输，不保证数据的到达，但保证恒定的数据流,多用于数据采集。

Linux中用struct usb_host_endpint来描述USB端点，每个usb_host_endpoint中包含
struct usb_endpoint_descriptor结构体，当中包含该端点的信息以及设备自定义
的各种信息，这些信息包括
bEndpointAddress(b for byte)
8位端点地址，其地址还隐藏了端点方向的信息(之前说过，端点是单向的),可以用掩码
USB_DIR_OUT和USB_DIR_IN来确定。
bmAttributes
端点的类型，结合USB_ENDPOINT_XFERTYPE_MASK可以确定端点是

USB_ENDPOINT_XFER_ISOC
USB_ENDPOINT_XFER_INT
USB_ENDPOINT_XFER_BULK

wMaxPacketSize
	端点一次处理的最大字节数。发送的BULK包可以大于这个数值，但会被分割传送。

bInterval
	
在逻辑上，一个USB设备的功能划分是通过接口来完成的。比如说一个USB扬声器，
可能会有两个接口：一个用于键盘控制，另外一个用于音频流传输。而事实上
这种设备需要用到不同的两个驱动来操作，一个控制键盘，一个控制音频流。但是
也有例外，比如蓝牙设备要求有两个接口，第一个用于ACL和EVENT的传输，另外
一个用于SCO链路，但两者通过一个驱动控制。在Linux上，接口使用struct
usb_interface来描述，以下是该结构体中比较重要的字段

struct usb_host_interface *altsetting(注意不是usb_interface)

每个usb_host_interface都包含一套由struct usb_host_endpoint定义的端点配置。
但这些配置次序是不定的。

unsigned num_altstting
int minor
当捆绑到该接口的USB驱动程序使用USB主设备号时，USB core分配的次设备号.
仅在成功调用usb_register_dev之后才有效。除了它可以用struct usb_host_config来
描述之外.

整个USB设备则可以用struct usb_device来描述，但基本上只会用它来初始化
函数的接口，真正用到的应该是我们之前所提到的自定义的一个结构体。

了解过USb一些规范细节后，我们现在看看Linux的驱动框架，事实上，Linux的设备驱动
特别是这种hot plug的usb驱动，会被编译成模块，然后需要时，挂在到内核。要写一个
Linux的模块并不复杂，以一个helloworld模块为例：

1 linux
2 linux内核
3 进程管理
4 进程调度
5 系统调用 
6 内核数据结构
7 中断
8 中断推进
9 同步概述
10同步方法
11定时器和时间管理
12 内存管理
13 VFS
14 I/O
15 进程地址空间
16 高速缓存和页回写
17 设备模型
18 调试
19 移植性
20 社区


21.DTS DTSi


module_init(hello_init);
moudle_exit(hello_eixt);
这个简单的程序告诉大家应该怎么写一个模块，MODULE——LICENSE告诉内核模块的版权信息
，很多情况下，用GPL或者BSD，或者两个，因为一个私有模块一般很难得到社区的帮助
module_init(hello_init);moudle_exit(hello_eixt);用于向内核注册模块的初始化和退出函数 

可以用insmod跟rmmod来验模块的挂在和卸载。
下面我们来分析一下usb-skeleton的源码吧。这个范例程序可以在drviers/usb下找到。
注册和注销方法很简单

usb_register(struct *usb_driver)
usb_unregister(struct *usb_driver)

	static struct usb_driver skel_driver = {
		.name = "skeleton",
		.probe = skel_probe,
		.disconnect = skel_disconnect,
		.id_table = skel_table,
	};

从代码看来，usb_driver需要初始化四个东西:
	模块的名字skeleton probe函数skel_probe,disconnect函数skel_disconnect 
	id_table


	在解释skel_driver各个 成员之前，我们先来看看另一个结构体。这个结构体的名字
	有开发人员自定义，它描述的是该驱动拥有的资源及状态：
	struct usb_skel{
		struct usb_device* udev;
		struct usb_interface *interface;
		struct semaphore   limit_sem;
		unsigned char * bulk_in_buffer;
		size_t          bulk_in_size;
		__u8            bulk_in_endpointAddr; 
		__u8            bulk_out_endpointAddr; 
		struct          kref;
	}
 我们先来对这个usb_skel作个简单分析，他拥有一个描述usb设备的结构体udev,一个接口
 interface,用于并发访问控制的semaphore(信号量)
 limit_sem,用于接收数据的缓冲bulk_in_buffer及其尺寸bulk_in_size
 最后是一个内核使用的引用计数器。他们的作用我们将在后面的代码中看到。
 我们在回过头来看看skel_driver.
 Name用来告诉内核模块的名字是什么，这个注册之后有系统来使用，跟我们关系不大。
 id_table用来告诉内核该模块支持的设备。USB子系统通过设备
 production ID和vendorID的组合或者设备的class subclass protocol组合来识别设备
 并调用相关的驱动程序作处理。我们可以看看这个id_table到底是什么东西：


 MODULE_DEVICE_TABLE的第一个参数是设备的类型，如果是USB设备，那自然是USB一个宏
 来注册所支持的设备。这设计PCI设备的驱动了。后面的一个参数是设备表。
 
 当有一个设备接到集线器时，usb子系统就会检查这个设备的vendor ID和product ID
 如果它们的值是0xfff0时，那么子系统就会调用这个skeleton模块作为设备的驱动。
 probe是usb子系统自动调用的一个函数，有USB设备接到硬件集线器时，usb子系统会
 根据production ID和vendor ID等待来调用相应的驱动程序的probe，对于skeleton
 来说就是skel_probe.系统会传递给probe一个usb_interface 跟一个struct 
 usb_device_id *作为参数。他们分别是该USB设备的接口描述(一般会是该设备的第0号接口
 跟它的设备ID描述（包含Vendor ID Production ID等)
 probe函数比较长，我们分段来分析这个函数：

 usb_get_dev(interface_to_usbdev(interface));
 在初始化一些资源之后，我们可以看到第一个关键的函数调用interface_to_usbdev
 用来得到该接口所在设备的设备描述结构。
 usb_device的计数，并不是对本模块的计数，本模块的计数要由kref来维护。所以，
 probe一开始就有初始化kref.事实上，kref_init操作不单只初始化了kref还将其设置为
 1，所以在出错的代码中有kref_put,它把kref的计数减1，如果kref计数已经为0，那么kref
 会被释放 

 当我们执行打开操作时，我们要增加kref的计数，我们可以用kref_get,来完成。所有
 对struct kref 的操作都有内核代码确保其原子性。
 得到了usb_device之后，我们要对我们自定义的usb_skel各个跟资源作初始化。


 接下来的工作是向系统注册一些以后会用的信息，首先说明一下set_intfdata(),他向内核
 注册一个data，这个data结构可以是任意的。在这段程序 是向内核注册了一个usb_skel结构
 就是我们刚刚初始化的那个，这个data在以后用usb_get_intfdata

 usb_set_intfdata(interface, dev);
 retval = usb_register_dev(interface, &amp; skel_class);

 然后我们向这个 interface注册一个skel_class结构。。

 这个结构又是什么？我们就来看看这个到底是什么东西：
 static struct usb_class_driver  skel_class = {
	 .name = "skel%d",
	 .fops = &amp;skel_fops,
	 .minor_base = USB_SKEL_MINOR_BASE,
 };

它其实是一个系统定义的结构，里面包含了一名字，一个文件操作操作结构体还有一个次
设备号的基准值。事实上它定义真正完成对设备IO操作的函数。所以他的核心内容应该是
skel_fops。这里补充一些我个人的估计:
因为usb设备可以有多个interface，每个interface所定义的IO操作操作可能不一样，所以
想系统注册的usb_class_driver要求注册到某一个interface,因此usb_register_dev的第
一个参数是Interface，而第二个参数就是某一个usb_class_driver.

通常情况下，linux系统用主设备来识别某类设备的的驱动程序，用次设备号管理识别具体
的设备，驱动程序可以依照次设备好来区分不同设备，所以，这里的次设备号用来管理不同
的interface的，但由于这个范例只有一个interface，在代码上无法求证这个

static struct file——operations skel_fops = {
	.owner = THIS_MODULE,
	.read  = skel_read,
	.write = skel_write,
	.open  = skel_open,
	.release = skel_release,
};

这个文件操作结构中定义了对设备的读写，打开释放(USB设备通常使用这个术语release)
他们都是函数指针，分别指向skel_read, skel_write, skel_open, skel_release这四
个函数，这个四个函数应该有开发人员自己实现。

当设备被拨出集线器时，usb子系统会自动地调用disconnect,他做的事情不多，最重要的
是注销class_driver和interface的data：

dev = usb_get_intfdata(interface);
usb_set_intfdata(interface, NULL);
usb_deregister_dev(interface, &amp;skel_class);

然后他会用kref_put(&amp; dev-&gt; kref, skel_delete)进行清理，kref_put的细节
参见前文

到目前为止，我们已经分析完usb子系统要求的各个主要操作，下一部分我们在讨论
一下对USB设备的IO操作。

说的usb子系统的IO操作，不得不说usb request block，简称urb。事实上，可以
打一个这样的比喻，usb总线就像一条高速公路，货物，人流之类可以看成是系统与设备
交互的数据，而urb就可以看成是交通工具，在一开始对USB规范，我们就说过USb的
endpoint有4种类型，于是能在这条高速公路上流动的数据也就有四种。但对车是没有要求
的，urb可以运载四种数据，不过你要先告诉司机你要运什么，目的地是什么.
我们现在就看看struct urb的具体内容。include/linux/usb.h

在这里我们重点介绍程序中出现的几个关键字段：
struct usb_device *dev
urb所发送的目标设备。
unsigned int pipe

	一个管理号码，该管道记录了目标设备的端点以及管道的类型。每个管理只有一种
类型和一个方向，它与他的目标设备的端点对应，我们可以通过以下几个函数来获得
管道号并设置管道类型：

unsigned int usb_sndctrlpipe(struct usb_device *dev, unsigned int endpoint);
	把指定USB设备指定端点设置为一个控制OUT端点。

unsigned int usb_rcvctrlpipe(struct usb_device *dev, unsigned int endpoint);

当不使用DMA是，应该transfer_flags |= URB_NO_TRANSFER_DMA_MAP(按照代码的理解，
		希望没有错)。
int status

	当一个urb把数据送到设备时，这个urb会由系统返回给驱动程序，并调用驱动程序的
urb完成回调函数处理。这时，status记录了这次数据传输有关状态，例如传送成功与否

要能够运货那当然首先要有车，所以第一步当然要创建urb：
	struct urb *usb_alloc_urb(int isoc_packets, int mem_flags);
	第一个参数是等时包的数量，如果不是乘载等时包应该为0，每个参数与kmalloc标志
	相同。
	要释放一个urb可以用:
	void usb_free_urb(struct urb *urb);

	要承载数据，还要告诉司机目的地信息跟要运的货物，对于不同的数据，系统提供了
	不同的函数，对于中断urb我们用
	void usb_fill_int_urb(struct urb *urb, struct usb_device *dev,
		unsigned int pipe,
		void *transfer_buffer,
		int buffer_length,
		usb_complete_t complete,
		void *context,
		int interval);	

这时要解释一下，transfer_buffer是一要送/收的数据的缓冲,buffer_length是它长度，
complete是urb完成回调函数的入口，context有用户定义，可能会在回调函数中使用的
数据，interval就是urb被调度的间隔。

	对于批量urb和控制urb,我们用：
	void usb_fill_bulk_urb(struct urb *urb, struct usb_device *dev,
			unsigned int pipe,
			void *transfer_buffer,
			int buffer_length,
			usb_complete_t complete,
			void *context);

控制包有一个特殊参数setup_packet,它指向即将被发送到端点的设置数据报的数据

对于等待urb，系统没有专门的fill函数，只能对各urb字段显示赋值。
有了汽车，有了司机，下一步就是要开始运货了，我们可以用下面的函数来提交urb

当我们的卡车运货之后，系统会把它调回来，并调用urb完成回调函数，并把这辆车作为
函数传递给驱动程序。我们应该在回调函数里面检查status字段，以确定数据的成功传输
与否。下面是用urb来传送数据的细节。 

/* initialize the urb properly */
usb_fill_bulk_urb(urb, dev-&gt; udev,
		usb_sndbulkpipe(dev, udev,
			bulk_out_endpointAddr),
		buf, writesize, skel_write_bulk_callback,dev);

/* send the data out the bulk port */
	retval = usb_submit_urb(urb, GFP_KERNEL);
这里skel_write_bulk_callback就是一个完成 回调函数，而他做的主要事情就是检查数据
传输状态和释放urb：
	dev = (struct usb_skel *)urb..context;


	事实上，如果数据的量不大，那么可以不一定用卡车来运货，系统还提供了一种不用
	urb的传输方式，而usb-skeleton的读操作正是采用这种方式：
	/* do a blocking bulk read to get data from the device */
		usb_bulk_msg来传送数据，它的原型如下：
		int usb_bulk_msg(struct usb_device *usb_dev,
				unsigned int pipe,
				void *data,
				int len,
				int *actual length,
				int timeout)

	这个函数会阻塞等待数据传输完成或者等到超时，data是输入/输出缓冲，len是它的大小，
	actual length是实际传送的数据大小，timeout是阻塞超时。

	对于控制数据系统还提供另外一个函数，他的原型是：
	Request是控制消息的USB请求值，requesttype是控制，消息的usb消息索引。

 　

