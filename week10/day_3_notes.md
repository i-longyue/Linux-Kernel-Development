
#内核版本： Linux-3.18.2
在3.x版本内核中paltform_device不再静态定义，而是通过device tree来动态生成，例如
(arch/arm/mach-s3c24xx/mach-sc2416-dt.c)

1.我想从三个方面来了解Device Tree,这个机制是用来解决什么问题的?
2.Device Tree的基础概念(请参考DT基础概念)
3.ARM linux中Device Tree相关代码分析(请参考DT代码分析)

本文要从下面几个方面阐述为何ARM linux会引入Device Tree:
1.没有Device Tree的ARM Linux是如何运转的？
2.混乱的ARM architecture代码存在的问题
3.新内核的解决之道


#二：没有Device Tree 的ARM linux是如何运转的？

具体移植内核的作法：
1自己编写一个bootloader并传递适当的参数给kernel.除了传统的command line
及tag list之类的。最重要的是申请一个machine type，当拿到属于自己项目
的machine type ID的时候。

2.在内核的arch/arm目录下建立mach-xxx目录，这个目录下放该soc的相关代码，例如
中断控制器的代码，时间相关的代码，内存映射，睡眠相关的代码。此外最重要的是
建立一个board specific文件，定义一个machine的宏：


```c
MACHINE_START(project name, "xxx公司的xxx硬件平台") 
	.phys_io    = 0x40000000, 
	.boot_params = 0xa0000100,   
	.io_pg_offst = (io_p2v(0x40000000) >> 18) & 0xfffc, 
	.map_io = xxx_map_io, 
	.init_irq = xxx_init_irq, 
	.timer = &xxx_timer, 
	.init_machine = xxx_init, 
MACHINE_END
```
在xxx_init函数中，一般会加入很多的platform device。因此,伴随这个board specific
文件中是大量的静态table,描述了各种硬件设备信息。

3.调通system level的driver(timer 中断处理，clock等)以及串口terminal之后，linux
kernel基本是可以起来了，后续各种driver不断的添加，直到系统软件支持所有的硬件。

综上，在linux中支持一个soc平台其实是非常简单的。让linux kernel在一个特定的平台
上跑起来也是简单，问题是如何优雅。


#三：混乱的ARM architecture代码和存在的问题 
每次正式的linux kernel release之后都会有两周的merge window，在这个窗口期间，
kernel各个部分的维护者都会提交自己的patch,将自己测试稳定的代码请求并入kernel
main line.
Tony Lindgre 内核OMAP development tree的维护者，发送了一个邮件给linus,请求OMAP
平台代码修改，并给出了一些细节描述：

对于一件事情，不同层次的人有不同层次的思考。这次争论涉及的人包括：
1.内核的维护者
2.维护ARM系统结构代码的人
3.维护ARM sub architecture的人(来自各个ARM SOC vendor)

经过争论，确定的问题如下：
1.ARM linux缺少platform(各个ARM sub architecture，或者说各个SOC之间的协调，导致
arm linux的代码有重复。值得一提的是在本次争论之前，ARM维护者已经进行了不少相关工作
例如PM和clock tree)来抽象相同的功能模块。

2.ARM Linux中大量的board specific的源代码应该踢出kernel，否则这些垃圾代码和
table会影响linux kernel的长期目标。

3.各个sub architecture的维护者直接提交给linux并入主线的机制缺乏层次。

#四：新内核的解决之道
针对ARM linux的现状，最需要解决的是人员问题，也就是如何整合arm sub architecture
(各个ARM Vendor)的资源.因此，内核社区成立了一个ARM sub architecture的team。该
team主要去负责协调各个ARM厂商代码(not ARM core part)
Russell King继续负责ARM core part的代码。此外建立一个ARM platform consolidation
tree.ARM sub architecture team负责review各个sub architecture维护者提交的代码。
并在ARM platform consolidation tree上维护。在下一个merge window到来的时候，将
patch发送给linus

