// 说明：
//     1. 我的实现方法中，X没有读文件，而是提前设定好一个长字符串进行处理
//     2. 我手动设定每个共享内存空间仅允许写入20个字符，这样打印结果比较整齐
//
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/shm.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem_mutex1;// 表示对缓冲区1的互斥操作
sem_t sem_mutex2;// 表示对缓冲区2的互斥操作
sem_t s1;// 信号量s1表示缓冲区区还可以放入的产品
sem_t s2;// 信号量s2表示缓冲区已经放的产品

#define NAME_LEN 20

typedef struct {// 初始化共享内存空间使用的结构体
	char name[20]; // 杨卓
	int id;// 200312*****
}yz;

struct buff_struct{
    int write_num; // 好像没用上，这应该是之前瞎写的
    int id;
};


// 杨卓 200312*****
// 说明: 生成一个共享内存区
// init_key: 生成共享内存区使用的键值
// ipc_size: 共享内存区大小，字节
// flag: 操作权限，默认666
// 返回值: 生成的共享存储区的标识ID
int sharememory(int ipc_size,int flag)
{
	int id;
	key_t key=ftok("/tmp",65+ rand()%10);
	if(key < 0)
	{
		printf("get key error\n");
		return -1;
	}
	id = shmget(key,ipc_size,flag);
	if(id < 0)
	{
		printf("get id error\n");
		return -1;
	}
	return id;
}
 
int create_ipc(int ipc_size)
{
	return sharememory(ipc_size,IPC_CREAT|IPC_EXCL|0666);
}
int get_ipc(int ipc_size)
{
	return sharememory(ipc_size,IPC_CREAT|0666);
}
int destroy_sharememory(int id)
{
	return shmctl(id,IPC_RMID,NULL);
}
// 共享内存空间涉及到的一个结构体
// text: 写入的文本信息
// Written: 没用上的变量
// id: 保存的是共享内存空间标识符
// size: 记录当前共享内存空间的剩余信息
struct shm_data{ 
	char text[20];
	int written;
	int id;
	int size;
}shm_data_struct;

// pthread_create的坑，只能传一个参数，所以将两个共享内存空间再次组合为一个结构体，套娃
// file_path 原计划是输入一个文件路径，但是实在是懒
// flag, flag2 用于记录当前共享内存空间是否已经写入
// 共享内存空间结构体
struct share_2_part{
    char file_path[30];
    char flag;
    char flag2;
	struct shm_data *share1;
	struct shm_data *share2;
};

// 读取文件写入共享区的 X
void* X(void *arg)
{  	
	int start = 0;
	int len = 20; 
    struct share_2_part *share;
    struct shm_data *ts1;
    struct shm_data *ts2;
    ts1 = (struct shm_data *)malloc(sizeof(struct shm_data));

    share = (struct share_2_part *)arg;
    ts1 = share->share1;
    ts2 = share->share2;
    
    int now_posi = 0;
    char l_str[500];
    // 这里我没有使用读文件操作，而是自定义了一个非常长的字符串，纯粹是偷懒
    strcpy(l_str,"1234567890qwertyuiopasdfghjklzxfjpasdjojoAJSDPIjpaJPDAJSdopjaodpJPOSdjAOPDJAOPdsjOPAJDOASMDKLAIFNKASNDIAJDADJklJIASJDjlJASldjAadasdcvbnmQWERTYUIOPASDFGHJKLZXCVBNMadjsjdidsadiwduhOIDHIJAIDnsiaDISdIJSLDKpiajdPIJSDADilAJDINDJSpdajdajdAJDOSJD#end");

    while(1){
        sem_wait(&sem_mutex1); // 拿到缓冲区1
        if (share->share1->size > 0){
            sem_wait(&s1);
			
			
            if (now_posi < strlen(l_str) - 20){
                printf("\n\n\n\n ===== X 正在运行共享内存区1 ，剩余空间%d ===== \n\n",share->share1->size);
                share->share1->size = 0;
                share->flag = 1;
                // share1->written = 1;
                
                
                char str[20];
                
                strncpy(str,l_str + now_posi, 20);
                printf(" ===== 写入的数据是: %s =====\n\n",str);
                strcpy(share->share1->text,str);
                now_posi += 20;
                printf(" =========== X 写入完毕，剩余空间%d ============ \n",share->share1->size);
            }
			sem_post(&s1);
        }
		sem_post(&sem_mutex1);

        sem_wait(&sem_mutex2); // 拿到缓冲区2
        if (share->share2->size > 0){
            sem_wait(&s2);
			
			
            if (now_posi < strlen(l_str) - 20){
                printf("\n\n\n\n ===== X 正在运行共享内存区2 ，剩余空间%d ===== \n\n",share->share2->size);
                share->share2->size = 0;
                share->flag2 = 1;                
                
                char str[20];
                
                strncpy(str,l_str + now_posi, 20);
                printf(" ===== 写入的数据是: %s =====\n\n",str);
                strcpy(share->share2->text,str);
                now_posi += 20;
                printf(" =========== X 写入完毕，剩余空间%d ============ \n",share->share2->size);
            }
			sem_post(&s2);
        }
		sem_post(&sem_mutex2);
		sleep(rand()%3); // 一般来说写入文件比读取慢，这里使用了sleep模拟写入瓶颈，也更便于观察
    }
    return NULL;
}


