/* 
基本上做的事情與c實驗相同,不同點在於讓兩條thread跑在同一顆cpu上 
*/
#define _GNU_SOURCE
 
#include <stdio.h>
#include <math.h>
#include <pthread.h>

cpu_set_t cpuset,cpuget;

struct test
{
int cpunu;
long int cut;
long end;
int pid;
};
static double final=0;
double waste_time(long n,long int cut)

{
   // printf("n,cut=%f %ld\n",n,cut);
    double res = 0;

   // printf("n= %f\n",n);
    long i = cut;
    while (i <n * 200000000) {
        i++;
//	printf("%ld\n",i);
        res += sqrt(i);
    }
//	printf("finish res=%lf\n",res);
    return res;
}
 
void *thread_func(void *param)
{   
	
    struct test test1=*(struct test*)param;
    CPU_ZERO(&cpuset);
    CPU_SET(test1.cpunu, &cpuset); /* cp i is in cpuset now */
  
    /* bind process to processor 0 */
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) !=0) {
        perror("pthread_setaffinity_np");
    }  
    // printf("i=%d\n",test1.cpunu);

 //   printf("%f",test1.end);

    /* waste some time so the work is visible with "top" */
 	printf("thread %d run on cpu %d\n",test1.pid,test1.cpunu);
    final=waste_time(test1.end,test1.cut)+final;

    pthread_exit(NULL);

}
 
int main(int argc, char *argv[])
{ 
	
    struct test test1,test2;
    pthread_t thread1,thread2;

    test1.cpunu=0;//跑在CPU0 上 
    test1.cut=0;
    test1.end=3;
    test1.pid=1;
    
    test2.cpunu=0;//跑在CPU0上 
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
