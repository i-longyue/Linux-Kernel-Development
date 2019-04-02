#第20章 
	补丁,开发和社区Linux的最大优势就是它紧密团结了众多开发者,社区能帮你检查代码
	社区中的专家给 你提出忠告,社区中的用户能帮你进行测试,用户还能向你反馈问题.
	更重要的是,什么样 的代码可以加入linus的官方内核树也是由社区做出决定的.因此
	了解系统到底是怎么 运作就显得重要.  20.1 社区 如果一定要让Linux内核社区在现
	实中找到它的位置,那它也许会叫做内核邮件列表 (Linux Kernel Mailing List)之家
	.内核邮件列表(或者简写成lkml)是对 内核进行发布,讨论,争辩和打口水的主战场.在
	做任何实际动作之前,新特性会在此处被 讨论.新代码大部分也会在此处张贴.这个列
	表每天发布的消息超过300条,所以决不适合 心血来潮的玩主.任何想踏实,认真开发内
	核的人都应该订阅它.单单看看这些奇才们使出 的一招一式,也能让你受益匪浅了. 

#20.2 Linux编码风格	codingStyle 

#20.2.1 缩进
```
int main()
{
1234int a;   
}
```

#20.2.2 switch语句
switch (x) {
case:
	break;
default:
	break;
}

#20.2.3 空格
	空格放在关键字周围,函数名和圆括号之间无空格
	if (foo)
	while (foo)
	for(i = 0, i < NR_CPUS; i++)
	switch (foo)

	相反,函数,宏以及与函数想像的关键字(如sizeof typeof alignof)在关键字和圆括号
	之间没有空格.
	wake_up_porcess(task);
	size_t nolongs = BTS_TO_LONG(nbits);
	int len = sizeof(struct task_struct);
	typeof(*p)
	__alignof__(struct sockaddr *)
	__attribute__((packed))

###在括号内,如前所示,参数前后也不回空格,例如,下面这样是禁止的:
```
***int prio*** = task_prio( task ); /* BAD STYLE */
```
###对于大多数二元或者三元操作符,在操作符的两边加上空格，例如：
```
int sum = a + b;
int product = a * b;
int mod = a % b;
int ret = (bar) ? bar : 0;

return (ret ? 0 : size);
int nr = nr ？：1; /* allowed shortcut,same as "nr ？nr :1" */
if (x < y)
if (task-》flags & PF_SUPERPRIV)
mask = POLLIN | POLLRDNORM;
```
###相反，对于大多数操作符，在操作符和操作数之间不加空格：
if(!foo)
int len = foo.len;
foo++;

###在提领运算符的周围加上合适的空格,正确的风格：
char *strcpy(char *dest, const char *src)


#2.2.4 花括号
	花括号的使用不存在技术的
	if (strcmp(buf, "NO_",3) == 0) {
		...
	}
最后不需要使用括号的语句可以忽略它：
if (cnt <63)
	cnt = 63;

#20.2.5 每个行代码的长度
static void get_new_parrot(const char *name,
						      unsigned long disposition,
							  unsigned long feather_quality)

#20.2.6 命名规范
	小写字母，要用含义，这里是Linux不是BSD.
#20.2.7 函数 
	函数的代码长度不应该超过两个屏幕，局部变量不应超过10个。
	如果你担心函数调用导致的开销，可以使用inline关键字。
#20.2.8 注解
/**/
#20.2.9 typedef 
	内核开发者强烈反对使用typedef语句。
#20.2.10 多用现在的东西
#20.2.11 在源码中减少使用ifdef
绝不应该在自己的函数 使用如下的实现方法：
#ifdef CONFIG_FOO
foo();
#endif
相反，应该采取的方法是在CONFIG_FOO没定义的时候让foo()函数为空。

#20.2.12 结构体初始化 
struct foo my_foo = {
	.a = ;
}

#20.2.13 代码事后的修正
indent -kr -i8 -ts8 -sob -l80 -ss -psl <file>

#20.3 管理系统 
CREDITS文件

#20.4 提交错误报告
如果碰到一个bug，最理想的应对无疑是写出修正代码创建补丁，测试后提交它，这个流程
会在20.5节会仔细介绍。当然，也可以报告这个问题，然后让其他人替你解决。
提交一个错误报告最重要的莫过于对问题进行清楚的描述。要讲清楚症状，系统输出信息
完整并经过解码的oops。更重要的是，你应该尽可能地提供能够准确重现这个错误的步骤 
并提供你的机器硬件配置基本信息。

