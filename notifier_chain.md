# Linux内核基础--事件通知链(notifier chain)
##  内核通知链
## 一:概述
Linux内核中各个子系统相互依赖，当其中某个子系统状态发生改变时，就必须使用
一定的机制告知使用其服务的其他子系统，以便其他子系统采取相应的措施。
为满足这样的需求，内核实现了事件通知链机制。

通知链只能在各个子系统之间，而不能在内核和用户空间进行事件的通知。组成内核的
核心代码均位于kernel目录下，通知链表位于kernel/notifier中，对应的头文件为
include/linux/notifier.h,通知链表并不复杂。

事件通知链表是一个事件 处理函数的列表，每个通知链都与某个或某些事件有关，当
特定的事件发生时，就调用相应的事件通知链中的回调函数，进行相应的处理。

##二:数据结构
###1.原子通知链(Atomic notifier chains):
	通知链元素的回调函数(当事件发生时要执行的函数)在中断或原子操作上下文中
	运行，不允许阻塞。对应的链表头结构：

	struct atomic_notifier_head {
		spinlock_t lock;
		struct notifier_block *head;
		int priority;
	};

###2.可阻塞通知链(Blocking notifier chains):通知链元素的回调函数在进程上下文
###运行，允许阻塞。对应的链表头：
	struct Blocking_notifier_head {
		struct rw_semaphore rwsem;
		struct notifier_block *head;
	};

###3.原始通知链(Ram notifierchains):对于通知链元素的回调函数没有任何限制，所有
###锁和保护机制都由调用者维护。

###4.SRCU 通知链（SRCU notifier chains）:可阻塞通知链的一种变体。对应的链表头：
	struct srcu_notifier_head {
		struct  mutex mutex;
		struct srcu_struct srcu;
		struct notifier_block *head;
	};

###这些链中都是一个个notifier_block结构
	struct notifier_block {
		int (*notifier_call)(struct notifier_block *, unsigned long, void *);
		struct notifier_block *next;
		int priority;
	};

其中:
1.notifier_call:当相应事件发生时应该调用的函数，由被通知方提供，
2.notifier_block *next :用于连接成链表的的指针：
3.priority:回调函数的优先级，一般默认为0.
内核中代码中一般把通知链命名XXX_chain,
							XXX_nofitier_chain这种形式的变量名。
围绕核心数据结构notifier_block,内核中定义了这四种通知链类型。



##三:运行机理
通知链的动作机制包括两个角色：

###1.被通知者：
对某一事件感兴趣的一方。定义了当事件发生时，相应的处理函数，即
回调函数。但是需要事先将其注册到通知链中
**(被通知者注册的动作就是在通知链中增加一项)**

###2.通知者
事件的通知者。当检测到某个事件，或者本身产生事件时，通知所有对该事件感兴趣的一
方事件发生。他定义了一个通知链，其中保存了每一个被通知者对事件的处理函数。

包括以下:
1.通知者定义通知链。
2.被通知者向通知链中注册回调函数。
3.当事件发生时，通知者发出通知(执行通知链中所有元素的回调函数)。

被通知者调用notifier_chain_register函数通知注册回调函数，该函数按照优先级将
回调函数加入到通知链中：

static int notifier_chain_register(struct notifier_block **nl, 
								   struct notifier_block *n) 
{
	while ((*nl) != NULL)
	{
		if (n->priority > (*nl)->priority)
			break;
		nl = &((*nl)->next);
	}

	n->next = *nl;
	rcu_assign_pointer(*nl, n);

	return 0;
}

static int notifier_chain_unregister(struct notifier_block **nl, 
		struct notifier_block *n)
{
	while((*nl)!=NULL) {
		if((*nl) == n) {
			rcu_assign_pointer(*nl, n->next);

			return 0;
		}

		nl =&((*nl)->next);
	}
}