void* Y(void *arg)
{  	
	int start = 0;
	int len = 20; 
    struct share_2_part *share;
    struct shm_data *ts1;
    struct shm_data *ts2;
    ts1 = (struct shm_data *)malloc(sizeof(struct shm_data));

    share = (struct share_2_part *)arg;
    char out_str[20];
    while(1){
        int i = 0;
        sem_wait(&sem_mutex1); // 共享内存区1
        
        if (share->share1->size == 0){ 
            sem_wait(&s1);
			
            if (share->flag == 1){// 可以进行读取操作
                printf("\n\n\n\n ===== Y 正在读取共享内存区1 ，待读取%d ===== \n\n",20-share->share1->size);
                share->share1->size = 20;
                share->flag = 0;                
                strcpy(out_str,share->share1->text);
                
                printf(" ===== 读取的数据是: %s ===\n\n",out_str);

                for (i = 0; i < 20; i++) {
                    if (out_str[i] >= 'a' && out_str[i] <= 'z') {
                        out_str[i] -= 'a' - 'A';
                    }
                }
                printf(" == 处理完毕后的结果: %s ==\n\n",out_str);
                printf(" ========== Y 读取完毕，剩余空间%d ========== \n",share->share1->size);
            }
			sem_post(&s1);
        }
		sem_post(&sem_mutex1);


        sem_wait(&sem_mutex2); // 共享内存区2
        if (share->share2->size == 0){
            sem_wait(&s2);

            if (share->flag2 == 1){// 可以进行读取操作
                printf("\n\n\n\n ===== Y 正在读取共享内存区2 ，待读取%d ===== \n\n",20-share->share2->size);
                share->share2->size = 20;
                share->flag2 = 0;                
                
                strcpy(out_str,share->share2->text);
                
                printf(" ===== 读取的数据是: %s ===\n\n",out_str);

                for (i = 0; i < 20; i++) { // 小写转大写操作实现
                    if (out_str[i] >= 'a' && out_str[i] <= 'z') {
                        out_str[i] -= 'a' - 'A';
                    }
                }
                printf(" == 处理完毕的结果: %s ===\n\n",out_str);
                printf(" ========== Y 读取完毕，剩余空间%d ========== \n",20-share->share2->size);
            }
			sem_post(&s2);
        }
		sem_post(&sem_mutex2);
    }
    return NULL;
}
        


// 200312***** 杨卓
int main()
{	

	void * shm = NULL;
	struct shm_data *shared;
	int shmid;

	int id=create_ipc(sizeof(yz)); // 创建共享内存空间1
	shmid = id;
	shm = shmat(shmid,0,0);

	shared = (struct shm_data*)shm;// 用shared 来保存共享空间1
	shared->written = 0;
	shared->id = id;
	shared->size = 20;


	void * shm2 = NULL;
	struct shm_data *shared2;
	int shmid2;

	int id2=create_ipc(sizeof(yz)+1);// 创建共享内存空间1
	shmid2 = id2;
	shm2 = shmat(shmid2,0,0);
	shared2 = (struct shm_data*)shm2;// 用shared2 来保存共享空间2
    
    // 辅助信息
	shared2->written = 0; 
	shared2->id = id2; // 保存的是共享内存空间的ID信息
	shared2->size = 20;

    shared->written = 0;
	shared->id = id; // 保存的是共享内存空间的ID信息
	shared->size = 20;
    printf("====================================================\n\n");
    printf("当前创建了2个共享内存区，程序出错or意外退出需要手动处理\n"); // 避免因为意外关闭程序或陷入死循环，需要手动清空信息
    printf("ID是: %d 和 %d, 使用ipcrm -m ID处理\n\n",id,id2);
    printf("这里我没有使用读取文件操作，而是滚键盘输入了一个超长字符串，效果反正一样的\n");
    printf("模拟文件的字符串：1234567890qwertyuiopasdfghjklzxfjpasdjojoAJSDP200312*****IjpaJPDAJSdopjaodpJPOSdjAOPDJAOPdsjOPAJDOASMDKLAIFNKASNDIAJDADJklJIASJDjlJASldjAadasdcvbnmQWERTYUIOPASDFGHJKLZXCVBNMadjsjdidsadiwduhOIDHIJAIDnsiaDISdIJSLDKpiajdPIJSDADilAJDINDJSpdajdajdAJDOSJD");
    printf("\n====================================================\n\n");
    struct share_2_part *share;
    share = (struct share_2_part *)malloc(sizeof(struct share_2_part));

    share->share1 = (struct shm_data*)malloc (sizeof(shared));//要向线程传入共享内存空间1，共享内存空间2的信息
    share->share2 = (struct shm_data*)malloc (sizeof(shared2)); // 这里用的结构体实现，pthread_create的传参有坑，只能这样
    share->share1 = shared;
    share->share2 = shared2;

    // 最后传入线程的是share，这个结构体包含了两个共享内存空间
	pthread_t xid1,yid1;
    sem_init(&s1,0,1); // s1是针对共享缓存区1的信号量
    sem_init(&s2,0,1); // 
    sem_init(&sem_mutex1,0,1); // 保证互斥访问
    sem_init(&sem_mutex2,0,1);
    pthread_create(&xid1,NULL,X,(void*)share); //创建X 和Y线程
    pthread_create(&yid1,NULL,Y,(void*)share);
    pthread_join(xid1,NULL);
    pthread_join(yid1,NULL);
}
