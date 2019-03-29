# 这个一个关于Linux内核设计与实现一书的一些笔记和总结。
## 第19章 可移植性

```c
char wolf[] = "like a wolf";
char *p = &wolf[1];
unsigned long l = *(unsigned long *)p;
```
