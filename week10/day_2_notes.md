#linux 下平台总线之一
	平台总线可以理解为一个虚拟的总线，主要描述soc上的资源，这些资源的共同点是
	CPU可以直接访问的资源。
	为什么要用这个平台总线，平台设备，平台驱动？

	1个linux设备驱动总要挂载在一条总线上面，比如usb I2c I2S SPI对于这种设备它
	们本身就已经有一条对应的总线了，但在嵌入式系统中soc上会有很多外设控制器，
	比如led，这个led并没有对应的标准总线让它挂载，所以linux内核就创建一个虚拟
	总线(平台总线)来让它挂载.

	平台驱动和传统驱动相比，有什么优势？
	平台驱动机制会将设备资源注册到linux内核里面，由linux的内核统一管理，这样就
	可以通过统一的接口来操作设备，在更换平台的时候，只需要修改平台驱动的设备资源
	就可以了。而不用修改驱动，移植性非常好。

平台设备的驱动软件架构设计流程是怎么样？
1.定义一个平台设备
2.注册平台设备
3.定义平台驱动
4.注册平台驱动。



#linux 下平台总线之二

一：重要结构体
1.1 总线是怎么定义的?
平台总线是一种模型，既然是模型，肯定对它进行抽象。那么必定有一个结构体用来表述它
另外，平台总线是总线的一种，它肯定会继承总线的特性，所以我们得先看看总线是怎么
定义的。

linux中总线是用结构体bus_type描述的，一般在drivers/base/device.h
```c
struct subsys_private{
	truct kset subsys;
	struct kset *devices_kset;           //sys/bus/platform/devices 目录
	struct list_head interfaces;
	struct mutex mutex;

	struct kset *drivers_kset;          //sys/bus/platform/drivers 目录
	struct klist klist_devices;         //device 链表
	struct klist klist_drivers;         //driver 链表
	struct blocking_notifier_head bus_notifier;
	unsigned int drivers_autoprobe:1;
	struct bus_type *bus;                //总线类型

	struct kset glue_dirs;
	struct class *class;
}

struct bus_type{
	const char		*name;        //总线的名字
	const char		*dev_name;
	struct device		*dev_root;
	struct device_attribute	*dev_attrs;	/* use dev_groups instead */
	const struct attribute_group **bus_groups;
	const struct attribute_group **dev_groups;
	const struct attribute_group **drv_groups;

	int (*match)(struct device *dev, struct device_driver *drv);
	int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
	int (*probe)(struct device *dev);
	int (*remove)(struct device *dev);
	void (*shutdown)(struct device *dev);

	int (*online)(struct device *dev);
	int (*offline)(struct device *dev);

	int (*suspend)(struct device *dev, pm_message_t state);
	int (*resume)(struct device *dev);

	const struct dev_pm_ops *pm;

	const struct iommu_ops *iommu_ops;

	struct subsys_private *p;
	struct lock_class_key lock_key;
}
 
struct bus_type有几个关键点：

	/* 总线的名称 */
	const char *name;  

	/* 设备和驱动的匹配方法 */
	int (*match)(struct device *dev, struct device_driver *drv);

	/* 设备和驱动匹配成功后调用的 */
	int (*probe)(struct device *dev);

	/* 移除设备时调用的函数 */
	int (*remove)(struct device *dev)
	
	struct subsys_private *p; 

```

1.2 平台总线是怎么定义的
在kerel-3.18/driver/base/platform.c找到平台总线的定义
```c
struct bus_type platform_bus_type = {
	.name = "platform,"
	.dev_groups = platform_dev_groups,
	.match      = platform_uevent,
	.pm         = &platform_dev_pm_ops,
};
```
可以看出平台总线就是用bus_type来表示的，只是它的名字为platform;
注意这里并没有初始化probe和remove函数！仅仅初始化了match函数

使用的时候是通过bus_register()函数进行注册的。

```c
int __init platform_bus_init(void)
{
	int error;
	early_platform_cleanup();
	error = device_register(&platform_bus);

	if(error)
		return error;

	error = bus_register(&platform_bus_type);

	if(error)
		device_unregister(&platform_bus);

	return error;
}
```
看到这里的时候会有一个？，那就是在bus_register()函数前面，还有一个
device_register(&platform_bus);
```c
struct device platform_bus = {
	.init_name = "platform"
}
```

二.平台总线是什么时候注册的?
这里对Android下Linux启动过程进行分析：



