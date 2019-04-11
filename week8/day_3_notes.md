Linux设备驱动模型之PLATFORM(平台)总线详解

1.什么是platform(平台总线)?

platform总线是一种虚拟，抽象出来的总线，实际中并不存在这样的总线.

那么为什么需要platform总线呢？其实是Linux设备驱动模型为了保持设备
驱动的统一性而虚拟出来的总线。

并不是所有的设备都能够归属这些常见的总线，在嵌入式系统里面，SOC
系统中集成的独立外设控制器，挂接在SOC内存空间的外设，却不依附与
此类总线。所以Linux驱动模型为了保持完整性，将这些设备挂在一条
虚拟的总线上(platform),而不至于使得有些设备挂在总线上(platform)
,而不至于使得有些设备挂在总线上，另一些设备没有挂在总线上。


platform总线相关代码：driver/base/platfrom
相关结构体定义：include/linux/platform_device.h



2.platform总线管理下的2员大将

(1)两个结构体platform_device和platform_driver
对于任何一种Linux设备驱动模型下的总线都由2个部分组成：描述设备相关的
结构体和描述驱动相关的结构体

在platform总线下就是platform_device和platform_driver,下面是对两个结构
体的各个元素进行分析：

platform_device结构体：(include/linux/platform_device.h)
	struct platform_driver{
		const char *name;          /* 平台的名字 */
		int         id;            /* id是用来区分如果设备名字相同的时候...*/ 

		struct device  dev;
		u32 num_resources;         /* 资源结构体数量 */
		struct resource *resource  /* 指向一个资源结构体数组 */
		const  struct platform_device_id  *id_entry   /* 用来进行与设备驱动*/

		struct pdev_archdata archdata; /* 添加自己的东西 */
	}

platform_device 结构体中的struct resource结构体分析：

struct resource {
	resource_size_t start;
	resource_size_t end;
	
	const char *name;
	unsigned long flags;
	struct resource *parent, *sibling, *child;
}


platform_driver结构体
struct platform_driver {
	int (*probe) (struct platform_device *);
	int (*remove) (struct platform_device *);
	int (*shutdown) (struct platform_device *);
	int (*suspend) (struct platform_device *, pm_message_t state);
	int (*resume) (struct platform_device *);
	struct device_driver driver;               /* 内置device_driver结构体 */
	const struct platform_device_id  *id_table;
}
(2)两组接口函数(driver/base/platfrom.c)
	int platform_driver_register(struct platform_driver *);
	void platform_driver_unregister(struct platform_driver *);

	int platform_device_register(struct platform_device *);
	void platform_device_unregister(struct platform_device *);


3.platform平台总线初始化 

(1)platform平台总线注册初始化：platform_bus_init
	early_platform_cleanup      进行一些早期的平台清理
	device_register            注册设备(在/sys/devices/目录下建立platform目录
								对应的设备对象/sys/devices/platform/)
	bus_register               总线注册


(2)相关结构体
struct bus_type {
	const char *name;                     总线名字
	struct bus_attribute *bus_attrs       总线属性
	struct device_attribute *dev_attrs;   总线下设备的属性
	struct driver_attribute *drv_attrs;   总线下设备驱动的属性

	/* 该总线下设备与设备驱动的匹配 */
	int (*match) (struct device *dev, struct device_driver *drv);

	/* 事件函数 热拨插 */
	int (*uevent) (struct device *dev, struct kobj_uevent_env *env)

	int (*probe)(struct device *dev)
	int (*remove)(struct device *dev)
	int (*shutdown) (struct device *dev)
	int (*suspend)(struct device *dev,pm_message_t state)
	int (*resume)(struct device *dev);

	/* 电源管理相关的 */
	const struct dev_pm_ops *pm; 

	/* 总线的私有数据p->subsys.kobj 表示该总线在驱动模型中的对象  */
	struct bus_type_private *p;
}

struct bus_type_provate {
	
	struct kset subsys;              这个是bus主要的kset
	struct kset *drivers_kset;	     这个kset指针用来指向总线的driver目录的
	struct kset *devices_kset;       这个是kse指针用来指向该总线的devices目录

	struct klist klist_devices;     用来挂接该总线下的设备的一个链表头
	struct klist klist_drivers;     用来挂接该总线下的设备驱动的一个链表头
	struct blocking_notifier_head bus_notifier
	unsigned int drivers_autoprobe:1; 是否需要在设备驱动注册时候子自动匹配
	struct bus_type *bus;           指向本bus结构体
};