针对重复代码问题。如果不同的soc使用相同的IP block(例如I2C controller),那么这个
driver的code要从各个arch/arm/mach-xxx中独立出来，变成一个通用的模块供各个soc
specific模块使用。移动到哪个目录，对于 I2C或者USB OTG而言，这些HW block的驱动
当然移动到kernel/drivers目录。因为对于 这些外设可能是in-chip,也可以是off-chip
对于软件而言，它们是没有差别的。
对于 那些system level的code.例如 clock control，interrupt control.
其实这些也不是ARM-specific。应该属于linux kernel的核心代码。应该放到linux/kernel
目录下。总结
##1.ARM核心代码仍然保存在arch/arm
##2.ARM Soc core architecture code 保存在arch/arm
##3.ARM Soc 的周边外设模块保存在drivers目录下
##4.ARM SOC 的特定代码在arch/arm/mach-xxx
##5.ARM SOC board specific的代码被移除，由Device Tree机制来负责传递硬件
##拓扑和硬件资源信息。

OK终于来到了Device Tree了。本质上，Device Tree机制改变了原来用hardcode方式
将HW配置信息嵌入到内核代码的方法。改用bootloader传递一个DB的形式。
对于基于ARM CPU的嵌入式系统，我们习惯于针对每个platform进行内核编译。
但是随着ARM在消费电子上的广泛应用，我们期望ARM可以用一个image来支持多个paltform
在这种情况下我们认识kernel是一个black box。
那么其输入参数应该包括：
1.识别platform的信息
2.runtime的配置参数
3.设备的拓扑结构以及特性

对于嵌入式系统，在系统启动阶段，bootloader会加载内核并将控制权转交给内核。此外
还需要把上述的三个参数信息传递给kernel。以便kernel可以有较大的灵活性。
Device Tree的设计目标就是如此。




Device Tree(二):基本概念
#一：前言
本文主要是介绍Device Tree的基础概念。

简单的说，如果要使用Device Tree，首先用户要了解自己的硬件配置和系统运行参数，并
把这些信息组织成Device Tree source file。通过DTC(Device Tree Compiler),可以将这些
	适合人类阅读的Device Tree Source file变成适合机器处理的Device Tree binary file
(有一个更好听的名字 DTB)

	在系统启动的时候，boot program(例如bootloader的交互式命令加载DTB,或者
			firmware可以探测到device的信息，组织成DTB保存在内存中)并把DTB的起始地址
	传递给client program(例如OS kernel. bootloader或者其他特殊的程序).对于计算机
	系统(computer system),一般是firmware->bootloader->OS,对于嵌入式系统，一般是
	bootloader->OS

	本文主描述下面两个主题：
	1.Device tree sourcefile语法介绍
	2.Device tree binaryfile格式介绍
	为了了解Device Tree结构我们给了一个例子：


```c
	/ o device-tree 
	|- name = "device-tree" 
	|- model = "MyBoardName" 
	|- compatible = "MyBoardFamilyName" 
	|- #address-cells = <2> 
	|- #size-cells = <2> 
	|- linux,phandle = <0> 
	| 
	o cpus 
	| | - name = "cpus" 
	| | - linux,phandle = <1> 
	| | - #address-cells = <1> 
	| | - #size-cells = <0> 
	| | 
	| o PowerPC,970@0 
	|   |- name = "PowerPC,970" 
	|   |- device_type = "cpu" 
	|   |- reg = <0> 
	|   |- clock-frequency = <0x5f5e1000> 
	|   |- 64-bit 
	|   |- linux,phandle = <2> 
	| 
	o memory@0 
	| |- name = "memory" 
	| |- device_type = "memory" 
	| |- reg = <0x00000000 0x00000000 0x00000000 0x20000000>
	| |- linux,phandle = <3> 
	| 
	o chosen 
	|- name = "chosen" 
	|- bootargs = "root=/dev/sda2" 
	|- linux,phandle = <4>

```
节点名字的格式是node-name@unit-address
如果该node没有reg属性(后面会描述这个property),那么该节点名字中必须不能包括@和
unit-address。unit-address的具体格式是和设备挂在那个bus上相关。

例如对于cpu,其unit-address就是从0开始编址，以此加一。而具体的设备，例如以太网
控制器，其unit-address就是寄存器地址。root node的node name是确定的，必须是"/"

在一个树状的device tree中，如何引用一个node？
要想唯一指定一个node必须使用full path，
例如/node-name-1/node-name-2/node-name-N.在上面的例子中，cpu node我们可以通过
/cpus/powerpc,970@0访问。

