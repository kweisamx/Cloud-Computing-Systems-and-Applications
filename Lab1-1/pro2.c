/* 
利用參數回傳的方式,讓我們能在main裡面去對我們想要命令thread做的事情變得比較輕鬆,也比較有彈性 
*/
#define _GNU_SOURCE
 
#include <stdio.h>
#include <math.h>
#include <pthread.h>

cpu_set_t cpuset,cpuget;
static double final=0;
struct test//傳給thread func的資訊 
{
int cpunu;//要跑第幾顆CPU 
long int cut;//從哪邊開始跑 
long end;//結束n值 
int pid;//thread的編號 
};
 
double waste_time(long n,long int cut)

{

    double res = 0;
    long i = cut;//讓thread從參數的值開始run 
    while (i <n * 200000000) {
        i++;

        res += sqrt(i);
    }

    return res;
}
 
void *thread_func(void *param)
{   
	
    struct test test1=*(struct test*)param;
    CPU_ZERO(&cpuset);
    CPU_SET(test1.cpunu, &cpuset); /* cpu i is in cpuset now */
  
    /* bind process to processor 0 */
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) !=0) {
        perror("pthread_setaffinity_np");
    }   

    /* waste some time so the work is visible with "top" */
	  printf("thread %d run on cpu %d\n",test1.pid,test1.cpunu);
        final=waste_time(test1.end,test1.cut)+final;
    pthread_exit(NULL);

}
 
int main(int argc, char *argv[])
{ 
	
    struct test test1,test2;
    pthread_t thread1,thread2;
 	//從這邊開始輸入資訊 
    test1.cpunu=0;//想要在第幾顆CPU 
    test1.cut=0;
    test1.end=3;
    test1.pid=1;
    
    test2.cpunu=1;
    test2.cut=600000000;
    test2.end=5;
    test2.pid=2;
    time_t startwtime, endwtime;
    startwtime = time (NULL); 
    if (pthread_create(&thread1, NULL, thread_func,
       (void *)&test1) != 0) {
        perror("pthread_create");
    }

    if (pthread_create(&thread2, NULL, thread_func,
        (void *)&test2) != 0) {
        perror("pthread_create");
    }
    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);
    printf("result=%f\n",final);
    endwtime = time (NULL);
    printf ("wall clock time = %d\n", (endwtime - startwtime));
    return 0;
}