(3)函数详解
bus_register:

int bus_register(struct bus_type *bus)
{
	int retval;              
	struct bus_type_private *priv;   定义一个bus_type_private结构体指针。

	/* 申请分配内存 */
	priv = kzalloc(sizeof(struct bus_type_private), GFP_KERNEL);

	priv->bus = bus; 使用priv->bus
	bus->p = priv;   通过bus->p 指向priv 这里其实就是将bus与priv建立关系


	/* 给我们的bus在设备驱动 模型中的对象设置*/
	retval = kobj_set_name(&priv->subsys.kobj,"%s",bus->name);

	/* 这里就是对bus的私有数据进行一些填充 */
	priv->subsys.koj.kset = bus_kset;

	/* 设置bus对象 对象类型为bus_ktype */
	priv->subsys.kobj.ktype = &bus_ktype;

	/* 配置为在注册设备*/
	priv->drivers_autoprobe = 1;

	/* 注册kset结构体 */
	retval = kset_register(&priv->subsys)

	/* 在bus下建立属性文件*/
	retval = bus_create_file(bus, &bus_attr_uevent);

	/* 通过priv->devices_kset*/
	priv->devices_kset = kset_create_and_add("devices", NULL,
			&priv->subsys.kobj);

	/* 初始化链表 */
	klist_init(&priv->klist_devices, klist_devices_get, klist_devices_put);
	klist_init(&priv->klist_devices, NULL, NULL);

	retval = add_probe_files(bus);
}


4.platform平台注册函数 
(1)platform平台总线注册函数：platform_device_register
device_initialize
platform_device_add
device_add

(2)函数分析
int platform_device_add(struct platform_device *pdev)
{
	int i,ret = 0;
	
	if (!pdev)
		return -EINVAL;

	if (!pdev->dev.parent)
		/* 将平台设备的父设备为platform_bus */
		pdev->dev.parent = &platform_bus; 

	pdev->dev.bus = &platform_bus_type;

	if(pdev->id != -1)
		...
			...
}


5.platform平台设备驱动注册
(1)platform平台设备驱动注册函数:platform_driver_register
plplatform_driver_register
	driver_register
		driver_find
		bus_add_driver
			kobject_init_and_add
			driver_attach
			klist_add_tail
			module_add_driver
			driver_create_file
			driver_add_attrs
		driver_add_groups


(2)函数详解
platform_driver_register：
int platform_driver_register(struct platform_driver *drv)
{
	drv->driver.bus = &platform_bus_type;

	/* 下面做的就是对drv中函数指针进行填充 */
	if(drv->probe)
		drv->driver.probe = platform_drv_probe;

	if(drv->remove)
		drv->driver.remove = platform_drv_remove;

	if(drv->shutdown)
		drv->driver.shutdown = platform_drv_shutdown;

	/* 注册设备驱动 */
	return driver_register(&drv->driver);
}


driver_register
int driver_register(struct device_driver *drv)
{
	int ret;

	/* 定义一个设备驱动指针 */
	struct device_driver *other;

	/*这个函数其实进行了一个校验，对比相当的总*/
	other = driver_find(drv->name, drv->bus);

	/* 在总线挂接设备驱动 就是将设备驱动对应的kobj对象与组织建立关系 */
	ret = bus_add_driver(drv);
}



bus_add_driver


driver_attach(drv) 尝试将驱动绑定到设备，也就是通过这个函数 进行设备与设备
驱动的匹配。


上面说到了当注册platform平台设备驱动时会进行自动匹配原理，
platform_device_add
	device_add
	 bus_probe_device

	

#总结：所以由此可知，当我们不管先注册设备还是先注册设备驱动都会进行一次
#设备和设备驱动的匹配过程，匹配成功之后就会调用 probe函数。

#匹配的原理就是去遍历总线下相应的链表来找到挂接在他下面的设备或者设备驱动，


6.platform 总线下的匹配函数 
总结： 由上面可知platform总线下设备与设备驱动的匹配原理就是通过名字进行匹配
的，先去匹配platform_driver中的id_table表中的各个名字与platform_device->name
名字是否相同，如果相同表示匹配成功直接返回，否则直接匹配
platform_driver->name与platform_driver->name是否相同，
相同则匹配成功，否则失败。







	