属性是(property)值标识了设备的特性，它的值value是多种多样的：
1.可能是空，也就是没有值的定义。
2.可以是一个u32 u64(值得一提是cell这个术语，在Device Tree表示32bit的信息单位)
	例如address-cells<1>.当然，可能是一个数组。例如<0x00000000 0x00000000 0x00000000
	0x20000000>

4.可能是一个字符串，例如device_type = "memory",当然也可能是一个string list.
例如"powerpc,970"



##三：Device Tree file语法介绍

了解基本的device tree结构后，我们总要把这些结构体现在device tree code上来。在
linux kernel中，扩展名是dts的文件就是描述硬件信息的device tree source file,
在dts文件中，一个node被定义成：

[label:]node-name[@unit-address]{
	[properties definitions]
	[child nodes]
}

"[]"表示option，因此可以定义一个只有node name的空节点，label方便在dts中引用，
具体后面会描述。child node的格式和node是完全一样的，因此，一个dts文件中就是
若干嵌套组成的node property以及child node child node property

考虑到空泛的谈比较枯燥，我们用实例来讲解Device tree Source file数据格式。
假设科技制作了一个s3c2416的开发板，我们把该development board命令为snail,那么
需要编写一个
**s3c2416-snail.dts**的文件。如果把所有的开发板的硬件信息(SOC以及外设)都描述
在一个文件中是不合理的。因此有可能其他公司也使用s3c2416搭建自己的开发板并命令
pig cow什么的。如果大家都用自己的dts文件描述硬件，那么其中大部分是重复的，因此
我们把和s3c2416相关的硬件描述保存一个单独的dts文件文件可以供使用s3c2416的target
board来引用并将文件的扩展名变成dtsi。同理，三星公司的s3c24xx系统是一个soc family
同样ARM vendor也会共用一些硬件定义信息，这个文件就是skeleton.dtsi。
我们自下而上逐个进行分析。

1.skeleton.dtsi 
```c
/ { 
#address-cells = <1>; 
#size-cells = <1>; 
	chosen { }; 
	aliases { }; 
	memory { device_type = "memory"; reg = <0 0>; }; 
};

```

device tree是一个树状结构。 "/"是根节点的node name
"{" "}"之间的内容是该节点的具体定义，其内容包括各种属性的定义以及child node的定义 
chosen, aliases和memory都是sub node,sub node的结构和root node是完全不一样的。
因此，sub node也有自己的属性和它自己的sub node,sub node和结构和root node是完全
一样的。因此,sub node也有自己的sub node，最终形成一个树状的device tree。
属性的定义采用property = value的形式。例如#address-ceslls和#size-cell就是
property.
而<1>就是value,value有三种情况:
1)属性值是text string 或者string list，用双引号表示例如：device_type = ”memory“
2)属性值是32bit unsigned integers #size-cells = <1>
3)属性值是binary data,用方括号 binary-property[0x01 0x23 0x45 0x67]

如果一个device node中包含了寻址需要(要定义reg property)的sub node(后文件也许
会用child node,和sub node是一样的意思),那么必须要定义这两个属性。"#"是number
的意思，#address-cells这个属性是用来描述sub node中的reg属性的地址域特性，也就是
说需要用多少个u32的cell来描述该地址域。同理可以推断#size-cell的含义，
下面对reg的描述中会给出更详细的信息.

choose node主要用来描述由系统firmware指定runtime parameter.如果存在choose这个
node，其parent node必须是名字是"/"的根节点。

原来通过tag list传递一些linux kernel的运行时参数可以通过Device Tree传递。

例如command line可以通过bootargs这个property这个属性传递,initrd的开始地址也
可以通过linux.initrd-start这个property这个属性.
在本例中，chosen节点是空的，在实际中，建议增加一个bootargs的属性，例如：

```c
"root=/dev/nfs nfsroot=1.1.1.1:/nfsboot 
ip=1.1.1.2:1.1.1.1:1.1.1.1:255.255.255.0::usbd0:off console=ttyS0,115200 
mem=64M@0x30000000
```


通过该command line 可以控制内核从usb启动，当然，具体项目要相应修改command line
.device tree用于hw platform识别，runtime parameter传递以及硬件设备描述。
chosen节点并没有描述任何硬件设备节点信息，它只是传递了runtime parameter

