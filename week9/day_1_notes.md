Linux 那些事之我是U盘
1.小城故事
	drviers/usb/storage
我们要关注的文件就是scsiglue.c,protocol.c,transport.c,usb.c, initializers.c
以及它们同名的.h文件.

5.外面的世界很精彩
	于是在drivers/usb下面出来了一个core目录，就专门放一些核心的代码，比如初始化整个
usb系统，初始化RootHub,初始化主机控制器的代码。后来把主机代码单独移到了host目录
下面让负责各种主机控制器的人去维护。常见的主机控制器有有三种：EHCI UHCI OHCI。

USB出来了三个概念，USB Core，USB主机控制器和USB设备。

6.未曾开始却似结束
	USB设备为设备联姻的是总线，是它把设备连入了计算机主机。嫁给了计算机驱动。

7.狂欢是一群人的孤单。
Linux设备模型中三个很重要的概念就是总线，设备和驱动，即bus，device和driver。
而实际上内核中也定义了这么一些数据结构，它们是struct bus_type, struct device,
struct device_driver,这三个重要的数据结构都来自同一个地方，即
include/linux/device.h

	usb_bus_type，它们都是struct bus_type类型的变量。
而struct bus_type结构中两个
非常重要的成员就是struct kset drivers和struct kset devices。kset和另一个叫做
kobject正是2.6内核中设备模型的基本元素。


	我们只需要知道，drivers和devices的存在，让struct bus_type与两个链表联系了
起来，分别是devices和drivers的链表。也就是说，知道一条总线所对应的数据结构，就
可以找到这条总线所关链的设备,及支持的驱动程序。
	而要实现这些目的，就要求每次出现一个设备，就要向总线汇报或者注册。

	每次出现一个驱动，也要向总线汇报.每次一个驱动，也要向总线汇报。
介绍系统初始化时,会连接那些设备，并为每一个设备建立起一个struct device的
变量，每一次有一个驱动程序，就要准备一个struct device_driver结构的变量。
把这个变量统统加入相应的链表，设备插入devices链表，驱动插入drivers链表。
这样通过总线就能找到每一个设备，每一个驱动。

	然而，假如计算机里只有设备却没有相应的驱动，那么设备无法工作。反过来，倘若
	只有驱动却没有设备，驱动也起不了任何作用，设备开始多了，驱动开始多了。它们
	像来自两个世界。


#8.总线，设备，驱动 
	struct bus_type中为设备和驱动准备了两个链表，
	
	而代表设备的结构体struct device  
						struct bus_type *bus
						struct device_driver *driver

	而代表驱动的结构体struct device_driver
						struct bus_type *bus
						struct list_head devices;

	struct device和struct device_driver的定义和struct bus_type一样,

	具体到USB系统，完成这个工作的就是USB Core的代码。USB Core会进行整个USB
	系统的初始化，比如申请总线，然后会扫描USB总线，看线上连接了那些USB设备
	或者说Root Hub上连上哪些USB设备，比如说连接了一个USB键盘，那么就为它
	准备了一个struct device,根据它的实际情况，为这个struct device赋值，并插入
	devices链表中来。又比如Root Hub上连接一个普通的实际情况，为这个struct device
	赋值，并插入devices链表中来。

	又比如Root Hub上连了一个普通的Hub，那么除了要为这个Hub本身准备一个struct 
	device以外,还要描述这个Hub是否又连上的别的设备。如果有的话继续重复之前的
	事情，这样一直进行下去，直到未完成整个扫描，最终就把usb_bus_type中的链表
	给建立起来。

	那drivers链表呢？这个就不用bus方面主动了，而该由每一个驱动本身去bus上登记
	或者挂牌。具体到USB系统，每一个USB设备的驱动程序都会有一个struct usb_driver
	结构体.

	将usb device_driver driver插入到usb_bus_type中drivers链表中去。 
	作为一个实际的USB驱动，它在初始化阶段所要做的事情就很少，很简单了，直接调用t
	usb_register即可。





#9.总线，设备和驱动
	bus上的两张链表记录了每一个设备驱动，那么设备驱动和驱动这两者之间又是如何
	联系起来的呢？此刻，必须抛出这样一个问题，先有设备还是先有驱动
	在目前，设备可以在任何时刻出现，而驱动也可以在任何时刻被加载。所以，就出现
	每当一个struct device诞生，它就会去bus的drivers链表中寻找自己的另一半，反之
	每当一个个strcut device_driver诞生，它就会去bus的devices链表中寻找它的设备

	usb_register,是传递了一个usb_storage_driver

	struct usb_driver的结构体变量

	重点是THIS_MODULE
	而那么就是这个模块的名字，USB Core会处理它，所以如果这个模块正常被加载后，


