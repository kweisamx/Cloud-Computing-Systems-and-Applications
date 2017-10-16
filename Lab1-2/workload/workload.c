#define _GNU_SOURCE
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/poll.h>
#include <sys/time.h>
#define TRIMz(x)  ((tz = (unsigned long long)(x)) < 0 ? 0 : tz)

cpu_set_t cpuset,cpuget;//設置CPU的affinity的集合 
static int lockcpu=0,lockio=0;//用來判別是否要block 
static pthread_cond_t cond,cond2;//CPU與IO的blocking參數 
static pthread_mutex_t mutex,mutexs[4],mutex1,mutexio;//四條thread的控制鎖與IO的控制鎖 
static double cpuload;//cpu使用率的全域變數 

static int tarcpu,tario ;//主要為輸入的cpu使用率速率與IO寫入的速率 


/*thread1會由四條thread來執行 並讓他們跑在四顆cpu上 並設定相關控制*/
void *thread_func(void *arg) 
{
    int i = *(int*)arg;
    CPU_ZERO(&cpuset);
    CPU_SET(i, &cpuset); /* cpu i is in cpuset now */
 /* bind process to processor i */
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) !=0) 
	{perror("pthread_setaffinity_np");}  
    pthread_mutex_lock(&mutexs[i]);//設置相對應的鎖 
       while (1)//跑無窮迴圈 一旦cpu使用率太高就會被鎖給停下來 
	   {
            if(lockcpu==1)
			{
			   pthread_cond_wait(&cond,&mutexs[i]);//停在這邊等待 
			}  
        }
    pthread_mutex_unlock(&mutexs[i]);//打開 
}

/*此為io寫入的thread function 與CPU有點像 透過開關來達到控制速率的方式*/
void *thread_func2(void *arg)
{
	char buffer[1024]={0};//寫一個準備輸出的buffer 
    long long int i=0;
    FILE *pFile;
    pFile = fopen( "write.txt","w");//寫入write.txt這個檔 
    if( NULL == pFile )
	{printf( "open failure" );}
    else{
	pthread_mutex_lock(&mutex1);
	   while(i<100000000)//預防把系統的空間給寫滿,這邊主要要求的是速率 
	    {
	      i++;
 	      fwrite(buffer,1,sizeof(buffer),pFile);
	      if(lockio==1)
	      pthread_cond_wait(&cond2,&mutex1);
	    }  
	pthread_mutex_unlock(&mutex1);
    }
    fclose(pFile);
    system("rm -rf write.txt");//若有跑完則清掉 
}



/*接下來這段主要是用來刷新用的 
 *透過每次刷新來取得目前的CPU使用率與IO寫入速度 
 *並進一步控制thread的行為
 */ 


void stat() {
system("clear");//刷新螢幕

//計算CPU的使用率 
FILE *fp_stat ;
char buff[128+1] ;
static unsigned long long  user,user_sav,user_frme,system,system_sav,system_frme,nice,nice_sav,nice_frme,idle,idle_sav,idle_frme,iowait,iowait_sav,iowait_frme,irq,irq_sav,irq_frme,softirq,softirq_sav,softirq_frme,steal,steal_sav,steal_frme,tot_frme,tz;
float scale ;
fp_stat = fopen("/proc/stat","r") ;//read the text
while( fgets(buff, 128, fp_stat) ) {
if( strstr(buff, "cpu") ) {
sscanf(buff, "cpu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",&user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) ;
break ;
}
}
fclose(fp_stat) ;
user_frme = user - user_sav ;
system_frme = system - system_sav ;
nice_frme = nice - nice_sav ;
idle_frme = TRIMz(idle - idle_sav) ;
iowait_frme = iowait - iowait_sav ;
irq_frme = irq - irq_sav ;
softirq_frme = softirq - softirq_sav ;
steal_frme = steal - steal_sav ;
tot_frme = user_frme + system_frme + nice_frme + idle_frme + iowait_frme + irq_frme + softirq_frme + steal_frme ;
scale = 100.0 / (float)tot_frme ;
cpuload=(float)(user_frme+system_frme+nice_frme)/(float)tot_frme;
user_sav = user;
system_sav = system;
nice_sav = nice;
idle_sav = idle;
iowait_sav = iowait;
irq_sav = irq;
softirq_sav = softirq;
steal_sav = steal;
/**
上半部是cpu的計算部分 下半部是IO的 
*/
FILE *fp_stat2;
double testf;
char buffio[128];
static unsigned  long read ,read_sav,read_frme,
write,write_sav,write_frme;
fp_stat=fopen("/proc/diskstats","r");
while(fgets(buffio,128,fp_stat))
{
if(strstr(buffio,"sda"))
{
sscanf(buffio,"%*d %*d sda %*d %*d %lu %*d %*d %*d %lu",&read,&write);
//printf("read sector=%lu,write=%lu\n",read,write);
break;
}
}
fclose(fp_stat);
read_frme=read-read_sav;
write_frme=write-write_sav;
//printf("Disk read = %.3fMB/s\nDisk write = %.3fMB/s \n",(float)read_frme*512/1000000,(float)write_frme*512/1000000);
read_sav=read;
write_sav=write;
testf=(float)write_frme*512/1000000;
if(tarcpu<cpuload*100)
{
  pthread_mutex_lock(&mutex);
  lockcpu = 1;
  pthread_mutex_unlock(&mutex);
}
else
{
  
  pthread_mutex_lock(&mutex);
  lockcpu = 0;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
}
if(tario<testf||tario==0)
{
	pthread_mutex_lock(&mutexio);
	lockio=1;
	pthread_mutex_unlock(&mutexio);
}

else
{
	pthread_mutex_lock(&mutexio);
	lockio=0;
	pthread_cond_signal(&cond2);
	pthread_mutex_unlock(&mutexio);
}



}
int main(int argc, char * argv[]) {
struct itimerval tick ;
int res ;
tarcpu=atoi(argv[1]);
long long int a=atoi(argv[2]);
tario=atoi(argv[3]);



/*
用來增加記憶體 以MB為單位 配置所需記憶體 
*/
int *arr=calloc(a*1000000,sizeof(char));





/*設signal*/
signal(SIGALRM, stat) ;        /* SIGALRM handeler */
/* setting first time interval */
tick.it_value.tv_sec = 3 ;         // sec
tick.it_value.tv_usec = 0 ;        // usec
/* setting next time interval */
tick.it_interval.tv_sec =0 ;          
tick.it_interval.tv_usec =10000;    
res = setitimer(ITIMER_REAL, &tick, NULL);//settimer
if(res)
fprintf(stderr, "Error: timer setting faul.\n") ;
else
printf("Timer start...\n") ;
/*set END*/
	int cpunu1=0;
	int cpunu2=1;
	int cpunu3=2;
	int cpunu4=3;
	pthread_t thread1,thread2,thread3,thread4,threadio;
	if(pthread_create(&thread1,NULL,thread_func,&cpunu1)!=0)
		{perror("pthread_create");}
	if(pthread_create(&thread2,NULL,thread_func,&cpunu2)!=0)
		{perror("pthread_create");}
	if(pthread_create(&thread3,NULL,thread_func,&cpunu3)!=0)
		{perror("pthread_create");}
	if(pthread_create(&thread4,NULL,thread_func,&cpunu4)!=0)
		{perror("pthread_create");}
	if(pthread_create(&threadio,NULL,thread_func2,NULL)!=0)
		{perror("pthread_create");}
getchar();

return 0 ;
}
