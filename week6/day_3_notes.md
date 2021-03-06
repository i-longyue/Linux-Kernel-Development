第18章 调度
	从调试内核的一种可能步骤开始

18.1 准备开始
	如果能重现一个bug,就基本成功一半了.

18.2 内核中的bug
	1.明白无误的代码
	2.同步时发现的错误
	3.错误的控制硬件
	4.从降低所有程序运行性能到毁坏数据到死锁状态

	例子:一个被共享的结构,如果没有引用计数.可能在一个进程是使用后释放后,然后
	第二个进程使用,就引用了空指针.引用空指针会导致产生一个OOPS,而垃圾数据可能
	会导致系统GG.

	内核确实有一些独特的问题需要考虑,像定时器限制和竞争条件等,它们都是允许多个
	线程在内核同时运行产生的结果.

18.3 通过打印来调试
	内核提供的打印函数是printk()和C库提供printf()函数功能几乎相同.

18.3.1 健壮性
	健壮性是printk()函数最容易让人们接受的一个特质.任何时候,任何地方都能调用它.

	在系统启动过程中,终端还没有初始化之前,在某些地方不能使用它.
	除非你要调度的是启动过程最开始的那些步骤,着手进行这样的调试挑战性很强.不过
	还是有一些可以指望的.核心硬件部分的依靠此时能够工作的硬件设备与外界通信.

	解决的方法是提供一个print()变体函数----early_printk(),这个函数在在一些内核
	支持的硬件体系结构上无法实现.

18.3.2 日志等级
    printk和printf()在使用上最主要的区别就是前者可以指定一个日志级别,内核根据这个
	级别来判断是否在终端上打印信息.内核把级别比某个特定值低的所有消息显示在终端上.

	KERN_EMERG
	KERN_ALERT
	KERN_CRIT
	KERN_ERR
	KERN_WARNING
	KERN_NOTICE
	KERN_INFO

	KERN_DEBUG

	如果没有指定一个记录等级,函数会选用默认的DEFAULT_MESSAGE_LOGLEVEL,现在默认等级
	是KERN_WARNING.由于这个默认值将业存在变化的可能性,所以指定一个等级.

	内核将最重要的记录等级KERN_EMERG定为<0>
	将无关紧要的记录等级  KERN_DEBUG定为<7>

	怎样调用的printk()等级完全取于自己.

18.3.3 记录缓冲区
	内核消息都被保存在一个LOG_BUF_LEN大小的环形队列中.该缓冲区大小可以在编译时通过
	设置CONFIG_LOG_BUF_SHIFT进行调整.在处理器系统上默认值是16KB.
	环形缓冲区的唯一缺点----可能会丢失浙消息,但是与简单性和健壮性的好处相比,这点
	代价是值得的.

18.3.4 syslogd和klogd
	标准的Linux系统上,用户空间的守护进程klogd从记录缓冲区中获取内核消息,再通过syslogd
	守护进程将它们保存在系统日志文件中.klogd程序既可以从/proc/kmsg文件中,也可以通过
	syslog()系统调用读取这些消息.
	默认情况下,它选择读取/proc方式实现.不管是哪种方法,klogd都会阻塞,直到有新的内核消息
	可供读出.在被唤醒之后,它会读取出新的内核消息并进行处理.默认情况下,它就是把消息传给
	syslogd守护进程.
	syslogd守护进程把它接收到的所有消息添加进一个文件中,该文件默认是/var/log/messages.
	也可以通过/etc/syslog.conf配置文件重新指定.
	在启动klogd的时候,可以通过指定-c标志来改变终端的记录等级.

#18.3.5 从printf()到printk的转换
> 
#18.4 OOPS
> **OOPS是内核告知用户有不幸发生的最常用方式**
> 由于内核是整个系统的管理者,所以它不能采取像在用户空间出现运行错误时使用的那些简单手段
> 因为它很难自行修复,它不能将自己杀死.内核只能发布OOPS.输出**寄存器**中保存的信息并输出
> 可供跟踪的回溯线索.

> 如果OOPS发生在中断上下文,内核根本无法继续,它会陷入混乱.混乱的结果就是系统死机.
> 不过,如果OOPS基其他进程运行时发生,内核就杀死该进程尝试继续执行.

回溯线索显示了导致错误发生的函数调用链.这样我们就可以观察究竟发生了什么:

#18.4.1 ksymoops
> 前面列举的OOPS可以说是一个经过解码的OOPS因为,内存地址转化成了对应的函数
> 回溯线索中的地址需要转化成有意义的符号名称才方便使用.这需要调用ksymoops
> 命令,并且还必须提供编译内核时产生的System.map.如果使用的是模块,还需要一些
> 模块信息.ksymoops通常会自行解析这些信息,所以一般可以这样调用它:
	ksymoops saved_oops.txt
	该程序就会吐出解码版的OOPS,如果ksymoops无法找到默认位置上的信息,或者
	想提供不同信息,该程序可以接受许多参数.

