#进程地址空间也就是每个进程使用的内存,内核对进程地址空间的定理，也就是对用户态程序的内存管理。
#***主要内容***: 
**地址空间**
**虚拟内存区域(VMA)**
**地址空间和页面**


补充说明：上面的属性中，mm_users和mm_count很容易混淆，这里特别说明一下：
-- mm_users比较好理解，就是mm_struct被用户空间进程(线程)引用的次数.
	如果进程A中创建了3个进程，那么进程A(这时候叫线程A也可以)对应的mm_struct中的 mm_users = 4;

	补充一点，linux中线程和进程和线程几乎没有什么区别，就是看它是否共享进程地址
	空间,共享地址空间就是线程，反之就是进程。
	所以，如果子进程和父进程共享了进程地址空间，那么父子都可以看做线程。如果父
	子没有共享进程地址空间，那就是2个进程。

	mm_count则稍微有点绕人，其实它记录就是mm_struct实际引用计数。
	简单点说，当mm_users = 0时;并不一定能释放mm_struct,只有当mm_count = 0时，才
	确认释放mm_struct

	从上面的解释可以看出，可能引用mm_struct的并不只是用户空间的进程 
	当mm_users > 0时，mm_count会增加1，表示用户空间进程(线程)在使用mm_struct,
	不管使用mm_struct的用户进程有几个，mm_count都只是增加1。

	也就是说，如果只有一个进程使用mm_struct,那么mm_users = 1, mm_count也是1.
	如果有9个进程使用，那么mm_users = 9，而mm_count仍然为1。

	那么什么情况mm_count大于1？
	当有内核线程使用mm_struct时，mm_count才会增加1.
	在下面这种情况下，mm_count就很有必要了：
	* - 进程A启动，并申请了一个mm_strcut  mm_users = 1,mm_count = 1
	* - 进程A新建了2个线程，此时mm_users = 3, mm_count = 1
	* - 内核调度发生，进程A及相关线程都被挂起，一个内核线程B 使用了A申请的mm_strcut
	此时 mm_users = 3,mm_count = 2.
	* - CPU的另一个core调度了进程A及其线程，并执行完了进程A 申请的mm_struct,
	此时mm_users = 0,mm_count = 1
	* -在这里就看出mm_count的用处了，如果只有mm_users的话，这里mm_users = 0就会
	释放mm_struct,从而有可能导致内核B线程异常。
	* -内核线程B执行完成后退出，这时mm_users = 0, mm_count = 0,可以完全释放
	mm_struct了.
	




补充说明2：为何内核线程会使用用户空间的mm_struct?
对Linux来说，用户进程和内核线程都是task_struct的实例，
唯一的区别是内核线程是没有进程地址空间的（内核线程使用的内核地址空间），内核
线程的mm描述符是NULL，即内核线程的tsk->mm域是空（NULL）。

内核调度程序在进程上下文的时候，会根据tsk->mm判断即将调度的进程是用户进程还是
内核线程。但是虽然内核线程不用访问用户进程地址空间，但是仍然需要页表来访问自己
的空间。

而任何用户进程来说，他们的内核空间是100%相同的，所以内核会借用上一个被调用的
用户进程的mm_strcut中页表来访问内核地址，这个mm_struct的页表来访问内核空间。
对于用户进程，tsk->mm == tsk->active_mm.


补充说明3：除了mm_users和mm_count之外，还有mmap和mm_rb需要说明以下：
其实mmap和mm_rb都是保存此进程地址空间中所有的内存区域。前者心链表形式存放，
后者以红黑树形式存放。用2种数据结构组织同一种数据是为了便于对VMA进行高效的操作。


3.地址空间和页表
地址空间中的地址都是虚拟内存中的地址，而CPU需要操作的是物理内存，所以需要将虚拟 
地址映射到物理地址的机制。

这个机制就页表，linux中使用3级页面来完成虚拟地址到物理地址的转换。
1.PGD 全局页目录 ，包含一个pgd_t类型数组，多数体系结构中pgd_t类型就是一个无符号
长整型

2.PMD 中间页目录，它是pmd_t类型数组

3.PTE-简称页表，包含一个pte_t类型页表项，该页表项指向物理页面

虚拟地址 - 页表 - 物理地址的关系如下图：
(虚拟地址 - 物理地址映射的关系如下图)

                          PGD       PMD      PTE


struct mm_struct -------- pgd_t     pmd_t    pte_t  页面结构  物理页面



 
#/*--------------------------------------------------------------------------*/
十六  页高速缓存和页回写

主要内容：
* - 缓存简介  
* - 页高速缓存  
* - 页回写 

