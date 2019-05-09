
# 关于内核中调用的说明
subsys_initcall(fn)		__define_initcall(fn, 4)


#define __define_initcall(fn, id) \
	static initcall_t __initcall_##fn##id __used \
	__attribute__((__section__(".initcall" #id ".init"))) = fn

最终我们看到的是把这个函数定义到.initcall6.init里面。
这函数躲在这里干嘛，找到有此字符串的文件vmlinux.lds.h,相关代码如下所示：
#define INITCALLS
.initcall6.init

字符串.init6.init夹杂在这个宏的第n行，具体自己数，他们挨个挨个有顺序
的组成一个整体，此整理又构成一个大写字母写的宏INITCALLS。


他在一个偏僻的vmlinux.lds.s里面！这是一个汇编文件！
INITCALLS的使用场景。


__initcall_start = .;
	INITCALLS



   …………省略一大段………….

	__initcall_start = .;

     INITCALLS

    __initcall_end = .;

    __con_initcall_start = .;

       *(.con_initcall.init)

    __con_initcall_end = .;

    __security_initcall_start = .;

       *(.security_initcall.init)

    __security_initcall_end = .;

    …………省略一大段……………

INITCALLS在__initcall_start = .与__initcall_end = .之间，表示INITCALLS
宏内涵的相关代码段顺序存放在这里，modeule_init所代表段的镶嵌其中，等候
挨个被访问。

内核启动流程如下所示
Main() 
linux_main
start_uml
start_kernel_proc
start_kernel
reset_init
kernel_init
do_+basic_setup
do_initcalls

do_initcalls的代码如下


根据下表，分别调用 do_one_initcall来执行第一个initcall
static initcall_entry_t *initcall_levels[] __initdata = {
	__initcall0_start,
	__initcall1_start,
	__initcall2_start,
	__initcall3_start,
	__initcall4_start,
	__initcall5_start,
	__initcall6_start,
	__initcall7_start,
	__initcall_end,
};

表中的 xxx_start恰恰就是前面所提到的存储标识，那么这些调用就联系到了一起，
实现函数的调用。


