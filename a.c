#include "fun.h"

int fdw,fdr,shmid,semid;
pid_t pid_a,pid_b,pid_a1;

void sig_handle(int sig)
{
	kill(pid_b,SIGINT);
	/*printf("%d\n",pid_b);*/
	/*kill(pid_a1,SIGINT);
	printf("%d\n",pid_a1);*/
	shmctl(shmid,IPC_RMID,NULL);
	semctl(semid,0,IPC_RMID);
	close(fdr);
	close(fdw);
	exit(0);
}

int main(int argc,char *argv[])
{	
	char *p;
	if(argc!=3)
	{
		perror("error args\n");
		return -1;
	}
	if (mkfifo(argv[1], 0666) == -1)
	{
		perror("mkfifo1 fail");
		return -1;
	}
	if (mkfifo(argv[2], 0666) == -1)
	{
		perror("mkfifo2 fail");
		return -1;
	}
	fdr = open(argv[1], O_RDONLY);
	if(-1==fdr)
	{
		perror("open");
		return -1;
	}
	fdw = open(argv[2], O_WRONLY);
	if(-1==fdw)
	{
		perror("open");
		return -1;
	}

	signal(SIGINT,sig_handle);//设置关闭信号，获取传给对方
	pid_a=getpid();
	write(fdw,&pid_a,sizeof(pid_t));
	read(fdr,&pid_b,sizeof(pid_t));

	//printf("open fdr=%d,fdw=%d\n",fdr,fdw);
	char buf[100]={0};
	fd_set rdset;
	shmid = shmget((key_t)1234, 4096, IPC_CREAT | 0660);
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
	semid = semget((key_t)2000, 1, IPC_CREAT | 0660);
	int ret;
	ret = semctl(semid, 0, SETVAL, 1);
	if (-1 == ret)
	{
		perror("semctl");
		return -1;
	}
	//pid_a1=atoi(*p);//获得a1的pid
	//memset(p,0,4096);
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
			semop(semid,&sopp,1);
			if(FD_ISSET(fdr,&rdset))
			{
				memset(buf,0,sizeof(buf));
				read(fdr,buf,sizeof(buf));
				strcpy(p, "b: ");
				strncat(p, buf, strlen(buf));
			}
			if(FD_ISSET(0,&rdset))
			{
				memset(buf,0,sizeof(buf));
				read(0,buf,sizeof(buf));
				write(fdw,buf,strlen(buf)-1);
				strcpy(p, "a: ");
				strncat(p, buf, strlen(buf) - 1);
			}
			semop(semid, &sopv, 1);
		}
	}
//	shmdt(p);
//	shmctl(shmid,IPC_RMID,NULL);
//	semctl(semid,0,IPC_RMID);
//	close(fdr);
//	close(fdw);
//	unlink(argv[1]);
//	unlink(argv[2]);
	return 0;
}