1.缓存简介
在编程中，缓存是很常见也很有效的一种提高程序性能的机制.
Linux内核也汪例外，为了提高I/O性能，也引入了缓存机制，即将一部分磁盘上的数据
缓存到内存中.

1.1 原理
之所以通过缓存能提高I/O性能是基于2个重要的原理：
1.CPU访问内存的速度远远大于访问磁盘的速度(有几个数量级的差异)
2.数据一旦被访问，就有可能在短期内再次被访问


1.2策略
缓存的创建和读取没有什么好说的。但是写缓存和缓存回收就需要好好考虑了。这里面涉及
到了缓存内容和磁盘内容同步的问题。
1.2.1 写缓存有3种策略

*不缓存(nowrite)::也就是不缓存写操作，当对缓存中的数据进行写操作时，直接写入
磁盘，同时使此数据的缓存失效。

*写透（write-through)::写数据时同时更新和缓存
*回写（copy-write or write-behind）::写数据时直接写到缓存，由另外的进程在合适的
时候将数据同步到磁盘(目前内核中采用的方法)

1.2.2 缓存回收的策略
* - 最近最少使用（LRU）
* - 双链策略::基于LRU的改善策略。具体见下面的补充说明。 

补充说明(双链策略):
	双链策略其实主浊LRU算法的改进版本。
	它通过2个链表(活跃链表和非活跃链表)来模拟LRU过程，目的是为了提高页面回收的
	性能。 
	页面回收动作发生时，从非活跃链表的尾部开始回收页面。


双链策略的关键就是页面如何在2个链表之间移动的。
双链策略中，每个页面都有2个标志位，分别为
PG_active - 标志页面是否活跃，也就是此页面是否要移动到活跃链表
PG_referenced - 表示页面是否被进程访问到


页面移动的流程如下：
1.当页面第一次被访问时，PG_active置为1，加入到活动链表
2.当页面再次被访问时，PG_referneced置为1，此时如果页面在非活动链表，则将其移动
到活动链表，并将PG_active置为1，PG_referenced置为0
3.系统中daemon会定时扫描活动链表，定时将页面的PG_referenced位置为0
4.系统中daemon定时检查页面的PG_referenced，如果PG_referenced = 0,那么将此页面的
PG_active置为0，同时将页面移动到非活动链表


2.页高速缓存
故名思义,页高速缓存中缓存的最小单元就是内存页。
但是此内存页对应的数据不仅仅是文件系统的数据，可以是任何基于页的对象，包括各种
类型的文件和内存映射。

2.1 简介
页高速缓存缓存的是具体的物理页面，与前面虚拟内存空间不同，假设有进程创建了多个
vm_area_struct都指向同一个文件，那么这个vm_area_struct对应的 页高速缓存只有一份。
也就是磁盘上的文件缓存后，它的虚拟内存地址可以有多个，但是物理内存地址却只有一个。

为了有效提高I/O性能，页高速缓存要需要满足以下条件：
1.能够快速检索需要的内存页是否存在
2.能够快速定位 脏页面（也就是被写过，但没有同步到磁盘上的数据)
3.页高速缓存被并发访问时，尽量减少并发锁带来的性能损失
下面通过分析内核中相应的结构体，来了解内核是如何提高I/O性能的


2.2 实现 
	实现页高速缓存的最重要的结构体要算是address_space,在<linux/fs.h>中

补充说明 ：
1.inode - 如果address_space是由不带inode的文件系统中的文件映射的话，此字段为
null
2.page_tree - 这个树结构很重要，它保证了页高速缓存中数据能被快速检索到。
脏页面能够快速定位。
3.i_mmap - 根据vm_area_struct,能够快速找到关联的缓存文件（即address_space),前
address_space 和 vm_area_struct是一对多的关系
4.其他字段主要是提供各种锁和辅助功能

此外，对于这里出现的一种新的数据结构，基数树radix,进行简要的说明. 
radix树通过long型的位操作来查询各个节点，存储效率高，并且可以快速查询。
Linux中radix树相关的内容参见：include/linux/radix-treeelh
下面根据我自己的理解，简单的说明一下radix树结构及原理。

2.2.1 首先是radix树节点的定义
struct radix_tree_node {
	unsigned int height;
	unsigned int height;
	struct rcu_head rcu_head;
	void *slots[RADIX_TREE_MAP_SIZE]
	unsigned long tags[RADIX_TREE_MAX_TAGS][RADIX_TREE_TAG_LONGS];
};

弄清楚radix_tree_node中各个字段的含义,也就差不多知道radix树是怎么一回事了。
*height 表示树的调度
*count 当前的节点的子节点数
*rcu_head RCU发生时触发的回调函数链表
*slots 每个slot对应一个子节点
*tags 标记子节点是否dirty或者writeback


