#**RK3399 USB DTS 配置说明** 


RK3399支持两个Type-C USB3.0,其中包括usb 3.0 PHY 和 DP PHY,两个USB2.0 Host。

其中两个Type-C USB3.0控制器都可以支持OTG(USB Peripheral和USB Host)
并且向下兼容USB2.0/1.1/1.0。

Type-C USB3.0 可以根据实现的应用需求，将物理接口设备设计为
Type-A USB3.0 Host
Micro USB3.0 OTG
Micro USB2.0 OTG等

RK内核USB驱动已经兼容这几种不同类型的USB接口，只需要修改DTS配置，
就可以使能相应的USB接口。


#(这里有DTS文档可以参考：Documentation/devicetree/bindings/usb/usb-ehci.txt)
#  ...                                                     phy/


#1 Type-C USB DTS配置（default)

RK3399 SDK DTS的默认配置,支持Type-C0 USB3.0 OTG  Type-C1 USB3.0 Host功能。
DTS的配置主要包括:
(1)DWC3控制器
(2)Type-C USB3.0 PHY
(3)USB2.0 PHY


##1.1 Type-C0/C1 USB控制器DTS配置----(1)DWC控制器配置RK3399 EVB
Type-C0/C1 USB控制器支持USB3.0 OTG功能,并且向下兼容USB2.0/1.1/1.0。

但是由于当前内核的USB框架只支持一个USB口作为Peripheral功能。
所以SDK默认配置Type-C0支持OTG mode而Type-C1仅支持Host mode,
以RK3399 EVB (某个版本)Type-C0/c1 USB3.0控制器DTS为例：
```dts
usbdr3_0:usb@fe800000{ /* type-c0 USB3.0 控制器DTS配置 */

			usbdrd_dwc3_0:dwc3@fe800000{
			  };
};

usbdr3_1:usb@fe900000{ /* type-c1 USB3.0 控制器DTS配置 */

			usbdrd_dwc3_1:dwc3@fe900000{
			  };
};

&usbdrd3_0 {
	extcon = <&fusb0>; /* extcon 属性 */
	status = "okay";""
};

&usbdrd_dwc3_0 {
	status = "okay";
};

&usbdrd3_1 {
	extcon = <&fusb1>; /* extcon 属性 */
	status = "okay";
};

&usbdrd_dwc3_1 {
	status = "okay";
};
```

##1.2.Type-C0/1 USB PHY DTS配置
USB PHY的硬件由USB3.0 PHY和USB2.0 PHY两部分组成所以，对应的USB PHY DTS也包括
USB3.0 PHY和USB2.0 PHY两部分.

###1.2.1 Type-C0/C1 USB3.0 PHY DTS配置
以RK3399 EVB3 Type-C0/C1 USB3.0 PHY DTS配置为例
```dts
tcphy0: phy@ff7c0000 {

	tcphy0_dp:dp-port {
		  };

	tcph0_usb3: usb3-port { /* Type-C0 USB3.0 port */
				};
};

tcphy1: phy@ff800000 {

	tcphy1_dp: dp-port {
			   };

	tcphy1_usb3: usb3-port { /* Type-C1 USB3.0 port */
			 };

};

&tcphy0 {
	   extcon = <&fusb0>;
	   status = "okay";
   };
&tcphy1 {
	extcon = <&fusb1>;
	status = "okay";
};


pinctrl {
	......
		fusb30x {
fusb0_int: fusb0-int { /* TypeC0 fusb302 中断 */
			   rockchip,pins = <1 2 RK_FUNC_GPIO &pcfg_pull_up>;
		   };
fusb1_int: fusb1-int { /* Type-C1 fusb302 中断 */
			   rockchip,pins = <1 24 RK_FUNC_GPIO &pcfg_pull_up>;
		   };
		};
};


 
&i2c0 {
fusb1: fusb30x@22 {
		   compatible = "fairchild,fusb302";
		   reg = <0x22>;
		   pinctrl-names = "default";
		   pinctrl-0 = <&fusb1_int>;
		   vbus-5v-gpios = <&gpio1 4 GPIO_ACTIVE_LOW>;
		   int-n-gpios = <&gpio1 24 GPIO_ACTIVE_HIGH>;
		   status = "okay";
	   };
	   ......
};

&i2c6 {
	status = "okay";
fusb0: fusb30x@22 {
		   compatible = "fairchild,fusb302";
		   reg = <0x22>;
		   pinctrl-names = "default";
		   pinctrl-0 = <&fusb0_int>;
		   vbus-5v-gpios = <&gpio1 3 GPIO_ACTIVE_LOW>;
		   int-n-gpios = <&gpio1 2 GPIO_ACTIVE_HIGH>;
		   status = "okay";
	   };
	   ......
};
```