```c
tart_kernel()  //kernel-3.18/init/main.c  start_kernel() 启动内核
	|
    |--- rest_init() //kernel_thread(kernel_init, NULL, CLONE_FS); 创建1个内核线程做初始化
   	|
    |---kernel_init()  kernel_init_freeable(); 
	|
	|--do_basic_setup(); //做一些基本的初始化设置
	|
	|---driver_init(); //kernel-3.18/drivers/base/init.c driver_init() 驱动初始化
	|    |
	|    |---devices_init();
	|    |
	|    |---buses_init();  //创建了 /sys/bus 目录
	|    |    |
	|    |    |--bus_kset = kset_create_and_add("bus", &bus_uevent_ops, NULL);  
	|    |        //也就是说 bus_kset 在 sysfs 中的节点是 sys/bus/
	|    |
	|    |---classes_init();
	|    |
	|    |---platform_bus_init(); // kernel-3.18/drivers/base/platform.c 注册平台总线
	|        |
	|        |---device_register(&platform_bus); //注册设备
	|        |
	|        |---bus_register(&platform_bus_type); //注册平台总线，可以看出/sys/bus/platform 会维护 devices 和 drivers 两个链表,匹配时会用到
	|            |
	|            |---priv = kzalloc(sizeof(struct subsys_private), GFP_KERNEL); 先申请一个subsys_private
	|            |
	|            |---kobject_set_name(&priv->subsys.kobj, "%s", bus->name); 将"platform" 赋值过去
	|            |
	|            |---kset_register(&priv->subsys);
	|            |
	|            |---bus_create_file(bus, &bus_attr_uevent); 创建 /sys/bus/paltform 结点
	|            |
	|            |---kset_create_and_add("devices", NULL, &priv->subsys.kobj); //创建 /sys/bus/platform/devices
	|            |
	|            |---kset_create_and_add("drivers", NULL, &priv->subsys.kobj); //创建 /sys/bus/platform/drivers
	|    
	|---do_initcalls(); //加载module_init声明的驱动
```





	其中，bus_kset在buses_init()中初始化时指定的就是/sys/bus目录结构为 /sys/bus
	/platform

	priv->driver_autoprobe = 1;这里的赋值1也是很关键的，代表注册驱动的时候会
	自动探测驱动是否匹配，从而匹配成功后跑probe函数。

	3）kset_register(&priv->subsys);
		注册kset,创建目录，一组目录下层关系。

	4）retval = bus_create_file(bus, &bus_attr_uevent);
	在当前bus,对我们的platform总线下生成bus_attr_uevent
	5)priv->devices_kset = kset_create_and_add("devices"...)
	6)priv->devices_kset = kset_create_and_add("drivers"...)

```c
三. int __init platform_bus_init（void) 函数分析

struct device platform_bus = {
	.init_name = "platform"
};

struct bus_type platform_bus_type = {
	.name = "platform"
	.dev_groups = platform_dev_groups，
	.match = platform_match,
	.uevent = platform_uevent,
	.pm     = &platform_dev_pm_ops,
};


paltform_bus_init(void)
	|
	|--early_platform_cleanup();
	|
	|--device_register(&platform_bus);   //注册设备,实际上创建 /sys/devices/platform 目录？
	|    |
	|    |--device_initialize(dev);
	|    |
	|    |--device_add(dev);
	|
	|--bus_register(&platform_bus_type);
	|
	|--priv = kzalloc(sizeof(struct subsys_private), GFP_KERNEL); 先申请一个subsys_private
	|
	|--kobject_set_name(&priv->subsys.kobj, "%s", bus->name); 将"platform" 赋值过去
	|
	|--bus_create_file(bus, &bus_attr_uevent); 创建 /sys/bus/paltform 结点
	|
	|---kset_create_and_add("devices", NULL, &priv->subsys.kobj); //创建 /sys/bus/platform/devices
	|
	|---kset_create_and_add("drivers", NULL, &priv->subsys.kobj); //创建 /sys/bus/platform/drivers
```

#linux下平台总线三
	一：重要结构体
	二：linux下平台的注册是如何实现的?
	2.1 linux 设备的注册
	2.2 linux 平台的注册