2.2.2 每个叶子节点指向文件内相应偏移所对应的缓存页

2.2.3 radix tree的叶子节点都对应一个二进制的整数，不是字符串，所以进行比较
的时候非常快
其实叶子节点的值就是地址空间的值(一般是Long型)



3.页回写
由于目标linux内核中对于写缓存采用的是第3种策略，所以回写的时机就显得非常重要，
回写太少容易造成数据丢失。

3.1 简介
linux页高速缓存中的回写是由内核中的一个线程(flusher 线程)来完成的，flusher线程
在以下3种情况情况发生时，触发回写操作。

1.当空闲内存低下于一个阀值时
	空闲内存不足时，需要释放一部分缓存，由于只有不脏的页面才能被释放，所以要把脏
页面都回写到磁盘，使其变成干净的页面。

2.当脏页在内存驻留时间超过一个阀值

3.当用户进程调度sync()和fsync()系统调度时



页回写汲及一些阀值可以在/proc/sys/vm中找到
下表中列出的是与pdflush相关的一些阀值


3.2 实现 
flusher线程的实现方法随着内核的发展也在不断的变化着。下面介绍几种在内核发展中
出现的比较典型的实现方法。

1.膝上型计算机模式
2.bdflush和kupdated

bdflush问题存在的问题：
整个系统仅仅只有一个bdflush线程，当系统回写任务较重时，bdflush线程可以会阻塞在
某个磁盘的I/O上，导致其他磁盘的I/O回写操作不能及时执行。

3.pdflush 线程数目是动态的，取决于系统的I/O负载。它是面向系统中所有磁盘的全局
任务。

Pdflush存在的问题：
pdflush的数目是动态的，一定程序 上缓解了bdflush的问题，但是由于pdflush是面向
所有磁盘的。所以有可能出现多个pdflush线程全部阻塞在某个拥塞的磁盘上，同样导致
其他磁盘的I/O回写不能及时执行。

4.flusher线程
flusher线程改善了上面出现的问题：
首先，flusher线程的数目不是唯一的，这就避免了bdflush线程问题
其次，flusher线程不是面向所有磁盘的，而是每个flusher线程对应一个磁盘，这就避免了
pdflush线程的问题

#/*--------------------------------------------------------------------------*/
	
主要内容：
设备类型:
内核模块:
内核对象:
sysfs:

总结

1.设备类型
2.内核模块
2.1 内核模块示例
  内核模块可以带参数也可以不带参数，不带参数的内核模块比较简单。
  我之前的几篇随笔中用于测试的例子都是用不带参数的内核模块来实验的。


2.1.1 无参数的内核模块
2.1.2 带参数的内核模块

构造带参数的内核模块其实也不难，内核中已经提供了简单的框架来给我们声明参数
.参数name::既是用户可见的参数名，也是模块中存放模块参数的变量名
.参数type::参数的类型（byte，short,int,unit,ulong,charp,bool..)byte型存放
在char变量中，bool型存放在int变量中
.参数perm::指定模块在sysfs文件系统中对应的文件权限(关于sysfs内容后面介绍)

	static int stu_id = 0;
	module_param(stu_id, int, 0644);


2.module_param_named(name, variable, type, perm);

3.module_param_string(name, string, len, perm);  拷贝字符串到指定的字符数

4.module_param_array(name, type, nump, perm)   定义数组类型的模块参数

5.module_param_array_named(name, array,type,nump,perm)   定义数组类型的模块参数
 
6.参数描述宏
可以通过MODULE_PARM_DESC()给内核模块的参数添加一些描述信息。
这些描述信息在编译完成内核模块后，可以通过modinfo命令查看。

static int stu_id = 0;
module_param(stu_id, int, 0644);
MODULE_PARM_DESC(stu_id, "学生ID, 默认为0");


7.带参数的内核模块的示例
示例代码： test_paramed_km.c
定义了3个内核模块参数，分别是int型，char*型，数组型。



2.2 内核模块的位置
2.2.1 内核代码外
上面的例子都是把内核模块代码放在内核之外来运行的。

2.2.2 内核代码模块
内核模块代码也只可以直接 放在内核源码树中。
如果你开发一种驱动，并且希望被加入到内核中，那么可以在编写驱动的时候

2.3 内核模块相关操作
2.3.1 模块安装
make modules_install <---- 把随内核编译出来的模块安装到合适的目录中

2.3.2 模块依赖性

2.3.3 模块的载入
insmod module.ko





2.3.4 模块的卸载

2.3.5 模块导出等号表