###1.2.2 Type-c0/c1 USB2.0 PHY DTS配置
以RK3399 EVB3 Type-C0/C1 USB2.0 PHY DT配置
```dts
grf: syscon@ff770000 {
u2phy0:usb2-phy@e450 {
		   ...
		u2phy0_otg:otg-port{ /* type-c0 USB2.0 PHY port */

			};

	   };


u2phy1: usb2-phy@e460 {
	   
	   u2phy1_otg:otg-port{
				  }

	  }

}

&u2phy0 {
	 
	u2phy0_otg: otg-port{

				};
&u2phy1 {
	u2phy1_otg:otg-port{
			   };
```

##1.3 Type-C1 USB OTG DTS配置
在RK3399 EVB中type-C1只支持Host mode,实际产品中可以根据应用需求，可以修改
Type-C1作为OTG mode支持USB Peripherala功能。

有两个地方要修改。
(1)DTS的"dr_mode"属性。
&usbdrd_dwc3_1{
	
	dr_mode = "otg"; /* 配置Type-C1 USB控制器为OTG mode */
};  ????另一个地方的要修改为otg吗?

(2)init.rk30board.usb.rc的USB控制器地址(适用于Android平台)
设置USB控制器的地址为TypeC1 USB控制器的基基址：
setprop sys.usb.controller "fe900000.dwc3"


#2 Type-A USB3.0 Host DTS配置
typc-C USB可以配置为type-A USB使用。
如RK3399 BOX SDK平台的type-C1USB默认设计为Type-A USB Host。
这种设计USB Vbus5V一般为常供电，不需要单独的GPIO控制，也不需要fusb302芯片，
但type-c的三路供电需要正常开启,才能正常支持USB3.0 Super-speed.
C0三路供电为(USB_AVDD_0V9  USB_AVDD_1V8  USB_AVDD_3v3)
C1三路供电为(USB_AVDD_0V9  USB_AVDD_1V8  USB_AVDD_3v3)

Type-A USB3.0 Host DTS配置的注意点如下：
	(1)对应的fusb节点不要配置，因为type-A USB3.0不需要fusb302芯片。
	(2)对应的USB控制器的父节点(usbdrd3)和PHY的节点.
	(ctphy u2phy)都要删除extcon属性
	(3)对应的USB控制器子节点(usbdrd_dwc3)的dr_mode属性要配置为"host"

