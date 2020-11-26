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
	// 读取文件
	char l_str[500]; 
	FILE *fp;
    if((fp = fopen("input.txt","r")) == NULL){
        printf("\n\n文件读取失败，会直接使用提前预设的一个长字符串\n\n");
    } else {
        fgets(l_str,500,fp);
    }
	printf("文件读取成功，数据长度为%d",strlen(l_str));
	// 共享内存空间1
    int shm_id;
    struct shm_data *shared;
    shm_id = shmget (666, sizeof(yz), IPC_CREAT);
    if(shm_id == -1){
        perror("shmget()");
    }
    shared = (struct shm_data *)shmat(shm_id, 0, 0);
    printf("共享内存空间创建完毕, ID 为%d",shm_id);
    shared->size = strlen(l_str);
    shared->written = 0;
	
	// 共享内存空间1
	int shm_id2;
    struct shm_data *shared2;
    shm_id2 = shmget (1234, sizeof(yz), IPC_CREAT);
    if(shm_id2 == -1){
        perror("shmget()");
    }
    shared2 = (struct shm_data *)shmat(shm_id2, 0, 0);
    printf("共享内存空间创建完毕, ID 为%d",shm_id2);
    shared2->size = strlen(l_str);
    shared2->written = 0;
	printf("文件读取成功，数据长度为%d",shared2->size);
    



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

	int now_posi = 0;
	int file_len = strlen(l_str);
	// PV
	int loop = 1;
	while(loop)
	{	
		char str[20];
		//进入临界区1
		if(!semaphore_p(sem_id))
			exit(EXIT_FAILURE);
		if (shared->written == 0){
			strcpy(shared->text,"");
			printf("\n\n=====X进入临界区1=====\n");
			int last_posi = now_posi;
			strncpy(str,l_str + now_posi, min(20,file_len - now_posi));// 一次最多向共享内存区写入20个字符
			if (min(20,file_len - now_posi) < 20){
				printf("\n\n\nERROE%d",file_len - now_posi);
			}
			now_posi += min(20,file_len - now_posi);
			if (now_posi -last_posi != 20){
				char tstr[file_len - now_posi];
				strncpy(tstr,l_str,now_posi -last_posi);
				printf("\n\n%s\n\n",tstr);
				strcpy(shared->text, tstr);
			}
			strcpy(shared->text, str);
			printf("X 在临界区1 写入%s \n", shared->text);
			shared->written = 1; // 说明已经写入
		}
		if(!semaphore_v(sem_id))
			exit(EXIT_FAILURE);

	
		//进入临界区2
		if(!semaphore_p(sem_id2))
			exit(EXIT_FAILURE);
		if (shared2->written == 0){
			strcpy(shared2->text,"");
			printf("\n\n=====X进入临界区2======\n");
			int last_posi = now_posi;
			strncpy(str,l_str + now_posi, min(20,file_len - now_posi));// 一次最多向共享内存区写入20个字符
			now_posi += min(20,file_len - now_posi);
			strcpy(shared2->text, str);
			printf("X 在临界区2 写入%s \n", shared2->text);
			shared2->written = 1; // 说明已经写入
		}
		if(!semaphore_v(sem_id2))
			exit(EXIT_FAILURE);
		sleep(rand() % 2);
		if (now_posi >= file_len){
			printf("!!!!!文件读取和写入共享区结束\n");
			loop = 0;
		}
	}
	sleep(2);

	// 关闭共享内存区1、2
	shmctl(shm_id,IPC_RMID,NULL);
    shmctl(shm_id2,IPC_RMID,NULL);
	return 0;
}
   

/*
int main(){
    int shm_id;
    int *share;
    shm_id = shmget (1234, getpagesize(), IPC_CREAT);
    if(shm_id == -1){
        perror("shmget()");
    }
    share = (int *)shmat(shm_id, 0, 0);
    while(1){
        sleep(1);
        printf("%d\n", *share);
    }
    return 0;
*/




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