3.内核对象 
2.6内核中增加了一个引人注目的新特性--统一设备设备型（DEVICE model)
统一设备模型最初的动机是为了实现智能的电源管理，linux为了实现智能电源管理，
需要建立表示系统中所有设备拓扑关系的树结构，这样在关闭电源时，可以从树的节
点开始关闭。

实现了统一设备模型之后，还给内核带来了如下好处：

1.代码重复最小化(统一处理的东西多了)
2.可以列举系统中所有设备，观察它们的状态，并查看它们连接的总线
3.可以将系统中的全部设备以树的形式完整
4.可以设备按照和其对应的驱动联系起来，反之亦然
5.可以将设备按照类型加以归类，无需理解物理设备的拓扑结构
6.可以沿设备树的叶子向其根的反向依次遍历，以保证能以正确的顺序关闭电源



3.1 kobject简介
统一设备模型的核心部分就是kobject，通过下面对kobject结构体的介绍，可以大概
了解它是如何使得各个物理设备能够以树结构的形式组织起来。



3.1.1 kobject 定义在<linux/kobject.h>中
struct kobject{
	...
}

kobject本身不代表什么实际内容，一般都是嵌在其他数据结构中来发挥作用
(感觉有点像内核数据结构链表的节点)

比如linux/cdev.h中的struct cdev(表示字符设备的struct)

struct cdev {
	struct kobject kobj; /* 嵌在cdev中的kobject */
}

cdev中嵌入了kobject之后，就可以通过 cdev-> kboj.parent建立cdev之间的层次关系 ，
通过 cdev->kobj.entry获取关联的所有的cdev设备等。
总之，嵌入了kobject之后，cdev设备就有了树结构关系，cdev设备和其他设备之间也有层
层次关系 








3.1.2
ktype的定义很简单，参见<linux/kobject.h>
struct kobj_type{
	void (*release)(struct kobject *kobject *kobj);
	struct sysfs_ops *sysfs_ops;
	struct attribute **default_attrs;
}

3.1.3 kse`
kset是kobject对象的集合体，可以所有相关的kobject置于一个kset之中，
比如所有的块设备可以放在一个表示块设备的kset中。


3.1.4 kobject, ktype和kset之间的关系 
在这3个概念中，kobject是最基本的，kset和ktype是为了将kobjcet进行分类，以便将
共通的处理集中处理，从而减少代码量，也增加维护性。
这里kset和ktype都是为了将kobject进行分类，为什么会有2种分类呢？

从整个内核的代码来看，如果把kobject比作一个人的话，kset相当于一个一个国家，
ktype则相当于人种

人种的类型少数几个，但是国家确有很多。
ktype侧重于描述，kset侧重于管理。



3.1.5 kref
kref记录kobject被引用的次数，当引用计数降到0的时候，则执行release函数释放相关
资源。



3.2 kobject操作
kobject的相关都在<linux/kobject.h>


4.sysfs
sysfs是一个处于内存中的虚拟文件系统，它提供了kobject对象层次结构的视图。
可以用下面这个命令来查看/sys的结构
tree -L 1 /sys


4.1.1 sysfs中的添加和删除kobject非常简单，就是上面介绍的kobject操作中提到

添加了kobject之后，只会增加文件夹，不会增加文件，
因为kobject在sysfs就是映射成一个文件夹。



4.1.2 sysfs中添加文件
kobject是映射成sysfs中的目录，那sysfs中的文件是什么呢？
其实sysfs中的文件就是kobject的属性，属性的来源有2个：
+ 默认属性::kobject所关联的ktype中的default_attrs字段
默认属性default_attrs的类型是结构体struct attribute

struct attribute {
	const char *name; /* sysfs文件中的文件名 */
	struct module      *owner;/* x86体系结构中已经不再继续使用了，可能在其它
							 结构中还会使用 */
	mode_t mode;      /* sysfs中该文件的权限 */

};

default_attrs字段描述了sysfs中的文件，还有一个字段sysfs_ops则描述如何使用默认属性

struct sysfs_ops{
	/* 在读sysfs文件时该方法被调用 */
	ssize_t  (*show)(struct kobject *kobj,struct attribute *attr, char *buffer
		)
	ssize_t  (*store)(struct kobject *kobj,struct attribute *attr, char *buffer
			}
};

show方法在读取sysfs中文件时调用，它会拷贝attr提供的属性到buffer指定的缓冲区
store方法在写sysfs中文件时调用 ，它会从buffer读取size字节的数据到attr





4.2 基于sysfs的内核事件
内核事件也是利用kobject和sysfs来实现的，用户空间通过监控sysfs中kobject
的属性的变化来异步捕获内核中kobject发现的信号


用户可以通过一种netlink的机制来获取内核事件
内核空间向用户空间发送信号使用kobject_uevent()


5.kobject加sys带来了好处





