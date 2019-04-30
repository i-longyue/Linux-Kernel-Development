USB充电规范----BC1.2中文详解

1.introduction
1.1 Scope
	规范定义了设备通过USB端口充电检测，控制和报告机制，这些机制是USB2.0规范的
扩展,用于专门充电器DCP，主机SDP，hub(SDP)和CDP(大电流充电端口)对设备的充电和
power up。这些机制适用于兼容USB2.0的所有主机和外设。

1.2 Backgroud
	PD(portable device)便携式设备连接到host或hub,USB2.0协议规定了三种情况下PD
	汲取电流的最大值:
	(1)bus在suspend(挂起)时，最大汲取电流2.5mA;
	(2)bus没suspend(挂起)并且未被配置时，是大汲取电流100mA;
	(3)bus没suspend(挂起)并配置时，最大涉取电流500mA;

	如果PD连接到CDP,DCP，ACA-Dock,ACA,在PD未配置时，汲取最大电流限制为1.5A,或者
遵循suspend的规则。定义了PD区别SDP和Charging port(充电端口)的机制.为不同的USB 
charger厂家定义了兼容性要求。如果PD的battery处在Dead或者weak状态，随USB2.0规范
发布的ECN规定，此时连接但未联通的PD可以汲取100mA电流
(连接与连通的的区别在于data线的上下电阻	)


1.3 reference Documents
(1) OTG and Embedded Host Supplement,Revision 2.0
(2) USB 2.0 Specification
(3) USB 3.0 Specification

1.4 Definitions of Terms
1.4.1 Accessory Charger Adaptor(附加者 适配)
	ACA是啥呢?也是一个充电器。一共三个口,一个OTG Port 连接PD,一个charger port
	连充电器，扩展出一个Accessory Port.对PD充电的同时，能使PD连接到Accessory。

	PS:根据Micro-ACA的<Table 6-1和Table 6-2可知,charger port连充电器的同时
	Accessory(附属) Port A-device,此时充电器可以对OTG Port的PD充电，但是PD并
	不能和Accessory Port连接的A-device进行通信。

##1.4.2 ACA-Dock
ACA Dock是一个扩展坞，有一个外接电源，有一个US port,没有或者有几个DS prot.
US port只能连接到作为host的PD，给PD提供最大1.5A的ICDP电流。DS port只能连接
device。
ACA-Dock怎么告知PD它是一个ACA-Dock呢？
	(1)在USB idle时候使能VDM_SRC(D-Source Voltage VDM_SRC Note 1 0.5 0.7v)
	(2)把ID下拉到GND,通过电阻RID_A.详细参考Section 3.2.4.4

PS:ACA Dock和ACA有啥区别呢？
连接ACA OTG Port的PD可以做B-Device
Accessary port可以连接A-device(但此时连接充电器);Dock的US Port只能连接作为A-
Device的PD,DS Port只支持B-device,只能在充电的同时连接一个或多个B-device到DUT。

#1.4.3 Attach versus Connect连接和连通的区别
	Attach我暂且把它翻译成连接，connect翻译为联通。它们有什么区别呢？
	Attach这个词是有方向性的，表示把设备连接到主机，有一个从下到上的动作。在
	物理上电源线，ID，信号线连接上了;"connect"是没方向性的，表示识别了物理上
	的连接，或建立了通信。所以"connect"是基于"attach"的.

	这里的connect是指在attach后,下游的设备通过上拉1.5K电阻到D+/D-线，
	使用bus进入Low-Speed,Full-Speed or High-Speed信号模式。

#1.4.4 charging Downstream Port
	CDP是啥呢?其实它就是在PC或者HUB上的一个USB口，但是这个usb口可以提供1.5A-5A
	的大电流充电，一般都会有一个闪电的标志在USB旁边。
	没连通外设时，当CDP检测到D+线上的电压VDAT_REF（Data Detect Voltage
	0.25V~ 0.4V) < VDP < VLGC(Logic Threshold 0.8V~2V)后，会在D-线上输出
	VDM_SRC(0.5 ~ 0.7v).从外设联通时起，CDP将不再把电压VDM_SRC输出到D-上，
	直到外设断开联通。

#1.4.5 Charging Port充电端口类型
	充电口的类型分为DCP CDP ACA—DOCK ACA

