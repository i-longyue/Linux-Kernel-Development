
编译模块的时候用make module
不要使用make
或者make -j18

完成是一个模块的测试


2.内核外编译就好配置内核树


3.增加编译内核的配置选项 ，也就是使用
在配置在表明增加了一项选项。(注册保持干净)


17.2.7 模块参数
	Linux提供了这样一个简单框架----它可以允许驱动程序声明参数.从而用户可以在
	系统启动或者模块装载时再指定参数值,这些参数对于驱动程序属于全局变量.值得
	一提的是模块参数同时也将出现在sysfs文件系统中,这样一样,无论是生成模块参数
	还是管理参数的方式都变得灵活多样了.

	定义一个模块参数可以通过宏module_param()完成:
	module_param(name, type, perm);
	参数name既是用户可见的参数名,也是你模块中存放模块参数的变量名.
	参数type则存放了参数的类型
	最后一个参数perm指定了模块在sysfs文件系统对应文件的权限,该值可以是八进制的格式
	比如0644(所有者可以读写,user可写)
	如果该值是零,则表示禁止所有的sysfs项目.


	你必须使用该宏前进行变量定义.通常使用类似下面的语句完成定义:
	/* 在模块参数控制下,我们允许在钓鱼gan 上用活鱼饵 */

	static int allow_live_bait = 1;              /* 默认功能允许 */
	module_param(allow_live_bait, bool, 0644);   /* 一个Bootlean */
	这个值处于模块代码文件外部,换句话,allow_live_bait是个全局变量.
	有可能模块的外部参数名称不同于它对应的内部变量名称.这时就该使用宏
	module_param_named()定义了:

	module_param_named(name, variable, type, perm);
	参数name是外部可见的参数名称,参数variable是参数对应的内部全局变量名称.
	比如:

	static unsigned  int max_test = DEFAULT_MAX_LINE_TEST;
	module_param_named(maximum_line_test, max_test, int, 0);

	通常,需要一个charp类型来定义模块参数(一个字符串),内核将用户提供的这个字符串
	拷贝到内存,而且将变量指向该字符串,比如:
	static char *name;
	module_param(name, charp, 0);

	通常,需要用一个charp类型来定义模块参数(一个字符串),内核将用户提供的这个字符
	串拷贝到内存,而且将变量指向该字符串.比如:
	static char *name;
	module_param(name, charp, 0)
	如果需要,也可以使用内核直接拷贝字符串到指定的字符数组.宏module_param_string()
	可完成上述任务:

	module_param_string(name, string, len, perm);
	这里参数name为外部参数名称,参数string是对应的内部变量名称,参数len是string命
	名缓冲区长度

	static char species[BUF_LEN];
	module_param_string(specifiles, species, BUF_LEN, 0);

	这些参数序列可通过宏module_param_array()存储在c数组中:
	module_param_array(name, type, nump, perm);

	static int fish[MAX_FISH];
	static int nr_fish;
	module_param_array(fish, int, &nr_fish, 0444);

	你可以将内核参数数组命名区别于外部参数,这时你需使用宏:
	module_param_array_named(name, array, type, nump, perm);

	其中参数和其他宏一致.
	最后,你可以使用MODULE_PARM_DESC()
	static unsigned short size = 1;
	module_param(size, ushort, 0644);
	MODULE_PARM_DESC(size, "the size in inches of the fishing pole")



17.2.8 导出符号表
	模块被载入后,就会被动地连接到内核.注意,它与用户空间中动态链接库类似.只有
	当被显示导出后的外部函数,才可以被动态库调用.在内核中,导出内核函数需要使用
	特殊的指令.EXPROT_SYMBOL和EXPROT_SYMBOL_GPL().

	
	导出的内核符号表被看做导出的内核接口,甚至称为内核API.导出符号相当简单,在
	声明函数后,紧跟上EXPROT_SYMBO()指令就搞定了.

	假定get_pirate_beard_color()同时也定义了一个可访问的头文件中,那么现在在任何
	模块都可以访问它.有一些开发者希望自己的接口仅仅对GPL

	如果你的代码被配置成模块,那么就必须保证当它被编译为模块时,它所有的全部接口
	都已被导出,否则就会产生连接错误




