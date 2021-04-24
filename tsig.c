#include <unistd.h>//所有的系统调用都要包含这个头文件
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>


#define NUM_CHILD 8
#define WITH_SIGNALS //when we define WITH_SIGNALS ,Conditional compilation between #ifdef and #endif will be excuted

int i;
int s;
int childExitNumb = 0;
int exit_code;

#ifdef WITH_SIGNALS
int j;
int interruptFlag=0;//stty -a can be used to view the terminal keyboard configuration
void interrupt_signal_handler(int signo);
void child_handler(int signo);
#endif

int main(){
    pid_t pid,parentid,pid_bk;
    parentid = getpid();
    printf("In parent[%d]:\n",parentid);//print parent process id

    #ifdef WITH_SIGNALS
	//There is a constant NSIG in signal.h that defines the number of signals, and its value is usually 64.
	for(j = 1; j < NSIG; j++){//from signal 1:SIGUP to 64:SGRTMAX (kill -l to check all signals), NSIG: is one greater than the total number of signals.So NSIG is 65.
		//If signal set to SIG_IGN - ignore this signal
		signal(j, SIG_IGN);
	}
	//Restore the default handler for SIGCHLD signal
	signal(SIGCHLD, SIG_DFL);//This signal is generated when the state of the child process changes (the child process ends), and the parent process needs to use the wait call to wait for the child process to end and reclaim it.
    //Register keyboard interrupt signal
    signal(SIGINT, interrupt_signal_handler);
	#endif

    for(i = 1; i <= NUM_CHILD; i++ )
   {
        #ifdef WITH_SIGNALS
        if(interruptFlag){
             kill(-parentid, SIGTERM);
             printf("parent[%d]The creation process is interruped.\n",parentid);
             printf("parent[%d]sending SIGTERM signal.\n",parentid);
             break;
        }
        #endif

        pid=fork();//The child process will inherit the code of the parent process after fork(),That is, the code after creating the child process

        // This is an abnormal situation,create process unsuccessfully
        if (pid==-1)
        {
            perror("create process unsuccessfully\n!");
            kill(-parentid, SIGTERM);//here parentid=groupid.Signals all processes in the specified process group 
            exit(1);                //SIGTERM: SIGTERM signal is a generic signal used to cause program termination. Unlike SIGKILL , this signal can be blocked, handled, and ignored.
           
        }
        else if(pid == 0){//child process
            printf("%dth child[%d] is created\n",i,getpid());
            childAlgorithm();
            break;//Let the child process jump out of the loop, do not execute pid=fork();
        }
        else{//parent process
          sleep(1); //Here delay the parent process program, wait for all child processes to be created first
        }
   }

   if(i>NUM_CHILD){//here because of the existence of break, I of subroutine cannot be greater than NUM_CHILD
       printf("all child processes are created successfully\n");

   }

   //f the calling process has no child process or its child process has ended, the function returns immediately.
   // here wait function :only the parent process can wait for the child process, and the parent process executes wait();child processes won't excute this wait function
    while((pid_bk=wait(&s))>0){//If the execution is successful, the process number of the child process is returned. Error return - 1, failure reason exists in errno//等待子进程结束，返回状态值(保存于&s),若执行成功则返回子进程的进程号，回收子进程的资源
      if(WIFEXITED(s)!=0){//If the program is terminated normally, 如果正常终止的
         printf("\nparent[%d] : child[%d] excuted completion, exited with code is %d.\n",getpid(),pid_bk,WEXITSTATUS(s));//Then print the process number of child process and the return value of the subprocess,
         childExitNumb++;                                                                         //Returns the exit status of the child process. The exit status is saved in status variables ,that is s here.     
        }
    }

    if(getpid()==parentid){
      printf("\nparent[%d]: there are no more child processes.The number of child processes terminations is :%d\n",getpid(),i-1);
      printf("parent[%d]: the number of just received child processes exit codes is :%d\n",getpid(),childExitNumb);
    }

    	#ifdef WITH_SIGNALS
	//Restore the old service handlers of all signals
	for(j = 1; j < NSIG; j++){
		signal(j, SIG_DFL);
	}
	#endif

    return 0;
}

int childAlgorithm(void){
    #ifdef WITH_SIGNALS
    signal(SIGINT,SIG_IGN);//ignore the keyboard interrupt signal
    /*
    The function of the SIGTERM signal is that if a process does not capture and process the SIGTERM signal, 
    then this signal will kill the process. ... The process can capture a certain signal, and then process the signal. 
    Signal is a way of inter-process communication, and it can also be a way to simply specify a process to perform a certain action through SHELL 
    */
    signal(SIGTERM,child_handler);
    #endif
    printf("%dth child[%d]: I am child[%d] my parentid is:%d \n",i,getpid(),getpid(),getppid());
    sleep(10);
    printf("child[%d]: execution completion\n",getpid());
    exit_code=5;
    exit(exit_code); //Return 5 after successful execution
}

#ifdef WITH_SIGNALS
void interrupt_signal_handler(int signo)//回调函数的形参必须是整型的The formal parameters of the callback function must be integer
{
    printf("\n\nparent[%d]: receive the the keyboard interrrupt signal:%d \n",getpid(),signo);
    interruptFlag=1;//SIGINT
}
#endif

#ifdef WITH_SIGNALS
void child_handler(int signo)//回调函数的形参必须是整型的The formal parameters of the callback function must be integer
{
    printf("\nchild[%d]: received SIGTERM signal:%d, terminating\n",getpid(),signo);
}
#endif