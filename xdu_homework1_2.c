#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

// 20031211470 杨卓
// vaddr: 虚拟地址信息
// paddr: 物理地址信息
// 返回值: paddr
void mem_addr(unsigned long vaddr, unsigned long *paddr)
{
    int pageSize = getpagesize();//调用此函数获取系统设定的页面大小

    unsigned long v_pageIndex = vaddr / pageSize;//计算此虚拟地址相对于0x0的经过的页面数
    unsigned long v_offset = v_pageIndex * sizeof(uint64_t);//计算在/proc/pid/page_map文件中的偏移量
    unsigned long page_offset = vaddr % pageSize;//计算虚拟地址在页面中的偏移量
    uint64_t item = 0;//存储对应项的值

    int fd = open("/proc/self/pagemap", O_RDONLY);//以只读方式打开/proc/pid/page_map
    if(fd == -1)//判断是否打开失败
    {
        printf("open /proc/self/pagemap error\n");
        return;
    }

    if(lseek(fd, v_offset, SEEK_SET) == -1)//将游标移动到相应位置，即对应项的起始地址且判断是否移动失败
    {
        printf("sleek error\n");
        return; 
    }

    if(read(fd, &item, sizeof(uint64_t)) != sizeof(uint64_t))//读取对应项的值，并存入item中，且判断读取数据位数是否正确
    {
        printf("read item error\n");
        return;
    }

    if((((uint64_t)1 << 63) & item) == 0)//判断present是否为0
    {
        printf("page present is 0\n");
        return ;
    }

    uint64_t phy_pageIndex = (((uint64_t)1 << 55) - 1) & item;//计算物理页号，即取item的bit0-54

    *paddr = (phy_pageIndex * pageSize) + page_offset;//再加上页内偏移量就得到了物理地址
}

const int a = 100;//全局常量

int main()
{   
    // 这里定义的变量会占用内存，用来测试线程的物理地址的变化是否符合预期
    int b = 100;
    static c = 100;
    const int d = 100;
    char *str = "Hello World!";

    unsigned long phy = 0;//物理地址
    char *p = (char*)malloc(100);//动态内存
    int num = 1;
    printf("你想要创建几个子进程?");
    scanf("%d",&num);
    while(num < 0 || num > 10){
        printf("输入一个正常一点的数值吧！");
        scanf("%d",&num);
    }
    int i =0;
    for (i ; i<num;i ++){
        printf("进程%d 的信息:", i+1);
        int pid = fork();//创建子进程
        // if(pid == 0)
        // {
        //     //p[0] = '1';//子进程中修改动态内存
        //     mem_addr((unsigned long)&a, &phy);
        //     printf("pid = %d, virtual addr = %x , physical addr = %x\n\n", getpid(), &a, phy);
        // }
        // else
        { 
            mem_addr((unsigned long)&a, &phy);
            printf("pid = %d, virtual addr = %x , physical addr = %x\n\n", getpid(), &a, phy);
        }
    }

    sleep(5);
    free(p);
    waitpid();
    return 0;
}