
ARM Linux和Device Tree相关代码分析

本文主要内容是：以Device Tree相关数据流分析为索引，对ARM linux kernel的
代码进行解析。主要的数据流包括：

1.初始化流程。也就是扫描dtb并将其转换成Device Tree Structure.
2.传递运行时参数传递以及paltform的识别流程分析
3.如何将Device Tree structure并入 linux kernel的设备驱动模型。

二：如何通过Device Tree完成运行时参数传递以及platform的识别功能？ 
1.汇编部分代码分析
linux/arch/arm/kernel/head.S文件定义了bootloader和kernel的参数传递要求：
MMU = off , D-cache = off,I-cache = dont care, r0 = 0, r1 = machine nr,
	r2 = atags or dtb pointer.

目前的kernel支持旧的tag list的方式，同时也支持device tree方式。r2可能是
device tree binary file指针(bootloader要传递给内核之前要copy到memory中)
也可能是tag List的指针。
在ARM汇编部分的启动代码中(主要是head.s和head-common.s)
machine type ID和指向DTB或者atags的指针被保存在__machine_arch_type
和__atags_point中，这么做是为了后续c代码进行处理。


2.和device tree相关的setup_arch代码分析
具体的c代码都是在setup_arch处理，这个函数是一个总的入口点具体代码如下
```c
void __init setup_arch(char **cmdline_p)
{
	const struct machine_desc *mdesc;
	mdesc = setup_machine_fdt(__atgs_point);
	if(!mdesc)
		mdesc = setup_machine_tags(__atags_pointer,__machine_arch_type);
	machine_desc = mdesc;
	machine_name = mdesc->name;
}
```
对于如何确定HW platform这个问题，旧的方法是静态定义若干的machine描述符
(struct machine_desc),在启动过程中，通过machine type ID作为索引，在这些静态
定义描述中扫描，找到那个ID匹配的描述符.

在新内核中，首先使用setup_machine_fdt来setup machine描述符，如果返回NULL。
才使用传统的方法setup_machine_tags来setup machine描述符。传统的方法需要给出
__machine_arch_type(bootloader通过r1寄存器传递给kernel)
	和tag list的地址(用来进行tag parse)

3.匹配platform(machine描述符)

setup_machine_fdt函数的功能就是根据Device Tree的信息，找到最适合的machine
描述符，具体代码如下：


const struct machine_desc * __init setup_machine_fdt(unsigned int dt_phys)
{
	const struct machine_desc *madesc, *mdesc_best = NULL;
	
	if(!dt_phys || !early_init_dt_scan(phys_to_virt(dt_phys)))
		return NULL;

	mdesc = of_flat_dt_match_machine(mdesc_best, arch_get_next_mach);

	if(!mdesc) {
		出错处理
	}

	__machine_arch_type = mdesc->nr;
	return mdesc;
}

early_init_scan函数有两个功能，一个为后续的DTB scan进行准备工作，另一个是运行
时参数传递，具体请参考下面一个section的描述。

of_flat_dt_match_machine是在machine描述符列表中scan,找到最合适machine描述符，
我们首先看如何组成machine描述列表。和传统的方法类似，也是静态定义的。
DT_MACHINE_START和MACHINE_END用来定义一个machine描述符。
编译的时候，compiler会把这些machine descriptor放到一个特殊的段中，.arch.info.init
形成machine描述符的列表，machine描述用下面的数据结构来标识。
struct machine_desc {
	unsigned int nr; /* architecture number */
	const char *const *dt_compat; /* array of device tree 'compatible' string*/

	...
}

nr成员就是过去使用的machine type ID.内核machine描述符的table有若干个entry，每
个都有自己的ID。bootloader递machine type ID,指明使用哪个machine描述符。
目前匹配描述符使用compatible strings,也就是dt_compat成员，这是一个
string list.
定义了这个machine所支持的列表。在扫描machine描述符列表的时候需要不断的获取下一
个machine描述符的compatible字符串信息。具体的代码如下：

```c
static const void * __init arch_get_next_mach(const char *const **match) 
{ 
	static const struct machine_desc *mdesc = __arch_info_begin; 
	const struct machine_desc *m = mdesc;

	if (m >= __arch_info_end) 
		return NULL;

	mdesc++; 
	*match = m->dt_compat; 
	return m; 
}
```
__arch_info_begin指向machine描述符列表的第一个entry,通过mdesc++不断移动machine.
match返回了该machine描述符的compatible string list.
一个是root node的compatible字符串列表，一个是machin描述符的copatible字符串列表，
得分最低就是我们最终选定的machine type.


4.运行时参数传递
运行时参数是在扫描DTB的choose node时候完成，具体动作就是获取chosen node的bootargs
initrd等属性的value.并将其保存在全局变量
boot_command_line
initrd_start
initrd_end
使用tag_list方法是类似的。获取相关信息，保存在同样的全局变量中，具体代码位于
early_init_dt_scan函数中：