#18.4.2 kallsyms
> 开发版2.5版本引入了kallsyms特性,它可以通过定义CONFIG KALLSYMS配置选项启用. 
> 该选项存放着内核镜像中相应函数地址的符号名称,所以内核可以打印解码好的跟踪线索


#18.5 内核调度配置选项
	在编译的时候,在内核开发(Kernel hacking)它们都依赖于CONFIG_DEBUG_KERNEL.
	当开发内核的时候,作为一种练习,不妨打开所有这些选项.

	slab layer debugging 
	high-memory debugging
	I/) mapping debugging
	spin-lock debugging (自旋锁调试选项)
	stack-overflow checking(栈溢出检查选项)
	sleep-inside-spinlock checking(自旋锁睡眠选项)


#18.6 引发bug并打印信息
	一些内核调用提供断言并输出信息.最常用的两个是BUG()和BUG_ON().当被调用的时候,它们会
	引发OOPS,导致栈的回溯错误信息的打印.

	多数内核开发者相信BUG_ON比BUG更清晰和可读.
	panic()引发更严重的错误.调用panic()不但会打印错误消息,而且还会挂起整个系统.

	有些时候需要在终端上打印一下栈的回溯信息来帮助调试.
	*dump_stack()*

#18.7 神奇的系统请求键
	该功能通过定义CONFIG_MAGIC_SYSRQ配置选项来启用.SysRq(系统请求)键在大多数据键盘上都是
	标准键盘.
	echo 1 > /proc/sys/kernel/sysrq
	从终端上,你可以输入Sysrq-h获取一份可用的选项列表.



#18.8 内核调试器的传奇
##18.8.1 gdb
	gdb vmlinux /proc/kcore
	p global_variable
	反汇编一个函数
	disassemble function
	如果在编译内核的时候使用了-g参数,gdb还可以提供更多的信息,比如,你可以打印出结构体中
	存放的信息或是跟踪指针.

##18.8.2 kgdb
	kgdb是一个补丁,它可以让我们在远端主机上通过串口利用gdb的所有功能对内核进行调试.这需要
	两台计算机:一台有kgdb补丁的内核,第二台通过串行线使用gdb对第一台进行调试.

#18.9 探测系统
	通过探测系统从而找到想要的答案.

##18.9.1 用UID作为选择条件
	if(current->uid != 7777) {
		/* 新算法 */
	} else{
		/* 新算法 */
	}
除了UID为7777以外,其他所有的用户用的都是用的老算法.可以创建一个UID为7777的用户,专门来测试
新算法.对于要求很严格的进程相关部分的来说.

##18.9.2 使用条件变量
	创建一个全局变量作为一个条件开关.如果该变量为零,就使用一个分支上的代码.如果它不是零就
	选择另外一个分支


##18.9.3 使用统计量
	有些时候你需要掌握某个特定事件发生规律.有些时候需要比较多个事件并从中得出规律.通过统计

***量并提供某种机制访问其统计结果,很容易就能满足这种需求***

##18.9.4 *** 重复频率限制 ***
*为了发现一个错误*,开发者往往在代码的某个部分加入很多错误检查语句,在内核中,有些函数每秒都要
调用很多次.如果你在这样的函数中加入了printk()那么系统马上就会被显示调度信息这一个任务压得喘
不过气来,
有两种相关的技巧可以防止此类问题.
1.在代码内设置
2.另一种是限制输出次数
> 不管上面提到的那个示例,用到的变量都应该是静态(static)的,并且应该限制在函数的
局部范围以内,这样才能保证变量的值仍然能够保留下来.
> 这些例子的代码都不是SMP或抢占安全的.不过,只需要用原子操作改造一下就没问题了.
不过,对于一个临时的调试检测来说,没必要搞得这么复杂

#18.10 用二分法找出引发罪恶的变更
#18.11 使用Git进行两分搜索
#18.12 当所有的努力都失败时:社区 
#18.13 小结
	本章讨论了内核的调试----调试过程其实是一种寻求实现与目标偏差的行为.我们考察
	了几种技术:从内核内置的调度架构到调度程序,从记录日志到用git二分查找.因为调试
	Linux内核困难重重,非调试用户程序能比,因此,本章的资料对于试图在内核供其以中小
	试牛刀,第19章涉及另外的话题:Linux内核的可移植性,不要止步.
	
	


	