aliases节点定义了一些别名。为何要定义这个node呢？因为Device tree是树状结构
当要引用一个node的时候要指明相对于root node的full path,例如/node-name-1/
node-name-2/node-name-N.如果多次引用，每次写复杂的名字，因此可以在aliases
节点定义一些设备节点的full path的缩写。


memory device node 是所有设备树节点文件的必备节点，它定义了系统物理内存的layout
device_type属性定义了该node的设备类型，例如cpu serial。对于memory node,
其中device_type必须等于memory。reg属性定义了访问该device node的地址信息，该属性
的值被解析成任意长度的(address size)数组,具体用多长的数据来表示address和size
是在其parent node中定义(#address-cell #size-cell)

对于device node，reg 描述了memory-mapped IO register的offset和length.
对于memory node,定义了memory的起始地址和长度.


本例中的物理内存的布局并没有通过memory node传递，其实我们可以使用command line传
递，我们command line中的参数“mem=64M@0x30000000”已经给出了具体的信息。
我们用另外一个例子来加深对本节描述的各个属性以及memory node的理解。
假设我们的系统是64bit的，physical memory分成两段，定义如下：

RAM: starting address 0x0, length 0x80000000 (2GB) 
RAM: starting address 0x100000000, length 0x100000000 (4GB)

对于这样的系统，我们可以将 root node中的address-cells 和size-cells这两个
属性值设定为2.可以用下面两种方法来描述物理内存
```c
方法1：

memory@0 { 
	device_type = "memory"; 
	reg = <0x000000000 0x00000000 0x00000000 0x80000000 
		0x000000001 0x00000000 0x00000001 0x00000000>; 
};

方法2：

memory@0 { 
	device_type = "memory"; 
	reg = <0x000000000 0x00000000 0x00000000 0x80000000>; 
};

memory@100000000 { 
	device_type = "memory"; 
	reg = <0x000000001 0x00000000 0x00000001 0x00000000>; 
	·};
```


2.s3c24xx.dtsi.位于arch/arm/boot/dts目录下，具体该文件内容如下：
```c
#include "skeleton.dtsi"

/ { 
	compatible = "samsung,s3c24xx"; －－－－－－－－－－－－－－－－－－－（A） 
	interrupt-parent = <&intc>; －－－－－－－－－－－－－－－－－－－－－－（B）

	aliases { 
		pinctrl0 = &pinctrl_0; －－－－－－－－－－－－－－－－－－－－－－－－（C） 
	};

	intc:interrupt-controller@4a000000 { －－－－－－－－－－－－－－－－－－（D） 
		compatible = "samsung,s3c2410-irq"; 
		reg = <0x4a000000 0x100>; 
		interrupt-controller; 
		#interrupt-cells = <4>; 
	 };

	 serial@50000000 { －－－－－－－－－－－－－－－－－－－－－－（E）  
		compatible = "samsung,s3c2410-uart"; 
		reg = <0x50000000 0x4000>; 
		interrupts = <1 0 4 28>, <1 1 4 28>; 
		status = "disabled"; 
	 };

	 pinctrl_0: pinctrl@56000000 {－－－－－－－－－－－－－－－－－－（F） 
		  reg = <0x56000000 0x1000>;

		  wakeup-interrupt-controller { 
		  compatible = "samsung,s3c2410-wakeup-eint"; 
		  interrupts = <0 0 0 3>,
				  <0 0 1 3>,
				  <0 0 2 3>,
				  <0 0 3 3>,
				  <0 0 4 4>,
				  <0 0 5 4>;
		   }; 
	  };
		   …… 
};
```



(A)在描述compatible属性之前要先描述一个model属性。model属性指明了该设备属于哪个
设备生产商的哪一个model。一般而言，我们会给model赋值"manufaturer,model"

现在我们回到compatible属性，该属性的值是string list定义了一系列的model
(每个string是一个model)这些字符串列表被操作系统用来选择用哪个driver来驱动设备

假设定义该属性:
compatible = "aaaaaa","bbbbbb" 那么操作系统可能首先使用aaaaaa来匹配适合的driver
如果没有匹配到，那么使用字符串bbbbbbb来继续寻找适合的driver,对于本例，

compatible = "samsung, s3c24xx",这里只定义一个modle而不是一个list.对于root node
compatible属性是用来匹配machine type的(在device tree代码分析文章中会给出更细致
的描述).对于普通的hw block的节点，例如interrupt-controller，compatible属性是
用来匹配适合的driver的。


(B)具体各个HW block的interupt source是如何连接到interruptcontroller?在dts文件中
是用interrupt-parent这个属性来标识的。且慢，这里定义的interrup-parent属性是
root node，难道root node会产生中断到interrupt-parent

intc是一个lable，标识了一个device node,实际上，不过,在dts文件中，可以使用类c
语言的机制。定义一个lable,唯一标识一个node或者property，后续可以使用&来引用这个
lable.DTC会将lable转换成u32的整数放入到DTB

关于interrupt
统中有一个interrrupt tree的根节点，device1、device2以及PCI host bridge的interrupt
line都是连接到root interrupt controller的。PCI host bridge设备中有一些下游的设备
，也会产生中断，但是他们的中断都是连接到PCI host bridge上的
interrupt controller（术语叫做interrupt nexus），然后报告到
root interrupt controller的。每个能产生中断的设备都可以产生一个或者多个
interrupt，每个interrupt source（另外一个术语叫做interrupt specifier，
描述了interrupt source的信息）都是限定在其所属的interrupt domain中。

在了解了上述的概念后，我们可以回头再看看interrupt-parent这个属性。
其实这个属性是建立interrupt tree的关键属性。
它指明了设备树中的各个device node如何路由interrupt event。
另外，需要提醒的是interrupt controller也是可以级联的，上图中没有表示出来。
那么在这种情况下如何定义interrupt tree的root呢？
那个没有定义interrupt-parent的interrupt controller就是root。

(C) pinctrl0是一个缩写，他是/pinctrl@56000000的别名。这里也是使用了Labels and
References机制。

(D)

intc（node name是interrupt-controller@4a000000 ，我这里直接使用lable）是描述
interrupt controller的device node。根据S3C24xx的datasheet，我们知道interrupt
controller的寄存器地址从0x4a000000开始，长度为0x100（实际2451的interrupt
的寄存器地址空间没有那么长，0x4a000074是最后一个寄存器），也就是reg属性定义的
内容。interrupt-controller属性为空，只是用来标识该node是一个
interrupt controller而不是interrupt nexus（interrupt nexus
需要在不同的interrupt domains之间进行翻译，需要定义interrupt-map的属性，
本文不涉及这部分的内容）。#interrupt-cells 和#address-cells概念是类似的，
也就是说，用多少个u32来标识一个interrupt source。我们可以看到，
在具体HW block的interrupt定义中都是用了4个u32来表示，
例如串口的中断是这样定义的：


interrupts = <1 0 4 28>,<1 1 4 28>

(E) 从reg属性可以serial controller寄存器从0x50000000开始，长度为0x4000。
对于一个能产生中断的设备必须定义interrupts这个属性。也可以定义interrup-parent

对于interrupt属性值，各个intertupt controller定义是不一样的。有的用3个u32，
有的用4个。具体上面的各个数字的解释权归相关的interrupt controller。对于
中断属性的具体描述我们会在device tree的第三份文件代码分析中描述。

(F)这个node是描述GPIO控制的，这个节点定义了一个wakeup-interrupt-controller的子
节点，用来描述有唤醒功能的中断源。


```
3. s3c2416.dtsi。位于arch/arm/boot/dts目录下，具体该文件的内容

```c

#include "s3c24xx.dtsi" 
#include "s3c2416-pinctrl.dtsi"

/ { 
	model = "Samsung S3C2416 SoC";  
	compatible = "samsung,s3c2416"; －－－－－－－－－－－－－－－A

		cpus { －－－－－－－－－－－－－－－－－－－－－－－－－－－－B 
		#address-cells = <1>; 
		#size-cells = <0>;

			cpu { 
				compatible = "arm,arm926ejs"; 
			}; 
		};

	interrupt-controller@4a000000 { －－－－－－－－－－－－－－－－－C 
		compatible = "samsung,s3c2416-irq"; 
	};

	……

};

```
(A) 在s3c24xx.dtsi文件中定义了compatible这个属性，在s3c2416.dtsi中重复定义了
这个属性,一个node不可能有相同的名字，具体如何处理就交给DTC了，经过反编译，
可以看出，DTC是丢弃了一个定义。因此，到目前为止，compatible = samsung，s3c2416.
在s3c24xx.dtsi文件中定义了compatible的属性被覆盖了。

(B)对于根节点，必须有一个cpus的child node来描述系统中的cpu信息。对于CPU的编址
我们用一个u32整数就可以描述了，因此，对于cpus node，#address-cells是1，而
#size-cells是0。其实cpu的node可以定义很多属性，例如TLB，cache，频率信息什么的。
不过对于ARM，这里只是定义了compatible属性就OK了，arm926ejs包括了所有processor
所有的相关信息。

(C)s3c24xx.dtsi文件和s3c2416.dtsi中都有interrupt-controller@4a000000这个node,
DTC会对这两个node进行合并，最终编译的结果如下：
```c
interrupt-controller@4a000000 { 
	compatible = "samsung,s3c2416-irq"; 
	reg = <0x4a000000 0x100>; 
	interrupt-controller; 
	#interrupt-cells = <0x4>; 
	linux,phandle = <0x1>; 
	phandle = <0x1>; 
};
```
4.s3c2416-pinctrl.dtsi
这个文件定义pinctrl@56000000这个节点的若干child node 主要用来描述GPIO的blank
的信息。

5.s3c2416-snail.dts
这个文件应该定义一些SOC之外的perpherals的定义

#四 Device Tree binary
1.DTB整体结构
经过Device Tree Compiler编译，Device Tree source file变成了Device Tree Blob
(又称作flattened device tree)的格式。Device Tree Blob的数据组织如下图所示：

DTB header

alignment gap

memory reserve map

alignment gap

device-tree structure

alignment gap

device-tree strings

2.DTB header
	对于DTB header,其各个成员解释如下：

header field name          description
magic                      用来识别DTB的，通过这个magic,kernel可以确定
						   bootloader传递的参数block是一个DTB还是taglist
totalisize                 DTB的total size

off_dt_struct	           device tree structure block的offset
off_dt_strings	           device tree strings block的offset
off_mem_rsvmap	           offset to memory reserve map。
						   有些系统，我们也许会保留一些memory有特殊用途
						   （例如DTB或者initrd image），或者在有些DSP+ARM的SOC
						   platform上，有写memory被保留用于ARM和DSP进行信息
						   交互。这些保留内存不会进入内存管理系统。
version						该DTB的版本。
last_comp_version			兼容版本信息
boot_cpuid_phys				我们在哪一个CPU（用ID标识）上booting

dt_strings_size           	device tree strings block的size。
							和off_dt_strings一起确定了strings block在内存中的位置
dt_struct_size	            device tree structure block的size。
							和和off_dt_struct一起确定了device tree structure
							block在内存中的位置


3. memory reserve map的格式描述
这个区域包括了若干的reserve memory描述符是由address和size组成。其中address和
size都是用u64来描述。

4.device tree struct block的格式描述
device tree structure block的格式描述
device tree structure block区域是由若干的分片组成，每个分片开始位置都是保存了
token以此来描述该分片的属性和内容，共计5种token
（1）FDT_BEGIN_NODE (0x00000001)。该token描述了一个node的开始位置，紧挨着该token的就是node name（包括unit address）

（2）FDT_END_NODE (0x00000002)。该token描述了一个node的结束位置。

（3）FDT_PROP (0x00000003)。该token描述了一个property的开始位置，该token之后是两个u32的数据，分别是length和name offset。length表示该property value data的size。name offset表示该属性字符串在device tree strings block的偏移值。length和name offset之后就是长度为length具体的属性值数据。

（4）FDT_NOP (0x00000004)。

（5）FDT_END (0x00000009)。该token标识了一个DTB的结束位置。

一个可能的DTB的结构如下：

（1）若干个FDT_NOP（可选）

（2）FDT_BEGIN_NODE

node name

paddings

（3）若干属性定义。

（4）若干子节点定义。

（5）若干个FDT_NOP（可选）

（6）FDT_END

5.device tree string bloc的格式描述
device tree strings bloc定义了各个node中使用的属性的字符串表，由于很多属性会
出现在多个node中，因此，所有的属性字符串组成了一个string block，这样可以压缩
size
							 




