#10 我是谁的他
	probe, disconnect，id_table，这三个元素中首先要登场的是id_table，它是

	我们说过，一个设备只能绑定一个驱动，但驱动并非只能支持一种设备。道理很简单
	比如，我有两块u盘，那么我可以一起都插入，但是需要加载一个模块，usb-storage.
	没听说过插入两块u盘就得加载两次驱动程序的，除非这两块u盘本身就使用不同的
	驱动程序，也是因为一个模块可以被多个设备共用，才会有设备计数这个说法。

		既然一个驱动可以支持多个设备，那么当发现一个设备时，如何知道哪个才是它
	的驱动呢？这就是id_table的用处，让每一个struct usb_dviver准备一张表，里面
	注明该驱动支持哪些设备。

	于是我们知道，一个usb_driver会把它的id表和每一个usb设备的实际情况进行比较。
	如果该设备的实际情况和这张表里的某一个id相同，准确地说，只有这许多特征都吻合
	才能把一个USB设备和这个USB设备驱动进行绑。


#11.从协议中来，到协议中去
- 在struct usb_driver中， .probe和.disconnect的原型如下：
```c
	int (*probe)(struct usb_interface *intf,
				 const struct usb_device_id *id);
	void (*disconnect) (struct usb_interface *intf);
```
	我们看来其中的参数，struct usb_device_id这个不用说了，刚才已经介绍过过，那么
struct usb_interface从何而来，还是让我们从struct usb_device说起。
	实际上对于一个u盘来说，它就是对应这么一个struct usb_device的变量，这个变量由
	USB Core负责申请和赋值。但是我们需要记住这个结构体变量，因为日后我们调用USB
	core提供的函数时，会把这个变量作为参数传递上去，因为很简单，要和USB Core
	交流，总得让人家知道我们是谁吧。比如后来要调用的一个函数，
	usb_buffer_alloc，它就需要这个参数。

	而对u盘设备驱动来说，比这个struct usb_device更重要的数据结构是，struct 
	usb_interface。

#12.从协议中来，到协议中去(中)
	关于这几条描述符，USB Core在总线扫描时就会去读取，获得里面的信息，其中，设备
	描述符描述的是整个设备。

	对于USB设备驱动程序编写者来说，更为关键的是下面的接口和端点。先说接口
	第一句一个接口对应一个USB设备驱动程序。

	一个usb设备，两个功能，一个键盘，上面带一个扬声器。
	道上的兄弟喜欢这样的两个整合在一起的东西叫做一个设备，我们用接口来区分这两者。
	于是有了我们前面提到的那个数据结构，struct usb_interface它定义 于
	include/linux/usb.h
```c
	struct usb_interface {

	};
```
	这个结构体贯穿整个u盘驱动程序，虽然我们不用去深入了解，整个u盘驱动程序在
	后面提到的struct usb_interface都是同一个变量，这个变量是在usbcore总线扫描
	时就申请好了的。我们只需要直接使用就是了。比如前面说过的storage_probe
	storage_disconnect
	这个函数中的intf

	因为有函数需要intf有些函数需要usb_device

#13.从协议中来，到协议中去(下)


#14.梦开始的地方 
	整个u盘的大戏从storage_probe开始，由storage_disconnect结束。其中storage_probe