```c
	一：重要结构体
	struct platform_device {
		const char *name;
		int id;
		bool id_auto;

		/* 设备 */
		struct device dev;

		/* 平台的资源 */
		u32 num_resources;

		/* 平台的资源 */
		struct resource *resource;

		/* MFD cell pointer */
		const struct paltform_device_id *id_entry;
		/* arch specific additions */
		char *driver_override;
	};


struct device {
	struct device		*parent;   //父设备，通常是一些总线或者主机控制器

	struct device_private	*p;

	struct kobject kobj;
	const char		*init_name; /* initial name of the device */
	const struct device_type *type;   //设备类型

	struct mutex		mutex;	/* mutex to synchronize calls to
								 * its driver.
								 */

	struct bus_type	*bus;  //表示设备挂载在那条总线上
	struct device_driver *driver;	//与设备绑定的驱动是哪个
	void		*platform_data;	/* Platform specific data, device
								   core doesn't touch it */
	void		*driver_data;	/* Driver data, set and get with
								   dev_set/get_drvdata */
	struct dev_pm_info	power;
	struct dev_pm_domain	*pm_domain;

#ifdef CONFIG_PINCTRL
	struct dev_pin_info	*pins;
#endif

#ifdef CONFIG_NUMA
	int		numa_node;	/* NUMA node this device is close to */
#endif
	u64		*dma_mask;	/* dma mask (if dma'able device) */
	u64		coherent_dma_mask;/* Like dma_mask, but for
								 alloc_coherent mappings as
								 not all hardware supports
								 64 bit addresses for consistent
								 allocations such descriptors. */
	unsigned long	dma_pfn_offset;

	struct device_dma_parameters *dma_parms;

	struct list_head	dma_pools;	/* dma pools (if dma'ble) */

	struct dma_coherent_mem	*dma_mem; /* internal for coherent mem
										 override */
#ifdef CONFIG_DMA_CMA
	struct cma *cma_area;		/* contiguous memory area for dma
								   allocations */
#endif
	/* arch specific additions */
	struct dev_archdata	archdata;

	struct device_node	*of_node; /* associated device tree node */
	struct acpi_dev_node	acpi_node; /* associated ACPI device node */

	dev_t			devt;	/* dev_t, creates the sysfs "dev" */
	u32			id;	/* device instance */

	spinlock_t		devres_lock;
	struct list_head	devres_head;

	struct klist_node	knode_class;
	struct class		*class;
	const struct attribute_group **groups;	/* optional groups */

	void	(*release)(struct device *dev);
	struct iommu_group	*iommu_group;

	bool			offline_disabled:1;
	bool			offline:1;
};

```



二：linux下平台设备的注册是如何实现的.


boot加载linux kernel
	|
    |--start_kernel() //启动内核
	|
	|--rest_init();
	|
	|--kernel_thread(kernel_init, NULL, CLONE_FS); //创建kernel_init线程
	|
    |--static int __ref kernel_init(void *unused)
	|
	|--kernel_init_freeable();
	|
	|--do_basic_setup();  //做一些初始化动作
	|
	|--driver_init();
	|    |
	|    |--devices_init(); //实际上是创建一些与device 相关的目录
	|        |
	|        |--devices_kset = kset_create_and_add("devices", &device_uevent_ops, NULL);  //创建/sys/devices 目录
	|        |
	|        |--dev_kobj = kobject_create_and_add("dev", NULL); //创建 /sys/dev 目录
	|        |
	|        |--sysfs_dev_block_kobj = kobject_create_and_add("block", dev_kobj);  //创建 /sys/dev/block 目录
	|        |
	|        |--sysfs_dev_char_kobj = kobject_create_and_add("char", dev_kobj);    //创建 /sys/dev/char 目录
	|
	|--do_initcalls(); //加载 module_init()声明的驱动，然后驱动中的platform_driver_register() 被加载

关键点分析：
1)driver_init()函数中先进行的是devices_init(),然后才是buses_init(),然后进行
platform_bus_init().

void __init driver_init(void)
{
	/* these are the core pieces */
	devtmpfs_init();
	devices_init();
	buses_init();
	classes_init();
	.....
	paltform_bus_init();
}


2)devices_kset = kset_create_and_add("devices", &device_uevent_ops, NULL);
这里指定了devices_kset的目录      /sys/devices



3) dev_kobj = kobject_create_and_add("dev", NULL); //创建 /sys/dev 目录
    sysfs_dev_block_kobj = kobject_create_and_add("block", dev_kobj); //创建 /sys/dev/block 目录
    sysfs_dev_char_kobj = kobject_create_and_add("char", dev_kobj); //创建 /sys/dev/char 目录     

可以看出，/sys/dev目录下将设备分为block设备和char设备，在char目录下放的是以主
设备号命名的文件，每个文件对应唯一的一个设备，注册过的设备都可以在这里面找到。


4) /sys/devices/platform 目录和/sys/bus/platform/devices目录下的文件是一样的。
如果在devices/platform/创建了设备，那么它平台总线上肯定有对应一样的设备，因为
我们的平台设备总是挂载到平台总线上！


