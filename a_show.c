#include "fun.h"

pid_t pid_a1;

int main()
{
	int shmid;
	shmid=shmget((key_t)1234,4096,IPC_CREAT|0660);
	if(-1==shmid)
	{
		perror("shmget");
		return -1;
	}
	char *p;
	p=(char*)shmat(shmid,NULL,0);
	if((char*)-1==p)
	{
		perror("shmat");
		return -1;
	}
	int semid;
	semid = semget((key_t)2000, 1, IPC_CREAT | 0660);
	int ret;
	ret = semctl(semid, 0, SETVAL, 1);
	if (-1 == ret)
	{
		perror("semctl");
		return -1;
	}
	//pid_a1=getpid();//将自己pid传给a
	//char s[10]={0};
	//sprintf(s,"%d",pid_a1);
	//memcpy(p,s,sizeof(s));
	struct sembuf sopp,sopv;
	sopp.sem_num=0;
	sopp.sem_op=-1;
	sopp.sem_flg=SEM_UNDO;
	sopv.sem_num=0;
	sopv.sem_op=1;
	sopv.sem_flg=SEM_UNDO;
	while (1) {
		/*if(strcmp(p,s)!=0){*/
			if (*p) {
				semop(semid, &sopp, 1);
				printf("%s\n", p);
				memset(p, 0, 4096);
				semop(semid, &sopv, 1);
			}
		/*}*/
	}
	int ret1=shmdt(p);
	if(-1==ret1)
	{
		perror("shmdt");
		return -1;
	}
	return 0;
}
