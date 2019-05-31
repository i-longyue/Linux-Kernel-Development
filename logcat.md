
# Logcat 工具
	Android 日志系统提供了记录和查看系统调试信息的功能。日志都是从各种软件和一些缓冲区记录下来的，缓冲区通过Logcat查看和使用。Logcat是调试程序用的最多的功能。该功能主要是通过打印日志来显示程序的运行情况。由于打印日志量非常大，需要对其进行过滤操作。



## logcat *:w
示例说明：显示优先级为warning或者更高的日志信息。
 控制日志标签和输出优先级;

 

 

## 控制日志标签和输出优先级
 示例如下：
 logcat ActivityManager:I MyApp:D *:S
 示例说明：支持所有的日志信息，除了那些标签为“ActivityManager"和优先级为"INF
 O" 以上的，标签为"MyAPP"和”优先级为"Debug"以上的。



## 只输出特定标签的日志
 示例如下:
 logcat wishTV:* *:s

 或者
 logcat -s WishTV
 示例说明：只输出标签为WishTV的日志。



## 只输出指定优先级和标签的日志
 logcat WishTV:I *:s
 示例说明：只输出优先级为I，标签为wish TV的日志。



## 查看上次LOG
 可以加-L参数来打印出上次系统复位前的logcat信息。若出现拷机异常掉电的情况,
 可通该命令打印出上一次Android运行状态的日志。命令如下:

 logcat -L

 注意：什么是拷机异常：






 



