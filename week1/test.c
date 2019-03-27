#include <stdio.h>
#include <unistd.h>

int main(void)
{
	pid_t pid;
	printf("hello world\n");

	/* 从fork开始就已经产生子进程了 */
	/* 就已经产生了新的4G空间，复制空间。 */
	/* 创建出来的子进程是父进程的一个副本，除了进程号，父进程号和子进程号
     * 不同。
   	 */
	pid = fork();
	printf("hello ketty\n");
	if (pid == 0)
	{
		/* 子进程运行 */
		printf("child curpid:%d parent pid:%d\n",getpid(),getpid());
		return 0;
	}

	/* 父进程运行 */
	printf("parent curpid:%d parent pid:%d\n",getpid(),getpid());
	return 0;

}