通知者调用notifier_call_chain函数通知事件的到达，这个函数会遍历通知链中所有元素
然后依次调用每一个`的回调函数(即完成通知动作):

static int __kprobes notifier_call_chain(struct notifier_block **nl, unsigned 
		long val, void *v, int nr_to_call,
		int *nr_calls)
{
	int ret = NOTIFY_DONE;
	struct = notifier_block *nb, *next_nb;

	nb = rcu_dereference(*nl);

	while(nb && nr_to_call) {
		next_nb = rcu_dereference(nb->next);

#ifdef 
		if (unlikely(!func_ptr_is_kernel_text(nb->notifier_call))) {

		}
#endif

	ret = nb->notifier_call(nb,val,v);
	if(nr_calls)
		(*nr_calls)++;

	if((ret & NOTIFY_STOP_MASK))
		break;
	nb = next_nb;
	nr_to_call--;
	}
	return ret;
}

参数nl是通知链的头部，val表示事件类型，v用来指向通知链上的函数执行时，需要
用到的参数，一般不用的通知链，参数类型也不一样，例如当通知一个网卡被注册时，
v就指向net_device结构，nr_to_call表示准备最多通知几个，-l表示整条链都通知，
nr_calls非空的话，返回通知了多少个。
每个被执行的notifier_block回调函数的返回值可能取值为以下几个：
1.NOTIFY_DONE
2.NOTIFY_OK
3.NOTIFY_BAD
4.NOTIFY_STOP
5.NOTIFY_STOP_MASK

Notifier_call_chain()把最后一个被调用函数的返回值作为它的返回值。

##四.举例应用:
在这里，写一个简单的通知链表的代码，实际上，整个通知链的编写也就是两个过程：
1.首先是定义自己的通知链的头节点，并将要执行的函数注册到自己的通知链中。
2.其次则是由另外一个的子系统来通知这个链，让其上面注册的函数运行

这里将第一个过程分成了两步来写，第一步是定义了头节点和一些自定义的注册函数
第二步则是使用自定义的注册函数注册了一些通知链节点。分别在代码buildchain.c
与regchain.c中。发送通知信息的代码为notify.c。

代码1 buildchain.c。它的作用是自定义一个通知链表test_chain,然后再自定义两个
函数分别向这个通知链中加入或删除节点，最后再定义一个函数通知这个test_chain链：


static RAW_NITIFIER_HEAD(test_chain);

/* 自定义的注册函数，将notifier_block节点加到刚刚定义的test_chain这个链表中来
 *  raw_notifier_chain_register会调用notifier_chain_register
 */
int register_test_notifier(struct notifier_block *nb)
{
	return raw_notifier_chain_register(&test_chain, nb);
}

int unregister_test_notifier(struct nontifier_block *nb)
{
	return raw_notifier_chain_unregister(&test_chain, nb);
}

/* 自定义的通知链表的函数，即通知test_chain指向的链表中的所有节点执行相应的函数
 */
int test_notifier_call_chain(unsigned long val, void *v)
{
	return raw_notifier_call_chain(&test_chain, val, v);
}
EXPORT_SYMBOL(test_notifier_call_chain);

/* init */
static int __init init_notifier(void)
{
	printk("init_notifier\n");
}
/* exit */
static int __exit exit_notifier(void)
{
	printk("exit_notifier\n");
}




regchain.c :该代码的作用是将test_notifier1 test_notifier2 test_notifier3
这三个节点加到之前定义的test_chain这个通知链表上，同时每个节点都注册一个函数


/* 注册通知链 */
extern int register_test_notifier (struct notifier_block *)
extern int unregister_test_notifier(struct notifier_block *)

static int test_event1(struct notifier_block *this, unsigned long event, void 
		*ptr)
{
	printk("INEVENT 1");
}


static int test_event2(struct notifier_block *this, unsigned long event, void 
		*ptr)
{
	printk("INEVENT 2");
}
	
static int test_event3(struct notifier_block *this, unsigned long event, void 
		*ptr)
{
	printk("INEVENT 3");
}


/* 事件1,该节点执行的函数为test_event1 */
static struct notifier_block test_notifier1 =
{
	.notifier_call = test_event1,
}

/* 事件2,该节点执行的函数为test_event2 */
static struct notifier_block test_notifier2 =
{
	.notifier_call = test_event2,
}


/* 事件3,该节点执行的函数为test_event3 */
static struct notifier_block test_notifier2 =
{
	.notifier_call = test_event3,
}

/* 对这些事件进行注册 */
static  int __init reg_notifier(void)
{
	err = register_test_notifier(&test_notifier1);
	err = register_test_notifier(&test_notifier2);
	err = register_test_notifier(&test_notifier3);
}


/* 卸载刚刚注册了的通知链 */
static void __exit unreg_notifier(void)
{
	register_test_notifier(&test_notifier1);
	register_test_notifier(&test_notifier2);
	register_test_notifier(&test_notifier3);
}






notify.c 该代码的作用就是向test_chain通知链中发送消息，让链中的函数运行:
extern int test_notifier_call_chain(unsigned long val, void *v);

/* 向通知链发送消息以触发注册了的函数 */
static int __init call_notifier(void)
{

	/* 调用自定义的函数 */
	test_notifier_call_chain(1, NULL); 
}












