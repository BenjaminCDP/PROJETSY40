#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>

#define IFLAGS (SEMPERM | IPC_CREAT)
#define SKEY   (key_t) IPC_PRIVATE	
#define SEMPERM 0600				  /* Permission */

int initsem(key_t semkey) 
{
    
	int status = 0;		
	int semid_init;
   	union semun
   	{
		int val;
		struct semid_ds *stat;
		ushort * array;
	} ctl_arg;
    
    if ((semid_init = semget(semkey, 1, IFLAGS)) > 0)
    {
		ushort array[1] = {0};
		ctl_arg.array = array;
		status = semctl(semid_init, 0, SETALL, ctl_arg);
    }
    if (semid_init == -1 || status == -1)
    { 
		perror("Erreur initsem");
		return (-1);
	} 
    else 
    	return (semid_init);
}


/*********************************************************************/
/*  Pour Operation P et V 					     */

int sem_id ;
struct sembuf sem_oper_P ;  /* Operation P */
struct sembuf sem_oper_V ;  /* Operation V */

/*********************************************************************/

void P(int semnum)
{
	sem_oper_P.sem_num = semnum;
	sem_oper_P.sem_op  = -1 ;
	sem_oper_P.sem_flg = 0 ;
}

void V(int semnum)
{
	sem_oper_V.sem_num = semnum;
	sem_oper_V.sem_op  = 1 ;
	sem_oper_V.sem_flg  = 0 ;
}


int msgid;
key_t key;



typedef struct
{
	long type;
	char * destination;
	int placeOccupe;
	int placeLibre;
	bool disponibleReception;
}tstat;

typedef struct
{
	long type;
	char * destination;
	int conteneur; //1 seul par message à priori
}tstatconteneur;



// void camion(int occupe,int libre,char * dest)
// {
// 	pid_t pid;
// 	pid=fork();
// 	tstat stat;
// 	stat.type=1;
// 	stat.placeOccupe=occupe;
// 	stat.placeLibre=libre;
// 	stat.disponibleReception=false;
// 	stat.destination=dest;
// 	if(msgsnd(msgid, &stat, sizeof(tstatgrue),0) == -1)
// 	{
// 	    perror("Erreur d'envoi \n");
// 	    exit(1);
// 	}
// }

void grue(int n) // n représente le numéro de la grue
{
	pid_t pid;
	pid=fork();
	tstatconteneur statconteneur;
	if(msgrcv(msgid, &statconteneur, sizeof(tstatconteneur),1,0) == -1)
	{
	    perror("Erreur de réception \n");
	    exit(1);
	}
	printf("un conteneur recupéré\n");
	sleep(5);
	V(0);

}

void train(int occupe,int libre,char * dest)
{
	pid_t pid;
	pid=fork();
	tstat stat;
	tstatconteneur statconteneur;
	stat.placeOccupe=occupe;
	stat.placeLibre=libre;
	stat.disponibleReception=false;
	stat.destination=dest;
	statconteneur.type=1;
	statconteneur.destination=dest;
	statconteneur.conteneur=1;
	for(int i=0;i<occupe;i++)
	{

		if(msgsnd(msgid, &statconteneur, sizeof(tstatconteneur),0) == -1)
		{
		    perror("Erreur d'envoi\n");
		    exit(1);
		}
		printf("Envoi conteneur\n");
		P(0);
	}
	exit(1);


}


int main(int argc, char ** argv)
{
	int semid;
	if((key = ftok(argv[0], 'A')) == -1)
	{
		perror("Erreur de creation de la clé \n");
		exit(1);
	}

	if ((semid = initsem(SKEY)) < 0)	/* Création d'un ensemble de sémaphore */
		return(1);

	if((msgid = msgget(key, 0750 | IPC_CREAT | IPC_EXCL)) == -1)
	{
		perror("Erreur de creation de la file\n");
		exit(1);
	}
	train(4,2,"Belfort");
	grue(1);
	
	msgctl(msgid, IPC_RMID, NULL);
	return EXIT_SUCCESS;
}