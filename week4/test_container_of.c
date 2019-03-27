/*************************************************************************
	> File Name: test_container_of.c
	> Author: i_longyue
	> Mail: 545641927@qq.com 
	> Created Time: Tue Mar 12 10:11:39 2019
 ************************************************************************/
#include<stdio.h>

#define offsetof(TYPE,MEMBER)   ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(PTR,TYPE,MEMBER)    ({  \
		    const typeof(((TYPE *)0)->MEMBER) *__mptr=(PTR);  \
		    (TYPE *) ((char *)__mptr - offsetof(TYPE,MEMBER)); })



struct test{
	int num;
	char ch;
};

int main(int argc, char *argv[])
{
	struct test t1={100,'c'};
	char *pch = &t1.ch;

	struct test *ptt = container_of(pch, struct test,ch);
	printf("num = %d \n",ptt->num);
	return 0;

}