以RK3399 BOX平台为例(Type-C0配置为Type-C接口,Type-C1配置为Type-A USB3接口)
介绍Type-A USB3.0 Host DTS配置的方法:
```dts

**usb3.0 typc phy修改**
&tcphy0 {
	extcon = <&fusb0>; /* Type-C0 USB3 PHY extcon 属性 */
	status = "okay";
};

&tcphy1 { /* Type-A USB3 PHY 删除了extcon属性 */
	status = "okay";
};

&u2phy0 {
	status = "okay";
	extcon = <&fusb0>; /* Type-C0 USB2 PHY extcon 属性 */
u2phy0_otg: otg-port {
				status = "okay";
		};
}


&u2phy1 {
	 status = "okay"; /*Type-A USB2 PHY 删除了extcon属性*/
u2phy1_otg: otg-port {
				status = "okay";
			};
			......
 };


&usbdrd3_0 {
	extcon = <&fusb0>;
	status = "okay";
};

&usbdrd_dwc3_0 {
	dr_mode = "otg";
	status = "okay";
};

&usbdrd3_1 {
	status = "okay";
};
&usbdrd_dwc3_1 { /* Type-C1 USB 控制器删除extcon属性，同时配置dr_mode为host */
	dr_mode = "host";
	status = "okay";
};



&pinctrl {
	......
		fusb30x {
fusb0_int: fusb0-int {
			   rockchip,pins =
				   <1 2 RK_FUNC_GPIO &pcfg_pull_up>;
		   };
		};
	......
};

&i2c4 {
	status = "okay";
fusb0: fusb30x@22 { /* Type-C0 对应的fusb302芯⽚的节点，Type-C1不需要fusb302 */
		   compatible = "fairchild,fusb302";
		   reg = <0x22>;
		   pinctrl-names = "default";
		   pinctrl-0 = <&fusb0_int>;
		   vbus-5v-gpios = <&gpio1 3 GPIO_ACTIVE_LOW>;

		   int-n-gpios = <&gpio1 2 GPIO_ACTIVE_HIGH>;
		   status = "okay";
	   };
};

```


#3 Micro USB3.0 OTG DTS配置
Typc-C USB可以配置为Micro USB3.0 OTG使用。这种设计，硬件上不需要fusb302芯
片,USB Vbus 5V一般由GPIO控制,Type-C的三路供电与Type-A USB3.0的硬件电路一样。
需要正常开启。

Micro USB3.0 OTG DTS配置的注意点如下：
(1)	对应的fusb节点不要配置，因为Micro USB3.0不需要fusb302芯片。
(2)	对应的USB PHY节点（tcphy和u2phy）都要删除extcon属性。
(2)	对应的USB控制器父节点（usbdrd3）中，extcon属性引用为u2phy。
(3)	对应的USB控制器子节点（usbdrd_dwc3）的dr_mode属性要配置为"otg"。
(4)	对应的USB2 PHY节点（u2phy）中，配置Vbus regulator。
以typc-C0配置为micro USB3.0为例子

```dts
&tcphy0 { /* Micro USB3 PHY 删除了extcon属性 */
	status = "okay";
};

&u2phy0 {
	status = "okay"; /*Micro USB2 PHY 删除了extcon属性*/
	otg-vbus-gpios = <&gpio3 RK_PC6 GPIO_ACTIVE_HIGH>; /* Vbus GPIO 配置，⻅Note1 */

u2phy1_otg: otg-port {

				status = "okay";
			};
			......
};

&usbdrd3_0 {
	extcon = <&u2phy0>; /* Micro USB3 控制器的extcon属性引⽤u2phy0 */
	status = "okay";
};

&usbdrd_dwc3_0 {
	dr_mode = "otg"; /* Micro USB3 控制器的dr_mode配置为otg */
	status = "okay";
};
```

在4.4中有修改故更改为：
```dts
vcc_otg_vbus: otg-vbus-regulator {
	compatible = "regulator-fixed";
	gpio = <&gpio3 RK_PC6 GPIO_ACTIVE_HIGH>;
	pinctrl-names = "default";
	pinctrl-0 = <&otg_vbus_drv>;
	regulator-name = "vcc_otg_vbus";
	regulator-min-microvolt = <5000000>;
	regulator-max-microvolt = <5000000>;
	enable-active-high;
};
&pinctrl {
	......
		usb {
otg_vbus_drv: otg-vbus-drv {
				  rockchip,pins = <3 RK_PC6 RK_FUNC_GPIO &pcfg_pull_none>;
			  };
		};
};

&u2phy0 {
	status = "okay";
u2phy0_otg: otg-port {
				vbus-supply = <&vcc_otg_vbus>; /* 配置Vbus regulator 属性 */
				status = "okay";
			};
			......
};
```