#1.4.6 Dead Battery Threshold 死电池阈值(没电电池的定义)
	什么是Dead Battery的阈值？通俗的说就是电池的一个电压值，低于这个值系统就
	肯定启动不了。电压就低于这个值的电池叫Dead Battery。

#1.4.7 Dedicated Charging Port CDP定义
	DCP就是墙充,即wall adapter。就是平时连到到220V插座的充电器。不能枚举USB设备
	可以供(4.75V<VCHG<5.25v)的稳定电压和(0.5A < IDCP <5A)的充电电流。DCP在内部
	将D+和D-短接.

#1.4.8 Downstream Port朝下的端口
	这个Spec中有两种DS，一种是SDP(standard Downstream port),另外一种是CDP
	(charging Downstream port)

#1.4.9 Micro ACA
	指ACA的accessory port是Micro-AB的母口

#1.4.10 Portable Device
	是能装在兜儿里的USB设备？就是移动设备。

#1.4.11 Rated Current额定电流
	充电端口的额定电流是指操持VBUS电压在VCHG(4.75V~5.25V)时所能输出的电流值。
	DCP的额定电流在(0.5A <IDCP <5A)之间;CDP or ACA-Dock的额定电流要求在(1.5A
			< ICDP < 5A)之间

#1.4.12 Standard ACA
	指ACA的accessory port是Standard-A的母口

#1.4.13 Standard Downstream Port SDP定义
	SDP是标准的DS Port，是遵循USB2.0规范的host或hub朝下的端口。
	一个带有good battery的设备在连接到SDP时：未联通或挂起时，从SDP汲取小于2.5mA
	的电流;

	联通未配置且没有挂起(suspend)时，从SDP汲取小于100mA的电流

	配置且没有挂起(suspend)时，从SDP汲取小于500mA的电流,具体值取决于host使能的
	配置SDP端口会将D+和D-和下拉500K电阻到GNDSDP有检测到D+被PD驱动到VDP_SRC
	(0.5v ~ 0.7v)的能力;

	在连接但没有联通的情况下，当PD汲取大于ISUSP(2.5mA)的电流时，要求PD驱动D+到
	VDP_SRC(0.5v ~ 0.7V)

#1.4.14 USB Charger USB
	这里就是指DCP,比如wall adapter或 car power adapter

#1.4.15 Weak Battery Threshold 弱电池阈值定义 
	Weak Battery阈值也是电池的一个电压值，电池电压高于这个值，系统就肯定能
	power up。

#1.5 OTG注意事项
	带有Dead Battery的PD不能区分PC和OTG-A device;
	当带有Dead Battery的PD连接到OTG-A device时，
	OTG A-device没有义务提供给PD超出正常值的供电电流
	(正常值是指OTG A-device正常供给其TPL列表中设备的电流值) 
	OTG A-device在TA_WAIT_BCON内没有检测到B-device的连接，可以停止驱动VBUS。
	也注是说，带有Dead Battery的PD,连接到OTG A-device以后因为不能
	连通，可能不会被充电。

#1.6 Super Speed Consideratons(SS注意事项)
	USB3.0规范中定义的SS Port也使用本规范的充电器检测机制,当PD检测到连接在一个
	SS port的时候，ICFG_MAX 变成900mA，IUNIT 变成150mA。


2.Dead Battery Provision 死电池的规定(仅适用于SDP)
	2.1背景
	从1.4.13 SDP的定义我们知道有一种情况，当带有Dead Battery或者Week Battery的
	PD连接到SDP时候，可能不能和host建立连通，这时候host会限制输出电流在ISUP
	2.5以下。一些PD在这种情况下需要一段时间充电电流是ISUSP来Power up设备。
	USB2.0允许复合型设备在连接且未连通,或者suspend的情况下，从每个DS port
	都汲取2.5mA的电流。

2.2 DBP-PD未配置时的规定
	带有Dead Battery或Week Battery的PD在未配置的情况下，可以根据DBP规定从SDP的
	DS port汲取IUNIT电流，规定如下：连接后超过45分钟，PD和host建立连通或被枚举
	涉取电流降到ISUSP(2.5mA);连接但未连通时，PD驱动VDP_SRC:
	1.PD在连接到host 1s内，PD使能D+的电压，VDP_SRC(0.5v - 0.7v);

	2.PD在disable VDP_SRC后1s建立连通，即使能上拉电阻。
	(1)这个电流只能用于使用PD尽快的上电并枚举，或者充电到Weak Battery Threshold
	后还用这个电流充电打电话...。

	(2)只有独立使用电池运行的设备才允许使用DBP带有Dead Battery的PD要
	要求通过USB-IF compliance inrush test
	未配置状态包括两个时段：
	(1)连接但是没连通
	(2)联通但没配置
	PD在接收到host发送的SET_CONFIGURATION命令进入configured state

