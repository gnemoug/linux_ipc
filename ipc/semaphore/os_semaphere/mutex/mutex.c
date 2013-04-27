/*
 *date:2012-12-3
 *author:chermong
 *description:use orderly resource allocation method to resolve died lock problem
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define Max 10 //the most number of threads
pthread_t E[Max];//the thread of E direction
pthread_t W[Max];//the thread of W direction
pthread_t S[Max];//the thread of S direction
pthread_mutex_t a;//the mutex of A
pthread_mutex_t b;//the mutex of B
pthread_mutex_t c;//the mutex of C
pthread_mutex_t d;//the mutex of D

/*
 *args:
 *    num:the number flag of the thread 
 *doc:S derection
 */
void *s(int num)
{
    pthread_mutex_lock(&c);
    printf("S %d enter C\n",num);
	sleep(2);
	pthread_mutex_lock(&b);
	printf("S %d enter B\n",num);
	pthread_mutex_unlock(&c);
	sleep(2);
	pthread_mutex_lock(&a);
	printf("S %d enter A\n",num);//note:the order of the unlocked sentence and the printf sentence
	pthread_mutex_unlock(&b);
	sleep(2);
	printf("S %d leave A\n",num);
	printf("!!!S finished one\n");
	pthread_mutex_unlock(&a);
}

/*
 *args:
 *    num:the number flag of the thread 
 *doc:E derection
 */
void *e(int num)
{
	pthread_mutex_lock(&b);
	printf("E %d enter B\n",num);
	sleep(2);
	pthread_mutex_lock(&a);
	printf("E %d enter A\n",num);
	pthread_mutex_unlock(&b);
	sleep(2);
	pthread_mutex_lock(&d);
	printf("E %d enter D\n",num);
	pthread_mutex_unlock(&a);
	sleep(2);
	printf("E %d leave D\n",num);
	printf("!!!E finished one\n");
	pthread_mutex_unlock(&d);
}

/*
 *args:
 *    num:the number flag of the thread 
 *doc:W derection
 */
void *w(int num)
{
	pthread_mutex_lock(&c);
	pthread_mutex_lock(&d);
	printf("W %d enter D\n",num);
	sleep(2);
	printf("W %d enter C\n",num);
	pthread_mutex_unlock(&d);
	sleep(2);
	printf("W %d leave C\n",num);
	printf("!!!W finished one\n");
	pthread_mutex_unlock(&c);
}

int main(int argc,char *argv[])
{
	int carnum,i;

	printf("This program will help you let cars cross this T crossing.\n");
	printf("Suppose that there are same quantity of cars in three derections.\n");
	printf("Please input the integer number,less than %d:",Max);

    //enter the number of the threads of three direction
	scanf("%d",&carnum);
	if(carnum > Max) {
	    printf("Please input the integer number,less than %d,try again\n",Max);
		exit(1);
	}

	//the mutexes becomes initialised and unlocked,success return 0
    if(pthread_mutex_init(&c,NULL) & pthread_mutex_init(&b,NULL) & pthread_mutex_init(&a,NULL) & pthread_mutex_init(&d,NULL)){
		printf("Initialises the 4 mutexes error.");
		exit(1);
	}

    //create threads to work
	for(i = 0;i < carnum;i++) {
		pthread_create(&W[i],NULL,(void*)w,(void*)(i + 1));
		pthread_create(&E[i],NULL,(void*)e,(void*)(i + 1));
		pthread_create(&S[i],NULL,(void*)s,(void*)(i + 1));
		sleep(5);
	}

    //make sure all the theads to finish before return in main thread.
	for(i = 0;i < carnum;i++) {
		pthread_join(W[i],NULL);
		pthread_join(E[i],NULL);
		pthread_join(S[i],NULL);
	}

	//the mutexes becomes destoryed,success return 0
    if(pthread_mutex_destroy(&c) & pthread_mutex_destroy(&b) & pthread_mutex_destroy(&a) & pthread_mutex_destroy(&d)){
		printf("Destory the 4 mutexes error.");
		exit(1);
	}

	exit(0);
}