#4.Micro USB2.0 OTG DTS配置
Type-C USB可以配置为Micro USB2.0 OTG使用。这种设计，硬件上不需要fusb302芯片
USB Vbus 5V般由GPIO控制，因为不需要支持USB3.0，所以对应的Type-C三路供电
USB_AVDD_0V9，USB_AVDD_1V8，USB_AVDD_3V3可以关闭。

Micro USB2.0 OTG DTS配置的注意点如下:
(1)对应的fusb节点不要配置，因为Micro USB2.0不需要fusb302芯片
(2)Disable对应的USB3 PHY节点（tcphy）
(3)对应的USB2 PHY节点（u2phy）要删除extcon属性，并且配置Vbus regulator
(4)对应的USB控制器父节点（usbdrd3）中，extcon属性引⽤为u2phy
(5)对应的USB控制器子节点（usbdrd_dwc3）的dr_mode属性要配置为"otg"，
maximum-speed 属性配置为high-speed，phys 属性只引用USB2 PHY节点

以Type-C0 USB配置为Micro USB2.0 OTG为例:

```dts
&tcphy0 {
	status = "disabled";
};

&u2phy0 {
	status = "okay"; /*Micro USB2 PHY 删除了extcon属性*/
	otg-vbus-gpios = <&gpio3 RK_PC6 GPIO_ACTIVE_HIGH>; /* Vbus GPIO 配置，⻅Note1 */
u2phy1_otg: otg-port {
				status = "okay";
			};
			......
};
&usbdrd3_0 {
	extcon = <&u2phy0>; /* Micro USB3 控制器的extcon属性引用u2phy0 */
	status = "okay";
};
&usbdrd_dwc3_0 {
	dr_mode = "otg"; /* Micro USB3 控制器的dr_mode配置为otg */
	maximum-speed = “high-speed”; /* maximum-speed 属性配置为high-speed */
	phys = <&u2phy0_otg>; /* phys 属性只引⽤USB2 PHY节点 */
	phy-names = "usb2-phy";
	status = "okay";
};
```


#5.USB2.0 Host DTS配置
	RK3399 支持两个USB2.0 HOST接口,对应的USB控制器为EHCI&OHCI	
	只提供type-A USB2.0 Host方案。

##5.1 USB2.0 Host 控制器 DTS配置

```dts
usb_host0_ehci: usb@fe380000 {
		compatible = "generic-ehci";
		reg = <0x0 0xfe380000 0x0 0x20000>;
		interrupts = <GIC_SPI 26 IRQ_TYPE_LEVEL_HIGH 0>;
		clocks = <&cru HCLK_HOST0>, <&cru HCLK_HOST0_ARB>,
			   <&cru SCLK_USBPHY0_480M_SRC>;
		clock-names = "hclk_host0", "hclk_host0_arb", "usbphy0_480m";
		phys = <&u2phy0_host>;
		phy-names = "usb";
		power-domains = <&power RK3399_PD_PERIHP>;
		status = "disabled";
	};
usb_host0_ohci: usb@fe3a0000 {
					compatible = "generic-ohci";
					reg = <0x0 0xfe3a0000 0x0 0x20000>;
					interrupts = <GIC_SPI 28 IRQ_TYPE_LEVEL_HIGH 0>;
					clocks = <&cru HCLK_HOST0>, <&cru HCLK_HOST0_ARB>,
						   <&cru SCLK_USBPHY0_480M_SRC>;
					clock-names = "hclk_host0", "hclk_host0_arb", "usbphy0_480m";
					phys = <&u2phy0_host>;
					phy-names = "usb";
					power-domains = <&power RK3399_PD_PERIHP>;
					status = "disabled";
				};
usb_host1_ehci: usb@fe3c0000 {
					compatible = "generic-ehci";
					reg = <0x0 0xfe3c0000 0x0 0x20000>;
					interrupts = <GIC_SPI 30 IRQ_TYPE_LEVEL_HIGH 0>;
					clocks = <&cru HCLK_HOST1>, <&cru HCLK_HOST1_ARB>,
						   <&cru SCLK_USBPHY1_480M_SRC>;

					clock-names = "hclk_host1", "hclk_host1_arb", "usbphy1_480m";
					phys = <&u2phy1_host>;
					phy-names = "usb";
					power-domains = <&power RK3399_PD_PERIHP>;
					status = "disabled";
				};
usb_host1_ohci: usb@fe3e0000 {
					compatible = "generic-ohci";
					reg = <0x0 0xfe3e0000 0x0 0x20000>;
					interrupts = <GIC_SPI 32 IRQ_TYPE_LEVEL_HIGH 0>;
					clocks = <&cru HCLK_HOST1>, <&cru HCLK_HOST1_ARB>,
						   <&cru SCLK_USBPHY1_480M_SRC>;
					clock-names = "hclk_host1", "hclk_host1_arb", "usbphy1_480m";
					phys = <&u2phy1_host>;
					phy-names = "usb";
					power-domains = <&power RK3399_PD_PERIHP>;
					status = "disabled";
				};



&usb_host0_ehci {
	status = "okay";
};
&usb_host0_ohci {
	status = "okay";
};
&usb_host1_ehci {
	status = "okay";
};
&usb_host1_ohci {
	status = "okay";
};

```