2.2 linux平台设备的注册
以mach-anw6410.c文件进行说明.
对于某个平台必定会有一个平台的.c文件和它对应，在系统初始化的时候会调用
	MACHINE_START声明的.init_machine = anw6410_machine_init，在个xxxx_init函数
里面会调用platform_add_devices(anw64_devices, ARRAY_SIZE(anw6410_devices))
	将平台设备添加进内核.

	|
    |--anw6410_machine_init()
	|
	|--platform_add_devices(anw6410_devices, ARRAY_SIZE(anw6410_devices));  //将平台设备注册进去
	|
	|--platform_device_register(devs[i]);   //循环注册平台设备
	|
	|--device_initialize(&pdev->dev);
	|    |
	|    |--dev->kobj.kset = devices_kset;   //这里设置了平台设备的kset 就是 /sys/devices,因为devices_kset在 devices_init()的时候就已经指向 /sys/devices
	|    |
	|    |--dev_set_name(&pdev->dev, "%s.%d", pdev->name,  pdev->id);  //pdev->name 就是板级信息中的设备名称
	|
	|    |--kobject_init(&dev->kobj, &device_ktype);
	|
	|--arch_setup_pdev_archdata(pdev);
	|
	|--platform_device_add(pdev);  //将平台设备添加进内核
	|
    |--	if (!pdev->dev.parent)
	|        pdev->dev.parent = &platform_bus;  //platform_bus 在driver_init() --》 buses_init()的时候就已经注册了
	|
	|--pdev->dev.bus = &platform_bus_type; //设备的总线被定义为平台总线
	|
	|--device_add(&pdev->dev); //添加设备，创建设备文件


关键点：
(1)device_initlialize(&pdev->dev);函数中语句dev->kobj.kset = devices_kset的
devices_kset是哪里来？
在init.c文件中 int __init  devices_init(void)里面得到

(2)platform_device_add(pdev)函数dev_set_name(&pdev->dev,"%s.%d",pdev->name,
		pdev->id);
就是平台设备的名称。


#linux下平台总线驱动四


一：到底是platform_device先注册呢？还是platform_driver先注册？

二：分析平台驱动的注册过程
2.1 platform_driver_register()是什么时候被调用的?

boot加载linux kernel
	|
    |--start_kernel() //启动内核
	|
	|--rest_init();
	|
	|--kernel_thread(kernel_init, NULL, CLONE_FS); //创建kernel_init线程
	|
    |--static int __ref kernel_init(void *unused)
	|
	|--kernel_init_freeable();
	|
	|--do_basic_setup();  //做一些初始化动作
	|
	|--driver_init();
	|
	|--do_initcalls(); //加载 module_init()声明的驱动，然后驱动中的platform_driver_register() 被加载

2.2 platform_driver_register()驱动注册过程分析
以usb20.c为例子进行说明

2.2.1 重要结构体
(1) device_driver
```c
struct device_driver{
	const char *name;
	struct bus_type *bus;
	struct module *owner;
	const char *mod_name;
	....
	....
	....
}
```


```c
struct driver_private{
	struct kobject kobj;
	struct klist klist_devices;
	struct klist_node knode_bus;
	struct module_kobject *mkobj;
	struct device_driver *driver;
}
```
(2)platform_id
```c
struct platform_device_id 
{
	char name[..];
	kernel_ulong_t driver_data;
};
```

(3)platform
```c
struct platform_driver{
	....
	....
	....
};
```

(4)platform_bus_type
struct platform_bus_type = {

}

2.2.2 platform_driver_register()函数分析
定义一个平台驱动mt_usb_driver,名字为mt_usb = {
	.remove = mt_usb_remove,
	.probe = mt_usb_probe,
	.driver = {
		.name = "mt_usb"
	},
};


```c
platform_driver_register(&mt_usb_driver);
|
|--platform_driver_register(drv，THIS_MODULE)
	|
	|--drv->driver.owner = owner;
	|
	|--drv->driver.bus = &platform_bus_type; /* 赋值，驱动所属总线为平台总线 */
	|
	|--drv->driver.probe = platform_drv_probe;/* 如果drv->probe存在，则赋值，这是mt_usb_probe */
	|
	|--drv->driver.remove = platform_drv_remove/* 如果drv->remove存在，则赋值，这是mt_usb_remove */;
	|
	| /* 注册驱动，注意这里传入的参数是&mt_usb_driver.driver名字为mt_usb */
	|--driver_register(&drv->driver);
		|
		|--if((drv->bus->probe && drv->probe) ||)
	这个 地方的笔记先记在这个地方
```