MAINTAINERS文件列举了每个相关设备驱动和子系统单独信息----接收关于其所有维护的
代码的所有问题。如果找不到对此感兴趣的人
文档REPORTING-BUGS和Documentation/oops-tracing.txt中更多相关信息。

#20.5补丁
	对内核的任何修改都是心补丁形式发布的。而补丁其实是GNU diff(1)程序的一种特定
	格式的输出，该格式的信息能够被patch(1)程序接受。

#20.5.1 创建补丁
	创建补丁最简单的办法是通过两分内核源代码进行，一份源码，另一份是加进了所修
	改部分的源代码。一般会给原来的内核代码起名linux-x.y.z(其实就是把源代码包解
			压缩后所得到的文件夹),而修改过的就真起名为linux.然后利用下面的命令
	通过这两份代码 创建补丁：
	diff -urN linux-x.y.z/ linux/ > my-patch
	
	你现在可以在自己的目录下运行该命令，一般都是在home目录下，而不是在/usr/src/
	linux目录下进行这种操作，所以不一定必须具备root权限。通过 -u参数指定使用特
	殊的diff输出格式，而-N参数指明做出修改的源代码中所有加入的文件在diff操作时
	会包含在内。另外，如果你想对一个单独的文件进行diff,你也可以这么做
	diff -u linux-x.y.z/some/file linux/some/file >my-patch
	
	注意，在你自己代码所在的目录下执行diff很重要。这样创建的补丁别人用起来更方
	便，执行一个这样生成的补丁，只需要在你自己代码树的根目录执行下列命令就可以
	了：
	patch -p1 < .../my-patch
	
		在这个例子中，补丁的名字叫my-patch,它位于当前目录的上一层目录中
		-p1参数用来剥去补丁中头一个目录的名称。这么做的好处是可以在打补丁的
		时候忽略创建补丁的人的目录命名习惯。

		diffstat是一个很有用的工具，它可以列出补丁所引起的变更的统计
		(加入或移动的代码行).输出关于补丁的信息，执行：
		diffstat -p1 my-patch
		在lkml贴出自己的补丁时，附带上这份信息往往会很有用。由于patch(1)会忽略
		第一个diff之前的所有内容，所以你甚至可以在patch的最前面直接加上简短的
		说明。

#20.5.2 用Git创建补丁
		如果你用Git管理源代码树，你照样需要Git创建补丁----也就是没有必要按部就
		班把上述手工的步骤操作一遍，但是你需要忍受Git的复杂性。用Git创建补丁并
		不是什么难事，只需要两个过程。首先你必须是修改者，然后在本地提交你的修
		改。把修改提交到Git树与提交到标准的源代码树并没有什么

		把所做的修改提交到你的Git版本库：
		git commit -a
		
		-a参数表示提交所有的修改。如果你仅仅想提交某个指定文件的修改
		git commit some/file.c

		但是即使有了-a参数，Git并不立即提交新文件，直到把它们添加到版本库中才提
		交。要增加一个文件，然后再提交
		git add some/other/file.c
		git commit -a
		
		当执行Git的commit命令时，Git会要求输入一个更改日志。
		下面命令只为最后一次提交产生一个补丁：
		git format-patch -1


#20.5.3 提交补丁
		补丁可以按照20.5.2节描述创建。如果补丁涉及了某个特定的驱动程序或子系统

		一般包含一份补丁的邮件，它的主题一栏内容应该以[PATCH]简要说明的格式写
		出。邮件的主体部分应该描述所做的改变的技术细节，以及要做这些的原因，在
		E-mail中还要注明补丁对应的内核版本。

#20.5 小结
	对于黑客而言，最可贵的品质便是渴望----如身上痒痒，本书讲述了Linux内核的主要 
	部分，讨论了接口，数据结构，算法和原理，它从实践出发，心内在的视角洞悉内核，
	既可以满足你的好奇心，也可以帮助你开始学习内核。


注意：用git diff查看，发现是，
	$ git diff util/webkit/mkdist-webkit
	diff --git a/util/webkit/mkdist-webkit b/util/webkit/mkdist-webkit
	old mode 100755
	new mode 100644
	
原来是msysgit在windows下需要为文件"仿造"访问权限。由于种种限制，信息不能复原
从而导致原来的755成644了。

解决方法：
	git config --global core.filemode false
	git config core.filemode false


###
```

```
###