这个函数占了相当大的篇幅，我们一段一段地来看，这两个函数都来自drivers/usb/storage/usb.c
	us_data
	不难发现，linux内核中每一个重要的数据结构都很复杂。
	换句话，我们会为设备申请一个us_data,因为这个结构里的东西我们之后一直会用得着。

	usb_stor_host_template
	scsi_host_alloc就是scsi子系统提供的函数，它的作用就是申请一个SCSI Host相应
	的结构指针，之所以USB Mass Storage 里面会涉及SCSI这一层。
	因为事实上把一块U盘模拟成一块SCSI设备，对于SCSI设备来说，更想正常工作，得有
	一个SCSI Hos。
	我们就得提供一个struct scsi_host_template结构体，这其实从名字可以看出，是一
	个模板，SCSI那层把一切都封装好了，只要提供一个模板给它，它就能为你提供一个
	struct Scsi_Host结构体。关于这个usb_stor_host_template,它的定义或者说初始化
	是在drivers/usb/storage/scsiglue.c中：

	按SCSI层的规矩，要想申请一个SCSI Host并且让它工作，我们需要调用三个函数，
	第1个是scsi_host_alloc(),第2个是scsi_add_host,第3个是scsi_scan_host.

	最后需要指出的是，scsi_host_alloc()需要两个参数，每1个参数是struct 
	scsi_host_template的指针，咱们当然给了它&usb_stor_host_template,而第二个
	参数实际上是被称为自己的数据，咱们传递的是sizeof(*us).SCSI层非常友好地给
	咱们提供一个接口，在struct Scsi_Host结构体被设计时就专门准备了一个
	unsigned long hostdata[0]来给别的设备驱动使用，这个hostdata的大小是可以由
	咱们来定的，把sizeof(*us)传递给scsi_host_alloc就意味着给us申请了内存，而
	今后如果我们需要从us得到相应的scsi Host就可以使用内联函数us_to_host(),
	而反过来要想从SCSI Host得到相应的us则可以使用内联函数host_to_us().

	总之咱们这么一折腾，就让USB驱动和SCSI Host联系起来。从此以后这个U盘既要
	扮演U盘的角色又要SCSI

	usb_usual_check_type()干什么用的，这个函数来自drivers/usb/storage/libusual.c
	中：

	这个函数就是保证现在认领的这个设备属于usb-storage所支持的设备。而不是另一个叫
	叫ub的驱动所支持的设备。




#15.设备花名册
	storage_probe这个函数挺有意思的，总长度不足100行，但是干了许多事情，这就像
	足球场上的后腰。


#16.冰冻三尺非一日之寒
	associate_dev()
	get_device_info()

	us之所以重要，是因为接下来很多函数都要用到它，以及它的各个成员。实际上目前
	这个函数associte_dev所做的事情就是为us的各个成员赋值，毕竟此刻us和我们之前
	提到的那些函数struct usb_device,struct usb_interface还没有一点关系。因而
	如此去为它赋值，主要就是因为后面要利用它。
	....

#17冬天来了，春天还会远吗？
	在整个usb-storage模块代码中，其最灵魂的部分在一个叫做
	usb_stor_control_thread()的函数中。

	get_device_info get_transport get_protocol get_pipes
	写代码的人把这些工作分配给了
	get_transport get_protocol get_pipes

	这里不得不说的是，这个世界上有许多USB设备，分了区分它们，USB规范或者说USB
	协议也被分成了子类。USB协议也是，首先每个接口属于一个Class.

	USB协议中为每一种Class,每一种SubClass和每一种Protocol定义一个数值。比如
	Mass Storage的Class就是0x08.

	说了这么多，U盘属于其中的哪一种呢?USB协议中规定，U盘的SubClass是属于
	US_SC_SCSI,而共通信协议使用的是bulk-only的




#18.冬天来了，春天还会远吗？(二)
	打开unusual_devs.h吧,之前我们看了它的最后几行，但是如果你仔细看的话会发现
	最后几行和前面的一些行有着明显的不同。最后几行都是USUAL_DEV的宏，
	下面这个设备，正是来自三星的一个Flash产品
	/* Submitted by Hartmut Wahl <hwahl@hwahl.de> */
	UNUSUAL_DEV( 0x0839, 0x000a, 0x0001, 0x0001,
			"sm msung"
			...)

	而我们之所以使用两个数组的原因是，storage_usb_ids是提供给USB Core，它需要
	比较驱动和设备从而确定设备是被这个驱动所支持的，我们只需要比较四项就可以了
	因为这四项已经可以确定一个设备的厂商，产品，序列号。

#19.冬天来了，春天还会远吗？(三)
	从两张表得到了我们需要的东西，然后下面的代码就是围绕着这两个指针来展开了。
	(unusual_dev和id)继续看get_device_info().

	把unusual_dev给记录在us里面，反正us里面也有这么一个成员.这样记录下来以后
	使用起来方便了.

	我们看到use_Protocol这一栏里写了US_SC_UFI,这表明它自称是属于UFI这个SubClass
	的，但是如果我们从它的描述符里面读出来也是这个，那就没有必要注明在这里了。
	这里直接写成US_SC_DEVICE好了。当然，总的来说这段代码有一些傻。写代码是希望
	能够更好地管理unusual_device.h希望它不要不断增加。
	

