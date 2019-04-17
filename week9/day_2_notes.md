
#31
usb_stor_xfer_buf()这个函数所做的事情就是从srb->request_buffer往
buffer里边复制数据，或者反过来从buffer往srb->request_buffer然后返
回复制了多少个字节。对于offset大于等于request_bufflen的情况.

这个参数其实是很简单的一个枚举数据类型，含义也很简单：一个表示
向srb->request_buffer(XFER就是TRANSFER的意思)

	针对queuecommand传递进来INQUIRY命令的情况。

#32
	解决了这个INQUIRY的问题，我们就可以继续往下走了，这就是真正的批量
	传输的地方，proto_handler()就是正儿处理SCSI命令的函数指针。



#33

#34.迷雾重重的批量传输
	us->proto_handler()其实就是一个函数指针，知道它指向什么呢？我们早在
	storage_probe(),在get_protocol就赋值了。当时只知道是get protocol。
	对于u盘被赋值为usb_stor_transparent_scsi_command.

#35
	us->transport(),这个函数指针同样在storage_probe时被赋值，对于U盘，它
	遵守的是Bulk-Only协议，因此us->transport()
	被赋值为usb_stor_Bulk_transport().来看usb_Bulk_transport)


#36 在usb_stor_Bulk_transport中，这个函数中调用的第一个最重要的函数，
	那就是usb_stor_Bulk_transfer_buf()
	我们要结合usb_stor_Bulk_transport()中usb_stor_bulk_transfer_buf被
	调用的上下文.
#37
#38
#39
	usb_stor_bulk_transfer_sglist()函数有一定的"蛊惑性",sg的目的就是让一
	堆不连续的buffers在一次DMA操作都传输出去。 

#40 
	如何处理CSW，usb_stor_bulk_transfer_buf函数再一次被调用。这次是获得
	CSW，期望长度是US_BULK_CS_WRAP_LEN,这个宏来自driver/usb/storage/
	transport.h
	一共有三个阶段：Command Data Status这里执行的命令就是WRITE_10,本来
	这是一次成功的传输。
#41
	我们知道SCSI通过命令通信，有一个命令是Request Sense.它是用来获取
	错误信息的，不知道为什么，有人把错误信息称为“sense data”
	如果一个设备收到一个request Sense命令，将返回sense data
