DTS 的学习主要通过三个文章来学习：

Linux DTS设备树详解之一(背景基础知识篇)
Linux DTS设备树详解之二
Linux DTS设备树详解之三

#一.什么是DTS?为什么要引入DTS
DTS即Device Tree Source 设备树源码，Device Tree是一种描述硬件的数据结构，
它起源于OpenFirmware(OF)。











#2.DTS的描述信息
Device Tree由一系列被命名的结点(node)和属性(property)组成,而结点本身可包含子
结点。所谓属性，其实就是成对出现的name和value。在Device Tree中，可以描述的
信息包括(原先这些信息大多被hard code到kernel中)：

CPU的数量和类别
内存基地址和大小
总线和桥
外设连接
中断控制器和中断使用情况 
GPIO控制器和GPIO使用情况
Clock控制器和Clock使用情况
	它基本上就是画一颗电路板上CPU，总线，设备组成的树，Bootloader会将这颗树
传递给内核，然后内核可以识别这颗树，并根据它展开出Linux内核中的paltform_device
i2c_client, spi_device等设备，而这些设备用到的内存，IRQ资源，也被传递给了内核，
内核会将这些资源绑定给展开的相应设备。

Device Tree要描述系统中所有硬件？可以动态探测到不需要写入

.dts文件中一种ASCII文本格式的Device Tree描述。
基本上在ARM Linux，一个.dts文件对应一个ARM的machine,
一般放在文件arch/arm/boot/dts/目录。由于一个Soc可能对应多个amchine
(一个soc对应多个产品和电路板),势必这些.dts文件需要包含许多共同的部分，
Linux内核为了简化，把SoC公用的部分或者多个machine共同部分一般提纯为.dtsi,类似于
C语言的头文件。其他machine对应的.dts就是include这个.dtsi


正常情况下所有的dts文件及dtsi文件都含有一个根节点“/" 
Device Tree Compiler会对DTS的node进行合并，最终生成的DTB只有一个root node。

Device Tree的基本单元是node。这些node被组织成树状结构，除了root node,每个node都 
只有一个parent。一个device tree文件中只能有一个root node。每个node包含了若干的
property/value来描述node的一些特性。每个node用节点名字(node name)标识，节点的
名字的格式是node-name@unit-adderss.

如果该node没有reg属性(后面会描述这个property),那么 节点名字必须不能包括@和unit-address
unit-address的具体格式是和设备挂在那个bus上相关

例如对于 cpu，其unit-address就
是从0开始编址，以此加一。而具体的设备，

例如以太网控制器，其unit-address就是寄存
器

root node和node name是确定的，必须是”/“

在一个树状结构的device tree中，如何引用一个node呢？要想唯一指定一个node必须使用
full path,例如/node-name-1/node-name-2/node-name-N.


#3.DTS的组成结构
/{
	node1{
		a-string-property = "A string";
		a-string-list-property = "first string", "second string";
		a-byte-data-property   = [0x01 0x23 0x34 0x56]
		child-node1{
			first-child-property;
			second-child-property = <1>;
			a-sting-property = "hello,world";
		};

		child-node2{
		};
	};

	node2{
		an-empty-property;
		a-cell-property= <1 2 3 4>;
		child-node1{
		}
	}
	
}

上述.dts文件并没有什么真实的用途，但它基本表征了一个DeviceTree源文件文件结构：
1个root结构"/"

下面以一个最简单的machine为例来看如何写一个.dts文件。假设machine配置如下：
1个双核ARM Cortex-A9 32位处理器;
ARM的local bus上的内存映射区域分布了2个串口(分别位于0x101F100 和0x101F2000)
	GPIO控制器（位于
	SPI控制器
	中断控制器和
	一个external bus桥
	External bus桥上又连接了SMC SMC91111 Ethernet()
	                        I2c控制器
							NOR flash

	External bus桥上连接的i2c控制器所对应的i2c总线上又连接了Maxin DS1338实
	时钟。其对于.dts文件为