##5.2 USB2.0 Host PHY DTS 配置
```dts
grf: syscon@ff770000 {
		 compatible = "rockchip,rk3399-grf", "syscon", "simple-mfd";
		 reg = <0x0 0xff770000 0x0 0x10000>;
#address-cells = <1>;
#size-cells = <1>;
		 ......
			 u2phy0: usb2-phy@e450 {
				 compatible = "rockchip,rk3399-usb2phy";
				 reg = <0xe450 0x10>;

				 clocks = <&cru SCLK_USB2PHY0_REF>;
				 clock-names = "phyclk";
#clock-cells = <0>;
				 clock-output-names = "clk_usbphy0_480m";
				 status = "disabled";
				 ......
					 u2phy0_host: host-port { /* 配置USB2.0 Host0 USB2 PHY 节点 */
#phy-cells = <0>;
						 interrupts = <GIC_SPI 27 IRQ_TYPE_LEVEL_HIGH 0>;
						 interrupt-names = "linestate";
						 status = "disabled";
					 };
			 };
u2phy1: usb2-phy@e460 {
			compatible = "rockchip,rk3399-usb2phy";
			reg = <0xe460 0x10>;
			clocks = <&cru SCLK_USB2PHY1_REF>;
			clock-names = "phyclk";
#clock-cells = <0>;
			clock-output-names = "clk_usbphy1_480m";
			status = "disabled";
			......
				u2phy1_host: host-port { /* 配置USB2.0 Host1 USB2 PHY 节点 */
#phy-cells = <0>;
					interrupts = <GIC_SPI 31 IRQ_TYPE_LEVEL_HIGH 0>;
					interrupt-names = "linestate";
					status = "disabled";
				};
		};


vcc5v0_host: vcc5v0-host-regulator {
				 compatible = "regulator-fixed";
				 enable-active-high;
				 gpio = <&gpio4 25 GPIO_ACTIVE_HIGH>; /* 配置USB2.0 Host Vbus GPIO */
				 pinctrl-names = "default";
				 pinctrl-0 = <&host_vbus_drv>;
				 regulator-name = "vcc5v0_host";
				 regulator-always-on;
			 };
			 &pinctrl {
				 ......
					 usb2 {
host_vbus_drv: host-vbus-drv {
				   rockchip,pins =
					   <4 25 RK_FUNC_GPIO &pcfg_pull_none>;
			   };
					 };
				 ......
			 };



 &u2phy0 {
	 status = "okay";
	 ...
		 u2phy0_host: host-port {
			 phy-supply = <&vcc5v0_host>; /* 配置USB2.0 Host0 Vbus regulator 属性 */
			 status = "okay";
		 };
 };
 &u2phy1 {
	 status = "okay";
	 ...
		 u2phy1_host: host-port {
			 phy-supply = <&vcc5v0_host>; /* 配置USB2.0 Host1 Vbus regulator 属性 */
			 status = "okay";
		 };
 };
 


```	




