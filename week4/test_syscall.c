/*************************************************************************
	> File Name: test_syscall.c
	> Author: i_longyue
	> Mail: 545641927@qq.com 
	> Created Time: Mon Mar 11 17:08:33 2019
 ************************************************************************/
#include<stdio.h>
/* 定义宏_syscall1 */
#include<linux/unistd.h> 
/* 定义类型time_t */
#include<time.h>

void main(void)
{
	time_t the_time;
	/* 调用time系统调用 */
	the_time = time((time_t *)0);
	printf("the time is %ld\r\n",the_time);


}