#	在.dts文件中每个设备，都有一个compation属性，compation属性用户驱动和设备绑定.
	compatible属性是一个字符串列表,列表中的第一个字符串表征了结点代表的确切设备.
	形式为"<manufacturer> , <model>",其后的字符串表征可以兼容的其他设备.可以说
	前面是特指,后面的则涵盖更广的范围.
	如在arch/arm/boot/dts/vexpress-v2m.dtsi中的Flash结点:
	flash@0,00000000 {
		compatible = "arm, vexpress-flash", "cfi-flash"
					  reg = <0 0x00000000  0x04000000>,
						  bank-width = <4>;
	};

	接下来root结点/的cpus子结点下面又包含2个cpu子结点,描述了此machine上的2个CPU,
	并且二者的compatible属性为"arm,cotex-a9".
	注意cpus和cpus的2个cpu子结点的命名,它们遵循的组织形式为:<name>[a<unit-address>],
	<>中的内容是必选项,[]中的则为可选项.name是一个ASCII字符串,用于描述结点对应
	的设备类型,如3com Ethernet适配器
	对应的结点name宜为ethernet

	如果有一个结点描述的设备有地址,则应该给出@unit-address.多个相同类型设备结点
	的name可以一样,只要unit-address不同即可,如本例中含有cpu@0, cpu@1
	以及serial@..          serial@...设备的unit-address地址也经常在其对应结点
	的reg属性中给出.

	reg的组织形式为reg = <address length1
	其中每一组addresslength表明了设备使用的一个地址范围.

	在本例中,root结点的#address-cells = <1>;
	和#size-cells = <1>;决定了serial gpio spi等结点的address和length字段的
	长度分别为1.

	cpu结点为#address-cells= <1>;和#size-cells =<0>;
	决定了2个cpu子结点的address为1，
	而length为空，于是形成了2个cpu的reg =<0>;和reg =<1>;

	external-bus结点的#address-cells= <2>和#size-cells =<1>;
	决定了其下的ethernet、i2c、flash的reg字段形如reg = <0 00x1000>;
	reg = <1 00x1000>;和reg = <2 00x4000000>;
	其中，address字段长度为0，开始的第一个cell（0、1、2）是对应的片选，
	第2个cell（0，0，0）是相对该片选的基地址
	第3个cell（0x1000、0x1000、0x4000000）为length

	特别要留意的是i2c结点中定义的 #address-cells = <1>;
	和#size-cells =<0>;又作用到了I2C总线上连接的RTC
	，它的address字段为0x58，是设备的I2C地址。


	root结点的子结点描述的是CPU视图,因此root子结点的address区域就是直接位于
	CPU的memory区域.但是,经过总线桥后的address往往需要经过转换才能对应的CPU
	的memory映射,external-bus的ranges属性定义了经过
	external-bus桥扣的地址范围如何映射到CPU的memory区域.
> ranges =<0 0 0x10100000 0x10000
> <1 0 0x10100000 0x10000
> <2 0 0x10100000 0x10000

	ranges是地址转换表,其中每个项目是一个子地址,父地址以及在子地址空间的大小
	映射.映射表中的子地址,父地址分别采用子地址空间的#address-cell和父地址空间
	的address-cells大小.对于本例而言,子地址空间的#address-cells为2.

	Device Tree还可以中断连接信息,对于中断控制器而言,它提供如下属性:
	interrupt-cells- 与address-cells和#size-cells相似,它表明连接此中断控制器的
	设备的interrupts属性的cell大小.

	在整个Device Tree中,与中断相关的属性还包括:
	interrupt_controller
	cell和size

	在整个Device Tree中,与中断相关的属性还包括:
	interrupt-parent



4.dts引起BSP和driver的变更
没有使用dts之前的BSP和driver

	rtk_gpio_ctl_mlk{
		compatible = "realtek, rtk-gpio-ctl-irq-mux";
		gpios = <&rtk_iso_gpio 8 0 1>;
	};
	针对上面的dts,注意一下几点:


5.常见的DTS函数

6.DTC(device tree compiler)
	将.dts编译为.dtb的工具.
	在Linux下,我们可以单独编译Device Tree文件.当我们在Linux内核下运行
	make dtbs时,若我们之前选择了ARCH_VEXEPRESS,上述.dtb都会由相应的.dts编译
	出来.因为arch/arm/Makefile中含有一个dtbs编译target项目.



