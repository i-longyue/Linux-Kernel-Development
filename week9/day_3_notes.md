
#Linux-DEVICE_ATTR()介绍及使用示例
##1.介绍
使用**DEVICE_ATTR**,就可以实现驱动在sys目录自动创建文件，我们只需要实现show和
store函数即可。
然后在应用层就能通过cat和echo命令来对sys创建出来的文件进行读写设备驱动，实现
交互。 


##2.DEVICE_ATTR()宏定义
DEVICE_ATTR()定义位于include/linux/device.h中，定义如下所示：
 #define DEVICE_ATTR(_name, _mode, _show, _store) \
	struct device_attribute dev_attr_##_name = __ATTR(_name, _mode, 
			_show，——store)
其中_mode
- 400    拥有者能够读，其他任何人不能进行任何操作；
- 644    拥有者都能够读，但只有拥有者可以编辑；
- 660    拥有者和组用户都可读和写，其他人不能进行任何操作；
- 664    所有人都可读，但只有拥有者和组用户可编辑；
- 700    拥有者能够读、写和执行，其他用户不能任何操作；
- 744    所有人都能读，但只有拥有者才能编辑和执行；
- 755    所有人都能读和执行，但只有拥有者才能编辑；
- 777    所有人都能读、写和执行（该设置通常不是好想法）。
- 当然也可以用S_IWUSR(用户可写)，S_IRUSR(用户可读)等宏代替.


```c
	/* cat命令时,将会调用该函数 */
	static ssize_t show_my_device(struct device *dev,
			 struct device_attribute *attr, char *buf) {
		    return buf;
	}
	
	/* echo命令时,将会调用该函数 */
	static ssize_t set_my_device(struct device *dev,
             struct device_attribute *attr,
		     const char *buf, size_t len) {
		    return len;
	}
	/* 定义一个名字为my_device_test的设备属性文件 */
	static DEVICE_ATTR(my_device_test, S_IWUSR|S_IRUSR, show_my_device, set_my_device);   //定义一个名字为my_device_test的设备属性文件
```

最终将宏展开为:
```c
	struct device_attribute dev_attr_my_device_test = {
		.attr = { .name = "my_device_test", .mode = S_IWUSR|S_IRUSR },
		.show = show_my_device,
		.store = set_my_device,
	}
```
然后通过device_create_file()或者sysfs_create_file()便来创建上面my_device_test
设备文件

##3使用示例
见 ../../rk_3288_5_1/kernel/drivers/char/fishing/fishing.c 文件