2.3 DBP - Configured clause DBP-PD在配置状态下规定
	带Dead or Weak Battery的PD在配置的状态下允许使用DBP规则从SDP涉取配置电流
	(最大到ICFG_MAX = 500mA),不需要通过USBCV测试.
	规则如下：响应接收到的令牌PD要求响应host发来的任何令牌，以NAK或有效的USB
	response响应USB reset一旦接收到复位信号，PD将减小充电电流到IUNIT。PD允许
	在接收到reset后断开连接。当断开连接，PD将使用DBP-Unconfigured Clause.
	响应USB suspend保持连接降低充电电流到ISUSP，或断开连接使用DBP-Unconfiged
	Clause超时后提供完整的USB功能，或者断开连接使用DBP-Unconfigured Clause
	从连接TDBP_FUL_FNCTN后，PD或着保持连通并且可以通过USBCV测试，或者断开
	连接。断开接连后使用DBP-Unconfigured Clause使用配置的DBP电流尽快的充电
	使用电压达到Weak Battery Threshold并提供完整的USB功能。

	PD使用DBP电流做不相关的事，比如高于Weak Battery Threshold后还用这个电流
	充电，打电话，播放音乐视频或游戏，建立无线连接。

	一旦电池压力在PD提供完整的USB功能PD需从在连接SDP后的TDBP_INFORM(mx = 1min)
	内通知用户PD正处在充电状态，且其它功能不可用。


3.charging Port Detection 充电端口的检测
3.1 概述
3.2 充电检测电路
本节简要的介绍了充电检测的硬件电路

3.2.1 Overview 概述
	Figure 3-2是PD中的充电检测的硬件电路

3.2.2 VBUS Detect VBUS检测
	Session是啥? 首先咱们先解释一下协议中经常出现的术语"session"
	在OTG的规范中对session做了这样的解释:
	"A session in defined as the period of time that VBUS is powered.
	The session ends when VBUS is no longer powered"

	从这名话来理解，session是VBUS从有到无一段时间,它是针对VBUS的，
	所以以后也可以理解为有效的VBUS，只是这时候VBUS是基于一定时间
	段有效的。

	每个PD的VBUS电源线的内部都有一个电压比较器，用来判断VBUS什么时候有效，
	和谁比较？和内部的有效电压阈值比，可以理解是和一个定值比，高于这个值就是
	有效的VBUS。这个阈值在本规范中叫internal session valid threshold。
	
	它的范围在定义为VOTG_SESS_VLD

	总结：PD中有个检测VBUS是否有效的电路，电路有一个参考值，高于这个值就认为是
	VBUS有效了,这个参考值不是固定的，设计的时候保证它在0.8V-4V之间就可以了。

