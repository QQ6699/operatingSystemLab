#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>// enable for semget(), semctl() and semop()
#include <sys/wait.h>
#include <signal.h>
  union semun
  {
      int val;//The value of a semaphore in the semaphore
      struct semid_ds *buf;//Semaphore attribute pointer
      unsigned short *array;//Place or set the values ​​of all semaphores in the semaphore
      struct seminfo *__buf;
  }; 

void interrupt_signal_handler(int signo);
pid_t parentId;
pid_t pid;
int s;
void child_handler(int signo);
#define THINKINGTIME 3
#define EATINGTIME 4
#define TotalMeals 25//the number of total meals 

int nuMeals=0; 
int num = 0;

//forks are a critical resource
//Equivalent to P operation
//The first parameter is the fork number
//The second parameter is the semaphores set number
  void grab_forks(int no,int semid)
  {
      //The fork number on the left of the philosopher is the same as the philosopher number
      int left = no;
      //fork on the right
      int right = (no + 1) % 5;
   
      //two forks
      //Two semaphores are operated, that is, both resources are satisfied before the operation is performed
      struct sembuf buf[2] = {
          {left,-1,SEM_UNDO},//left:member in set;second parameter: positive number is V operation;negative is p operation 
          {right,-1,SEM_UNDO}//SEM_UNDO : at the end of the process, the corresponding operation will be cancelled. If there is no such flag, when the process exits without releasing the shared resource, the kernel will release it on its behalf
      };
      //There are 5 semaphores in the signal set, but the resource sembuf is operated on
      semop(semid,buf,2);
	  
  }
   
  //Equivalent to V operation, release forks plus 1
  void put_away_forks(int no,int semid)
  {
      int left = no;
      int right = (no + 1) % 5;
      struct sembuf buf[2] = {
          {left,1,SEM_UNDO},//positive:1 increase 1 v operation;
          {right,1,SEM_UNDO}
      };
      semop(semid,buf,2);//2 is the length of the structure array sembuf
  }
   
   
  //What philosophers do
  void philosophere(int no,int semid)
  {
      for(;;) 
      {
          //When both forks are available, philosophers can eat
         printf("philosopher[%d] is thinking\n",no);  //thinking
         sleep(THINKINGTIME);
         printf("philosopher[%d] is hungry\n",no);    //hungry
         grab_forks(no,semid); //to get two forks to eat
         printf("philosopher[%d] is eating\n",no);    //eating
         sleep(EATINGTIME);
		 ++nuMeals;
		 printf("philosopher[%d] finished eating\n",no);    //eating
         put_away_forks(no,semid); //put down forks
		 if (nuMeals==TotalMeals){
			 raise(SIGTERM);//send signal SIGTERM to current process.
		 }
		 
     }
 }
  
 int main(int argc,char *argv[])
 {
	 int semid;
	//If signal set to SIG_IGN - ignore this signal
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
    signal(SIGINT, interrupt_signal_handler);
	
     
     //Create a semaphore set, which has 5 semaphores,because we have 5 forks
     semid = semget(IPC_PRIVATE,5,IPC_CREAT | 0666); //IPC_PRIVATE is user-specified semaphore set key value. 0666:access_rights 666 = -rw- rw-- rw--
     if(semid < 0) {// catching errors
	     kill(-parentId,SIGKILL);
	     perror("\nERROR: Allocation of semaphor set unsuccessfull.\n");
         exit(1);
     }
     union semun su;
     su.val = 1;
     int i;
	 //Initialize the signal set
     for(i = 0;i < 5; ++i) {
         //semid is the semaphore set id, i is the semaphore member number
         semctl(semid,i,SETVAL,su);//SETVAL means to set the value of semnum--i
     }
     //Create 5 child processes
     //int num = 0;
	 parentId = getpid();
     for(i = 0;i < 5;i++) 
     {
        pid = fork(); 
        if(pid < 0) 
         {
            exit(1);
         }
         if(0 == pid)  //child processes
         {
			 signal(SIGTERM,child_handler);
             num = i;
			 //What philosophers do
             philosophere(num,semid);
             break;
         }  	 
     }
	 
	while((pid=wait(&s))>0){
		printf("manager[%d]:the philosopher[%d] finished eating,exit the table.\n",getpid(),pid);
	}
	// Deallocation of semaphores:
	while (semctl (semid, 0, IPC_RMID, 1)<0) {//0 means we operate on all signals,IPC_RMID--delete the semaphores from the memory
		printf("Error in deallocating semaphores.\n");
	}
	printf("Succeed in deallocating semaphores.\n");
    return 0;
 }
 
 
void interrupt_signal_handler(int signo){
   kill(-parentId, SIGTERM);
}

void child_handler(int signo)
{
	printf("\n philosopher[%d]: has eaten %d times meals",num,nuMeals);
    printf("\n philosopher[%d]: received SIGTERM signal:%d, terminating\n",num,signo);
	exit(1);
}