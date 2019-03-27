/*************************************************************************
	> File Name: test_fifo.c
	> Author: i_longyue
	> Mail: 545641927@qq.com 
	> Created Time: Tue Mar 12 19:42:06 2019
 ************************************************************************/
#include<stdio.h>

void main(void)
{
	unsigned int i;

	/* 将[0,32]压入到名为'fifo'的kfifo中 */
	for(i = 0; i<32; i++)
		kfifo_in(fifo, &i, sizeof(i));

	unsigned int val;
	int ret;
	ret = kfifo_out_peek(fifo, &val, sizeof(val), 0);

	if(ret != sizeof(val))
		return -EINVAL;

	printk(KERN_INFO "%u\n", val);


}
