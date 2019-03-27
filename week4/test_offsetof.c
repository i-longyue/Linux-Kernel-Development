/*************************************************************************
	> File Name: test_offsetof.c
	> Author: i_longyue
	> Mail: 545641927@qq.com 
	> Created Time: Tue Mar 12 10:42:27 2019
 ************************************************************************/

#include<stdio.h>
struct test 
{
	char i;
	char j;
	char k;
};

int main(void)
{
	struct test temp;
	printf("&temp =%p\n",&temp);
	printf("&temp.k =%p\n",&temp.k);
	printf("&((struct test *)0)->k =%d\n",((size_t)&((struct test *)0)->k));


}