bool __init early_init_dt_scan(void *params)
{ 
	if (!params) 
		return false;

	/* 全局变量initial_boot_params指向了DTB的header*/ 
	initial_boot_params = params;

	/* 检查DTB的magic，确认是一个有效的DTB */ 
	if (be32_to_cpu(initial_boot_params->magic) != OF_DT_HEADER) { 
		initial_boot_params = NULL; 
		return false; 
	}

	/* 扫描 /chosen node，保存运行时参数（bootargs）到boot_command_line，此外，还处理initrd相关的property，并保存在initrd_start和initrd_end这两个全局变量中 */ 
	of_scan_flat_dt(early_init_dt_scan_chosen, boot_command_line);

	/* 扫描根节点，获取 {size,address}-cells信息，并保存在dt_root_size_cells和dt_root_addr_cells全局变量中 */ 
	of_scan_flat_dt(early_init_dt_scan_root, NULL);

	/* 扫描DTB中的memory node，并把相关信息保存在meminfo中，全局变量meminfo保存了系统内存相关的信息。*/ 
	of_scan_flat_dt(early_init_dt_scan_memory, NULL);

	return true; 
}

设定meminfo（该全局变量确定了物理内存的布局）有若干种途径：
1、通过tag list（tag是ATAG_MEM）传递memory bank的信息。
2、通过command line（可以用tag list，也可以通过DTB）传递memory bank的信息。
3、通过DTB的memory node传递memory bank的信息。
目前当然是推荐使用Device Tree的方式来传递物理内存布局信息。


三：初始化流程
在系统初始化的过程中，我们需要将DTB转换成节点的是device_node的树状结构,以便后续
方便操作。具体的代码位于setup_arch->unflatten_device_tree中。

void __init unflatten_device_tree(void)
{
	__unflatten_device_tree(initial_boot_params, &of_allnodes,
			early_init_dt_alloc_memory_arch);

	of_alias_scan(early_init_dt_alloc_memory_arch);
}



struct device_node { 
	const char *name;－－－－－－－－－－－－－－device node name 
		const char *type;－－－－－－－－－－－－－－－对应device_type的属性 
		phandle phandle;－－－－－－－－－－  －－－－对应该节点的phandle属性 
		const char *full_name; －－－－－－－－从“/”开始的，表示该node的full path

		struct    property *properties;－－－－该节点的属性列表 
		如果需要删除某些属性，kernel并非真的删除，而是挂入到deadprops的列表 
		struct    property *deadprops; 
		parent、child以及sibling将所有的device node连接起来 
		struct    device_node *parent;
		struct    device_node *child; 
		struct    device_node *sibling; 
		struct    device_node *next;  通过该指针可以获取相同类型的下一个node 
		通过该指针可以获取node global list下一个node 
		struct    device_node *allnext;
		struct    proc_dir_entry *pde;－开放到userspace的proc接口信息 
			struct    kref kref;－－－－－－－－该node的reference count 
			unsigned long _flags; 
		void    *data; 
};


unflatten_device_tree函数的主要功能就是扫描DTB,将device node被组织成：
1.global list 全局变量struct device_node *of_allnodes就是指向设备树的
global list.

2.tree
这些功能主要是在__unflatten_device_tree函数中实现。


		                                      －－－需要扫描的DTB 
static void __unflatten_device_tree(struct boot_param_header *blob,
		struct device_node **mynodes,－－－－－－－－－global list指针 
		void * (*dt_alloc)(u64 size, u64 align))－－－－－－内存分配函数 
{ 
	unsigned long size; 
	void *start, *mem; 
	struct device_node **allnextp = mynodes;

	此处删除了health check代码，例如检查DTB header的magic，
	确认blob的确指向一个DTB。

	/* scan过程分成两轮，第一轮主要是确定device-tree structure的长度，
	   保存在size变量中 */ 
	start = ((void *)blob) + be32_to_cpu(blob->off_dt_struct); 
	size = (unsigned long)unflatten_dt_node(blob, 0, &start, NULL, NULL, 0); 
	size = ALIGN(size, 4);

	/* 初始化的时候，并不是扫描到一个node或者property就分配相应的内存，
	   实际上内核是一次性的分配了一大片内存，
	   这些内存包括了所有的struct device_node、node name、
	   struct property所需要的内存。*/ 
	mem = dt_alloc(size + 4, __alignof__(struct device_node)); 
	memset(mem, 0, size);

	//用来检验后面unflattening是否溢出
	*(__be32 *)(mem + size) = cpu_to_be32(0xdeadbeef);  

	/* 这是第二轮的scan，第一次scan是为了得到保存所有
	   node和property所需要的内存size，第二次就是实打实的要构建
	   device node tree了 */ 
	start = ((void *)blob) + be32_to_cpu(blob->off_dt_struct); 
	unflatten_dt_node(blob, mem, &start, NULL, &allnextp, 0);  
	此处略去校验溢出和校验OF_DT_END。 
}
	具体的scan是在unflatten_dt_node函数中,如果已经清楚地了解DTB的结构