#20.冬天来了，春天还会远吗？(四)
	结束了get_device_info,我们继续沿着storage_probe一步一步地走下去。继续，
	这是我们前面提到过的三个函数，get_transport,get_protocol,和get_pipes.
	一旦结束这三个函数，我们就将进入本故事的高潮部分。

	LUN就是LogicalUnit NuMber。通常在谈到SCSI设备时，不可避免的要说起LUN
	一个LUN就是设备中的一个驱动，下面举例来说USB中引入LUN的目的。
	有些读卡器可以有多个插槽，比如有两个，其中一个支持CF卡，另一个支持SD卡，
	那么这种情况要区分两个，就引入了LUN。
	有时候人们常把U盘中的一个分区当做一个LUN,但是不应该这么理解。


#21.冬天来了，春天还会远吗？
	看完get_transport()继续看get_protocol函数和get_pipe函数。仍然是来自
	driver/usb/storage/usb.c
	根据us->subclass来判断，对于u盘来说，spec里面规定了，它的SubClass是
	US_SC_SCSI，所以这里就是两句赋值语句：一个是令us的protocol_name为
	Transparent SCSI。另一个是令us的proto_handler为usb_stor_transparent_scsi
	_command,后者又是一个函数指针。

	然后是get_pipes(),来自drivers/usb/storage/usb.c
	有了前面的两个函数作为基础，这个函数就不用说了，如果这个端点就是中断
	端点，那么就让ep_int指向它。MSC规定一个USB MASS Storage设备至该有两个
	批量端点。控制端点显然是必需的。

	可能会有一个中断端点，这种设备支持CBI协议。U盘遵守的是Bulk-only协议。
	它不需要有中断端点。

	首先我们从storage_probe出发，一共调用了五个函数，它们是assocaite_dev，
	get_device_info, get_transport, get_portocol, get_pipes.
	我们这样做的目的就是为了建立 一个数据结构，us_data.


	

#22.通信春天的管道
	1991年，一个在linux中引入管理的概念，并且把管道用在很多地方，如文件系统，
	设备驱动中，于是后来我们看到在linux中有了各种各样的管道。

	眼下我们在USB代码中看到的管道就是用来传输数据及通信。通信是双方的，不可能
	自言自语。而在USB的通信中，一方肯定是主机，另一方是什么？是设备？

	而从概念上来说，端点是主机和USB设备之间通信的终点。主机和设备可以进行不同的
	种类通信。或者说数据传输。首先，设备连接在USB总线上，USB总线为分辨每一个设备
	给每一个设备上编号，然后为了实现多种通信，设备方于是提供了端点，端点多少，
	自然要编上号，而让主机最终和端点失去联系。

	一个设备能支持这四种传输的哪一种或者几种是设备本身的属性，在硬件设计时就
	确定了。比如一个纯粹的U盘。
	至此，get_pipes函数结束了，信息都保存到了us里面。下面us该发挥它的作用了。
	回到storage_probe()函数，把us作为参数传递给了usb_stor_qcquire_resources()
	函数.
	kthread_creadte(),这个函数造就了许多经典的Linux内核模块。

	kthread_create()几乎是整个驱动的灵魂，或者说是该linux内核模块的灵魂。