3.2.3 Data Contact Detect 数据连接检测
	3.2.3.1 Overview 概述
	DCD机制使用了向D+提供的电流源IDP_SRC来检测PD连接host后，数据信号连接。
	观察USB数据线的公头里边的信号线，你会发现两边的PIN长，中间PIN的短。两
	侧的PIN是VBUS和GND，中间的是数据线。这样的作法是先供电现通信。

	PD并不一定要求实现DCD，如果PD没实现DCD，会使用一个定时器，它将在连接到host
	TDCD_TIMEOUT(DCD timeout TDCD_TIMEOUT 300 900 ms 3.2 3.1)后，开始primary 
	Detection.当PD连接到SDP或者CDP时，DCD机制能检测数据连接。
	这样可以降低通信建立的时间。

	一个上电的USB设备，要求在连接到host的TSVLD_CON_PWD内建立连通。DCD机制也可以
	在PD连接到DCP和ACA的多数情况下检测数据线的连接。

	DCD机制也可以在PD连接到DCP和ACA的多数情况下检测数据线的连接。DCD不起作用的
	情况有：

	1.漏电流太大的DCP
	2.连接charger和在Accessory Port连有FS或者HS B-device的
	3.ACA-DOCK
	4.把D+拉高的PS2端口
	5.把D+拉高的专用充电器 因为DCD并不能在所有情况work,如果PD在attach event后
	TDCD_TIMEOUT max(900ms)内还没检测到D+或者ID PIN的连接,就要求PD必须开始进行
	Primary Detection

	间隔时间，最长观察间隔时间是200msPD区分充电口和SDP的方式是根据data line.
	如果在检测到data pin连接前PD进行了primary Detection操作，则根据primary 
	Detection协议，PD认为被连接到SDP。如果PD连接到DCP，但是被其错误的识别为
	连接到SDP，在这种情况下PD将汲取ISUSP(2.5mA)电流同时等待被枚举。因为
	DCP不能枚举设备，因此PD将不会被充电.


	DCD协议如下：
	PD检测VBUS有效PD使能D+电流源IDP_SRC和D-线上的下拉电阻PD检测到D+线保持
	TDCD_DBNC.(Data contact detect debounce min = 10ms)
	低电平关闭D+电流源IDP_SRC和D-线上的下拉电阻 如果没有连接到PD上时，D+
	线保持在高电平。
	IDP_SRC(7uA)的最小值要求能保证在最坏漏电流情况下，使用D+保持在VLGC_HI

	当PD连接SDP时，D+线被SDP的RDP_DWN拉低IDP_SRC的最大值值要求能保证在最
	坏漏电流

3.2.4 Primary Detection
	PD要求实现Primary Detection。3.2.4 primary Detection用来区分SDP和charging
	port.

	3.2.4.1
	Primary Detection DCP
	3.2.4.2 Primary Detection CDP
	3.2.4.3 Primary Detection SDP
	3.2.4.4 primary Detection ACA-Dock


	3.2.5 Secondary Detection二次检测
	二次检测用来区分DCP还是CDP.PD在检测到VUBS的TSVLD_CON_PWD时间内，如果PD
	还没做好被枚举的准备，则要求PD进行二次检测。PD做好了被枚举的准备。则
	可以跳过二次检测。

	PS：什么是做好枚举准备？
	是指建立了连通，即下游设备已经使能了D+/D-线上的1.5K上拉电阻，使数据
	线进入了相应的信号模式。
	3.2.5.1 Secondary Detection DCP


3.3 Charger Detection Algorithms
3.3.1 Weak Battery Algorithm 弱电池算法
3.3.2 Good battery algorithm




3.4.2 Detection Timing,CDP
是CDP的主要检测和二次检测的时序，包含了比较D+和VDAT_REF and VLGC
根据条件使能


3.5 Ground current and noise Margins
	在USB2.0 spec图。100mA的电流在USB线缆中的GND line上能产生25mv
	的压差。这就造成了Host的GND和Device有25mv的压差。GND的电压差
	降低了信号和充电检测的噪声

4.对充电端口和PD的要求
CDP ACA-DOCK DCP ACA PD

4.1 charging Port Requirements对充电端口的要求
4.1.1 Overshoot正过冲
4.1.2 Maximum Current最大电流
4.1.3 Detection Renegotiation重新检测 
4.1.4 shutdown Operation 关断操作

4.2.1 需要的工作范围
4.2.2 关断操作
4.2.3 负过冲
4.2.4 Detection signaling 信号检测 
4.2.5 Connector 连接器

4.3.1 Required Operating Range
4.3.2 Undershoot负过冲
4.3.3 Detection signaling信号检测 
4.3.4 Connector连接器

4.4.1 Required Operating Range需要的工作范围
4.4.2 Undershoot负过冲
4.4.3 Detection Signaling信号检测
4.4.4 Connect

4.5.1 要求的工作范围
4.5.2 Undershoot负过冲
4.5.3 Detection Signaling信号检测
4.5.4 connector

4.6.1 要求的操作范围
4.6.2 Detection Signaling
4.6.3 重新检测
4.6.4 连接器

6.Accessory Charger Adapter
6.1 前言
	随着移动设备变得起来越小，对外只留一接口.
	端口，可以同时连接充电器和设备。这种方法是用了一种ACA的设备实现。


6.2.1 Micro ACA Ports

6.2.3 Micro ACA Architecture Micro ACA结构

6.2.4 Micro ACA Modes of Operation Micro ACA的工作模式

6.2.5 Implications of not Supporting Micro ACA Detection不支持
Micro ACA检测的影响。