#四，如何并入linux kernel的设备模型
	在linux kernel引入统一的设备驱动模型之后，bus,driver和device形成了设备
	模型中铁三角。

	如果要并入linux kernel设备驱动模型，那么就需要根据device_node的树状结构
	(root是of_allnodes)将一个的device node挂入到相应的总线device链表中，只要
	做到这一点，总线机制就会安排device和driver匹配。

	当然不是所有的devicenode 都会挂入bus上设备链表，比如cpus node 
	memory node choose node等。

	1.cpus node的处理
	这部分的处理可以参数setup_arch-> arm_dt_init_cpu_maps

```c

```
	2.memory的处理
	这部分的处理可以参考setup_arch->setup_machine_fdt->early_init_dt_scan->
	early_init_dt_scan_memory.

	3.interrupt controller的处理
	初始化是通过start_kernel->init_IRQ->machine_desc->init_irq()
	我们用s3c2416为例来描述interrupt controller的处理过，下面machine描述符定义




	DT_MACHINE_START(S3C2416_DT, "Samsung S3C2416 (Flattened Device Tree)") 
	…… 
	.init_irq    = irqchip_init, 
	…… 
	MACHINE_END

	在driver/irqchip/irq-s3c24xx.c文件中定义了两个interrupt controller,如下：

	IRQCHIP_DECLARE(s3c2416_irq, "samsung,s3c2416-irq", s3c2416_init_intc_of);
	IRQCHIP_DECLARE(s3c2410_irq, "samsung,s3c2410-irq", s3c2410_init_intc_of);

	当然，系统中可以定义更多的irqchip，不过具体用哪个根据DTB中interrupt
	controller node中的compatible属性确定的。

	__irqchip_begin就是所有irqhcip的一个列表，of_irq_init函数是遍历Device Tree
	找到irqchip具体代码如下：

	只有该node中有interrupt-controller这个属性定义，那么linux kernel就会分配
	一个interrupt controller的描述符（struct intc_desc）并挂入队列。
	通过interrupt-parent属性，可以确定各个interrupt controller的层次关系。
	在scan了所有的Device Tree中的interrupt controller的定义之后，
	系统开始匹配过程。一旦匹配到了interrupt chip列表中的项次后，
	就会调用相应的初始化函数。如果CPU是S3C2416的话，匹配到的是
	irqchip的初始化函数是s3c2416_init_intc_of。

	ok我们通过compatible属性找到了适合的interrupt controller，那么如何解释reg
	属性？我们知道，对于s3c2416的intertupt controller而言。其#interrupt-cells
	的属性值是4，定义为<ctrl_num type ="" ctrl_irq ="" parent_irq ="">

	1）ctrl_num表示使用哪一种类型的interrupt controller，其值的解释如下：

	- 0 ... main controller 
	- 1 ... sub controller 
	- 2 ... second main controller

	（2）parent_irq。对于sub controller，parent_irq标识了其在main controller的bit position。

	（3）ctrl_irq标识了在controller中的bit位置。

	（4）type标识了该中断的trigger type，例如：上升沿触发还是电平触发。

	53个分为两类
	一种：需要sub寄存器进行控制的。
	另一： 不需要sub eg：timer

	不过随着硬件的演化，更多的HW block加入到SOC中，这使中断源不够用了，因此
	中断寄存器又被分成了两个group,一个是group 1
	另一个是group2

	对于s3c2416而言,irqchip初始化函数是s3c2416_init_intc_of
	s3c2416_ctrl作为参数传递给了s3c_init_intc_of
	大部分处理都 是在s3c_init_intc_of函数中完成的由于这个


4.GPIO controller的处理

5.machine初始化

	machine初始化的代码可以沿着start_kernel->rest_init->kernel_init->
	kernel_init_freeable->do_basic_setup->do_initcalls路径寻找。
	在do_initcalls函数中，kernel会依次执行各个initcall函数。在这个
	过程中,会调用customize_machine，具体如下：

	在这个函数中，一般会调用 machine描述中的init_machine callback函数来把各
	种Device Tree中定义各个设备节点加入到系统。如果machine描述符中没有定义
	init_machine函数。那么直接调用of_platform_populate把所有的platform device
	加入到kernel中。

	由此可见，最终生成platform device 的代码来自of_paltform_populate函数。
	该函数逻辑比较简单，遍历device node global list中所有的node，并调用
	of_paltform_bus_create处理。 


	具体增加platform device的代码在of_platform_device_create_padata代码如下

	of_device除了分配struct plat_device的内存，还分配了该platform device需要的
	resource的内存.当然,这就需要解析该device node的interrupt资源以及memory 
	address资源.

	of_device_add(dev)把这个platform device加入统一的设备模型系统中。