#23.传说中的URB
	usb_stor_acquire_resources(),从名字上来看是获取资源。不是专门为USB
	Mass Storage设备准备了一个struct us_date这么一个结构体。

	USB设备需要通信，要传递数据,就需要使用urb，确切说是USB设备驱动。实现上
	作为USB设备驱动，它本身并不能直接操纵数据的传输。在这个大观园里，
	外接设备永远都是配角，真正的核心只是USB Core,而真正负责调度的是USB主控制器
	这个通常看不见的USB主机控制器芯片。

	只需要知道，一个urb包含了执行USB传输所需要的所有信息。而作为驱动程序，要
	通信就必须创建这么一个数据结构。并且赋值，显然不同类型的传输，需要对urb
	赋不同的值。然后将她提交给底层。
	完了底层会找到相应的USB Core会找到相应的USb主机控制器，从而实现数据的传输
	传输完了之后，USB控制器会通知设备驱动。

	总之我们知道，784行就是调用usb_alloc_urb申请一个struct urb结构体。
	关于usb_alloc_urb()这个函数

	这个函数很明显，就是为一个urb结构体申请内存。它有两个参数，其中第一个
	iso_packets用来在等时传输方式下指定你需要传输多少个包，对于 非iso。而
	us的成员current_urb也是一个struct urb的指针，所以就赋值给它了。不过
	需要记住，usb_alloc_urb除了申请内存以外，还对结构体做了初始化，结构体
	urb被初始化为0。
		所以接下来我们就将要和us->current_urb打交道了。如果你对urb究竟怎么用
	还有些困惑的话，可以查看主机控制器驱动的代码。

	USB是一种总线，是总线它就要通信。我们现实生活中真正要使用的是设备，但是光
	有设备还不足以实现USB通信，于是世界上有了USb主机控制器，它来负责统一调度。

	那么设备和主机控制器的分工如何？
	在Linux中，设备驱动程序调用USB Core提供的函数，把这个urb传递给主机控制器，
	主机控制器就会把各个设备驱动程序所提交的urb统一规划，去执行每一个操作。而
	这期间，USB设备驱动程序通常会进入睡眠，而一旦主机控制器把urb要做的事情给做
	完了，它会调用一个函数去唤醒USB设备驱动程序。然后USB设备驱动程序就可以继续
	往下走了。


	Linux内核中引入了守护进程，也正是与这个传说对应，守护进程也叫内核精灵.
	调用kthread_create()函数，kthread_creadte(usb_stor_control_thread, us,
			"usb-storage").

	实际上,简单一点说，kthread_create()这么一执行呢？就会有两个进程，一个是
	父进程一个是子进程，子进程将会执行usb_stor_control_thread()之后，子
	进程就结束了，它会调用exit()函数退出.

	而父进程继续顺着usb_stor_acquire_resources函数往下走，kthread_create()
	函数对于父进程而言返回的是子进程的进程task_struct结构体的指针。IS_ERR(th)
	判断返回的指针是否是错误代码。
	于是分别跟踪父进程和子进程。
	唤醒子进程，之所以需要唤醒子进程，是因为当你用kthread_create()创建一个子
	进程后，它不会立即执行，它要等待到你唤醒之后才会执行。

	那我们来看看子进程，也就是usb_stor_control_thread()函数，这个函数定义于
	drivers/usb/storage/usb.c


#25
	usb_stor_control_thread()这个函数无疑是整个模块中最为精华的代码。我们只
	需要它中间306行的for(;;)


#26
	usb_stor_acquire_resources()函数中，返回了0。回到storage_probe函数中来
	scsi_add_host()函数被执行，之前申请的us->host被作为参数传递给它。同时
	intf->dev也被传递给给它。

	又一次见到kthread_create,不需要更多解释，这里自然还是创建一个内核守护进程
	只不过这次是usb_stor_scan_thread,而上次是usb_stor_control_thread.
	usb_stor_scan_thread()

	设置了delay_use为5，而module_param是Linux Kernel2.6
	storage_probe()函数的最初

	一旦进入睡眠，那么有三种情况：一种是wake_up或者wake_up_interrutible函数
	被另一个进程执行，从而唤醒它，第二是信号中断它，第三种就是刚才讲的超时，
	时间到了，自然就会返回。

	再来看父进程，也就是storage_probe(),在用kernel_thread()创建了usb_stor_scan
	_thread()，storage_probe()也走到了尽头。

#27
	跟着storage_probe()走完，所以U盘工作真正需要的四个模块，usbcore,
	scsi_mod, sd_mod, 以及咱们这里的usb-storage,其中sd_mod恰恰就是SCSI
	硬盘驱动程序。没有它，你的SCSI硬盘的驱动程序。没有它，你的SCSI硬盘
	就别想在Linux下面转起来。

#28
	下面讲讲usb_stor_control_thread()函数唤醒它的来自queuecommand
	而srb是来自scsi核心层在调用queuecommand时候传递进来的参数。聚集
	usb_stor_control_thread()314,dev_mutex这把锁我们必须在看完整个模块后
	再来从较高的角度来看。

	scsi命令。 INQUIRY
	查询设备的一些基本信息。




#29
#30
	在编写Linux设备驱动时，总是要涉及内存管理。
	所以usb_stor_access_xfer_buf()函数映入我们的眼帘

	scatter/gather 它是一种用高性能IO的标准技术。它通常意味着一种DMA传输方式
	，对于一个给定的数据块，它可以在内存中存在于一些离散的缓冲区










