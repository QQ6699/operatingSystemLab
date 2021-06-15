/*
When using a mutex to achieve thread synchronization, the system will
 add a mark called a mutex to the shared resource to prevent multiple
 threads from accessing the same shared resource at the same time. 
 Mutex is usually called a mutex, which is equivalent to a lock. 
 The use of a mutex can guarantee the following 3 points:
(1) Atomicity: If a mutex is set in a thread, the operation between 
locking and unlocking will be locked as an atomic operation, and these 
operations will either be completed or not executed;
(2) Uniqueness: If a mutex is locked for a thread, no other thread can 
lock the mutex before the lock is released;
(3) Non-busy waiting: If a thread has locked a mutex, and then the second 
thread tries to lock the mutex, the second thread will be suspended; until 
the first thread releases the mutex When locked, the second thread will be
awakened and lock the mutex at the same time.

Why m mutex is initialized with 1 and mutexes from the array s are
initialized with 0's?
1.m mutex is initialized with 1; we use m mutex to prevent multiple
 philosophers from modifying the state[i] and graping the forks at the same 
 time; when the mutex m is equal to 1,it means no philopher is modifying state[i]
 and graping the forks.
Once a philosopher wants to grap chopsticks, he needs to lock the mutex m.If  
 the lock mutex m is available,that is,mutex m is 1.else
the philophers' threads will are suspended until the mutex m is unlocked ,m=1.

2.mutexes from the array s are initialized with 0's
I think it is because we will judge if the philophers’ neighbours are eating
in test function.If not ,that means the two forks are available,then we 
will set up(s[i]).up(s[i]) is to set the mutex of s[i] to 1, unlock, and 
then when the code is executed to the down(s[i]) which is last line of code
of the grap function, the thread will not be suspended. Otherwise, 
if the condition that the neighbors are not thinking is not satisfied, 
we do not execute up(s[i]), and then when the thread wants to locked the mutex s[i]
down(s[i]),it will be suspended, because if a thread has locked a mutex, and then the second 
thread tries to lock the mutex, the second thread will be suspended; until 
the first thread releases the mutex When locked, the second thread will be
awakened and lock the mutex at the same time.So in put_away_forks function,
we will releases the mutex.

what difference is it between task5 and task7?
in task5:we use semaphores. we use semop(semid,buf,2) to 
operate the left and right forks of the philosopher at the same time 
to achieve atomicity.We regard each fork as a semaphore, the initial value is 1.
in task7:
we use mutex m and s[i] to achieve atomicity.We judge if the 
state of the two neighbors are the thinking state,and if the states are both
 thinking,then unlock mutex s[i],that means the current philopher won't be 
 suspended when it excute the code:down(s[i]) because the mutex s[i] is
 available(we has excute code up(s[i]) in the test function).But the second 
 thread philopher will be suspended when it excutes the code:down(s[i])
 because the mutex s[i] has already been locked by the last thread.

*/
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

#define N		5
#define LEFT		( i + N - 1 ) % N
#define RIGHT		( i + 1 ) % N
#define THINKING	1
#define HUNGRY		2
#define EATING		3
#define FOOD_LIMIT	3
#define EATING_TIME	1
#define THINKING_TIME	2
#define	up		pthread_mutex_unlock
#define	down	pthread_mutex_lock

pthread_mutex_t m; 		//main mutex initialized to 1 for critical regoin
int state[N]={THINKING};			//initiated to THINKING
pthread_mutex_t s[N];		//initialized to 0,initializing philosopher/fork
pthread_t philosophers[N];
int meal[N];//counter for each ppl eat times

void grab_forks( int i );
void put_away_forks( int i );
void test(int i);
void *thread_function(void *i);//philosopher action

int main(){
	int i=0;
	int thread_id[N] = {0,1,2,3,4};
	//pthread_mutex_init(pthread_mutex_t * mutex,const pthread_mutexattr_t *attr);
	pthread_mutex_init(&m, NULL);//initialize main mutex variable is m// attr is attribute,NULL means default attribute
	for(i=0;i<N;i++){
		pthread_mutex_init(&s[i], NULL);//Initialize mutex s[N]////initialize fork 
		down(&s[i]);//Initialize s[i] to 0
		/*  Parameters of pthread_create: 
		//first is the pointer points to the threads identifier
		//second set the attributes of the threads
		//third parameter for the start address of the threads
		// last one is the parameter of the executing function */
		if(pthread_create(&philosophers[i], NULL, thread_function,&thread_id[i])!=0){//create the threads return 0 if success create
			fprintf(stderr,"\nERROR: pthread_create() failed. Cannot create philosopher!\n");
			return 1;
		}
	}
	for(i=0;i<N;++i){
		//
		pthread_join(philosophers[i],NULL);
	}
	pthread_mutex_destroy(&m);//release after finish
	for(i=0;i<N;++i){
		int num = (int) i;
		printf("philopher[%d] ate %d times..in total.\n",num,meal[i]);
		pthread_mutex_destroy(&s[i]);//destroy the mutex(lock) //release after finish
	}
	pthread_exit(NULL);
	return 0;
}

void grab_forks( int i ) {
	down(&m);// lock main mutex, "one philopher can grab forks"
		state[i]=HUNGRY;//change state to hungry 
		meal[i]++;//counting for each philopher eating times
		test(i);
	up(&m);//unlocking the mutex m 
	down(&s[i]);// other philopher will be suspended if they mutex s[i] is not unlocked
}

void put_away_forks( int i ) {
	down(&m);// locking the mutex m,only one philopher can excute the following codes
		state[i]=THINKING;
		test(LEFT);//check if neighbour 'left ' can eat
		test(RIGHT);//check  if neighbour 'right ' can eat
	up(&m);//unlock the main mutex 
}

void test(int i) {
	if( state[i] == HUNGRY && state[LEFT] != EATING	&& state[RIGHT] != EATING )
	{
		state[i] = EATING;//change state to eating
		up(&s[i]);//unlock fork for the philopher[i]
	}
}

void *thread_function(void *i){
	int meals_left=FOOD_LIMIT;
	int num = *(int*) i;//Number of the philopher
	printf("philosopher[%d] came to the table.\n",num);
	while(meals_left){//check if meal has finish or not
		printf("philosopher[%d] is thinking........\n", num);
		sleep(THINKING_TIME);//sleep time for thinking
		grab_forks(num);
		printf("philosopher[%d] is eating [ %dth] meal.\n",num,(FOOD_LIMIT-(--meals_left)));
		sleep(EATING_TIME);//sleep time for eating
		put_away_forks(num);
	}
}
