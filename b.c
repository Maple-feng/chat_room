#include "fun.h"

int fdw,fdr,shmid,semid;
pid_t pid_a,pid_b,pid_b1;

void sig_handle(int sig)
{
	kill(pid_a,SIGINT);
	/*kill(pid_b1,SIGINT);*/
	shmctl(shmid,IPC_RMID,NULL);
	semctl(semid, 0, IPC_RMID);
	close(fdr);
	close(fdw);
	exit(0);
}

int main(int argc,char *argv[])
{	
	char *p;
	fdw = open(argv[1], O_WRONLY);
	if(-1==fdw)
	{
		perror("open");
		return -1;
	}
	fdr = open(argv[2], O_RDONLY);
	if(-1==fdr)
	{
		perror("open");
		return -1;
	}
	
	signal(SIGINT,sig_handle);//设置关闭信号，获取对方信号
	pid_b=getpid();
	read(fdr,&pid_a,sizeof(pid_t));
	write(fdw,&pid_b,sizeof(pid_t));

	char buf[100]={0};
	fd_set rdset;
	shmid = shmget((key_t)2345, 4096, IPC_CREAT | 0660);
	if (-1 == shmid)
	{
		perror("shget");
		return -1;
	}
	p = (char*)shmat(shmid, NULL, 0);
	if ((char*)-1 == p)
	{
		perror("shmat");
		return -1;
	}
	semid = semget((key_t)1000, 1, IPC_CREAT | 0660);
	int ret;
	ret = semctl(semid, 0, SETVAL, 1);
	if (-1 == ret)
	{
		perror("semctl");
		return -1;
	}
	//pid_b1=*p;//获得b1的pid
	//memset(p,0,4096);//清空
	struct sembuf sopp, sopv;
	sopp.sem_num = 0;
	sopp.sem_op = -1;
	sopp.sem_flg = SEM_UNDO;
	sopv.sem_num = 0;
	sopv.sem_op = 1;
	sopv.sem_flg = SEM_UNDO;
	while(1)
	{
		FD_ZERO(&rdset);
		FD_SET(0,&rdset);
		FD_SET(fdr,&rdset);
		ret=select(fdr+1,&rdset,NULL,NULL,NULL);
		if(ret>0)
		{
			semop(semid, &sopp, 1);
			if(FD_ISSET(fdr,&rdset))
			{
				memset(buf,0,sizeof(buf));
				read(fdr,buf,sizeof(buf));
				printf("%s\n",buf);
				strcpy(p, "a: ");
				strncat(p, buf, strlen(buf));
			}
			if(FD_ISSET(0,&rdset))
			{
				memset(buf,0,sizeof(buf));
				read(0,buf,sizeof(buf));
				write(fdw,buf,strlen(buf)-1);
				strcpy(p, "b: ");
				strncat(p, buf, strlen(buf) - 1);
			}
			semop(semid, &sopv, 1);
		}
	}
//	shmdt(p);
//	shmctl(shmid,IPC_RMID,NULL);
//	close(fdr);
//	close(fdw);
	return 0;
}
