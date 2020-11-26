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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sem.h>
#define min(a,b) ( ((a)>(b)) ? (b):(a) )

typedef struct {// 初始化共享内存空间使用的结构体
	char name[20]; // 杨卓
	int id;// 20031211470
}yz;

struct shm_data{ 
	char text[20];
	int written;
	int id;
	int size;
    
}shm_data_struct;

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *arry;
};


static int sem_id = 0;
static int sem_id2 = 0;
static int set_semvalue();
static void del_semvalue();
static int semaphore_p();
static int semaphore_v();

int main(){
    int shm_id;
    struct shm_data *shared;
    shm_id = shmget (666, sizeof(yz), IPC_CREAT);
    if(shm_id == -1){
        perror("shmget()");
    }
    shared = (struct shm_data *)shmat(shm_id, 0, 0);
    int shm_id2;
    struct shm_data *shared2;
    shm_id2 = shmget (1234, sizeof(yz), IPC_CREAT);
    if(shm_id2 == -1){
        perror("shmget()");
    }
    shared2 = (struct shm_data *)shmat(shm_id2, 0, 0);


    // X创建信号量1,2
    int i = 0;
    sem_id = semget((key_t)2016, 1, 0666 | IPC_CREAT);
	sem_id2 = semget((key_t)1998, 1, 0666 | IPC_CREAT);
    if(!set_semvalue(sem_id)){
		fprintf(stderr, "初始化信号量错误\n");
		exit(EXIT_FAILURE);
	}
	printf("\n信号量1创建成功\n");
	if(!set_semvalue(sem_id2)){
		fprintf(stderr, "初始化信号量错误\n");
		exit(EXIT_FAILURE);
	}
	printf("\n信号量2创建成功\n");
    int loop = 1;
    int now_posi = 0;
    char output[500];
    strcpy(output,"");
    while(loop){
        char out_str[20];
		//进入临界区1
        while(!semaphore_p(sem_id)){
            
        }
		if (shared->written == 1 && now_posi <= shared->size){
			printf("\n\n=====Y 进入临界区1=====\n");
			strcpy(out_str,shared->text);
            now_posi += strlen(out_str)-1;
            printf("Y 从共享区得到的信息是：%s\n",out_str);
            int i =0;
            for (i = 0; i < strlen(out_str); i++) {
                    if (out_str[i] >= 'a' && out_str[i] <= 'z') {
                        out_str[i] -= 'a' - 'A';
                    }
                }
            printf("Y 处理后的信息是：%s",out_str);   
            strcat(output,out_str);
			shared->written = 0; // 说明已经写入
		}
		while(!semaphore_v(sem_id)){

        }
	
		//进入临界区2
		while(!semaphore_p(sem_id)){
            
        }
		if (shared2->written == 1 && now_posi <= shared->size){
			printf("\n\n=====Y 进入临界区2=====\n");
			strcpy(out_str,shared2->text);
            now_posi += strlen(out_str)-1;
            printf("Y 从共享区得到的信息是：%s\n",out_str);
            int i =0;
            for (i = 0; i < strlen(out_str); i++) {
                    if (out_str[i] >= 'a' && out_str[i] <= 'z') {
                        out_str[i] -= 'a' - 'A';
                    }
                }
            printf("Y 处理后的信息是：%s\n",out_str);   
            strcat(output,out_str);
			shared2->written = 0; // 说明已经写入
		}
		while(!semaphore_v(sem_id)){

        }
		sleep(rand() % 2);
		if (now_posi+1 >= shared->size){
			printf("\n\n\n\n=============文件读取和写入共享区结束===========\n");
			loop = 0;
		}
        
    }
    printf("\n\n最终处理好的数据是 \n%s\n",output);
    return 0;
}

static int set_semvalue(k)
{
	//用于初始化信号量，在使用信号量前必须这样做
	union semun sem_union;
 
	sem_union.val = 1;
	if(semctl(k, 0, SETVAL, sem_union) == -1)
		return 0;
	return 1;
}
 
static void del_semvalue(k)
{
	//删除信号量
	union semun sem_union;
 
	if(semctl(k, 0, IPC_RMID, sem_union) == -1)
		fprintf(stderr, "Failed to delete semaphore\n");
}
 
static int semaphore_p(k)
{
	//对信号量做减1操作，即等待P（sv）
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;//P()
	sem_b.sem_flg = SEM_UNDO;
	if(semop(k, &sem_b, 1) == -1)
	{
		fprintf(stderr, "semaphore_p failed\n");
		return 0;
	}
	return 1;
}
 
static int semaphore_v(k)
{
	//这是一个释放操作，它使信号量变为可用，即发送信号V（sv）
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;//V()
	sem_b.sem_flg = SEM_UNDO;
	if(semop(k, &sem_b, 1) == -1)
	{
		fprintf(stderr, "semaphore_v failed\n");
		return 0;
	}
	return 1;
}

