/*
	透過從/proc/stat來讀取有關cpu的相關資訊 從/proc/meminfo來讀取記憶體的資訊 從/proc/diskstats
	來讀取開機到目前時間的讀取與寫入sector
	再設signal 時間間隔為一秒 每秒都會刷新 disk的變化量也可以藉由直接相減獲得(因為間隔時間是1S) 
	每次都會刷新螢幕 退出時請按ctrl+c 
	
	 

*/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/poll.h>
#include <sys/time.h>
#define TRIMz(x)  ((tz = (unsigned long long)(x)) < 0 ? 0 : tz)

void stat() {
system("clear");//刷新螢幕

//計算CPU的使用率 
FILE *fp_stat ;
char buff[128] ;
static unsigned long long int user,user_sav,user_frme,system,system_sav,system_frme,nice,nice_sav,nice_frme,idle,idle_sav,idle_frme,iowait,iowait_sav,iowait_frme,irq,irq_sav,irq_frme,softirq,softirq_sav,softirq_frme,steal,steal_sav,steal_frme,total_frme,tz;
float scale ;
fp_stat = fopen("/proc/stat","r") ;//read the text
while( fgets(buff, 128, fp_stat) ) {
if( strstr(buff, "cpu") ) {
sscanf(buff, "cpu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
             &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) ;
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
total_frme = user_frme + system_frme + nice_frme + idle_frme + iowait_frme + irq_frme + softirq_frme + steal_frme ;
scale = 100.0 / (float)total_frme ;
printf("%.1f%%user,%.1f%%system,%.1f%%nice,%.1f%%idle,%.1f%%wait,%.1f%%irq,%.1f%%softirq,%.1f%%steal\n",
user_frme * scale, system_frme * scale, nice_frme * scale,
idle_frme * scale, iowait_frme * scale, irq_frme * scale, softirq_frme * scale, steal_frme * scale) ;
double cpuload=(float)(user_frme+system_frme+nice_frme)/(float)total_frme;
printf("CPU loading= %5.2f%%\n\n",cpuload*100);
user_sav = user;
system_sav = system;
nice_sav = nice;
idle_sav = idle;
iowait_sav = iowait;
irq_sav = irq;
softirq_sav = softirq;
steal_sav = steal;
//cpuend

//計算memory的使用率 
static unsigned  long memt,memf,buf,cah,memt_sav,memf_sav,buf_sav,cah_sav,
memt_frme,memf_frme,buf_frme,cah_frme;
fp_stat = fopen("/proc/meminfo","r");
while(fgets(buff,128,fp_stat))
{
if(strstr(buff, "MemTotal:"))
sscanf(buff,"MemTotal:	%lu",&memt);
if(strstr(buff,"MemFree:"))
sscanf(buff,"MemFree:	%lu",&memf);
if(strstr(buff,"Buffers:"))
sscanf(buff,"Buffers:	%lu",&buf);
if(strstr(buff,"Cached:"))
sscanf(buff,"Cached:	%lu",&cah);

}
fclose(fp_stat);
scale=(float)(memt-memf-buf-cah);
//printf("%lukb\n%lukb\n%lukb\n%lukb\nMemory utilization %5.1f%%\n",memt,memf,buf,cah,100*scale);
printf("Memory utilization= %.2lf/%.2lf(%.2lf%%) \n\n",scale,(float)memt,100*scale/(float)memt);

//memend
//iostart
static unsigned  long read ,read_sav,read_frme,
write,write_sav,write_frme;
fp_stat=fopen("/proc/diskstats","r");
while(fgets(buff,128,fp_stat))
{
if(strstr(buff,"sda"))
{
sscanf(buff,"%*d %*d sda %*d %*d %lu %*d %*d %*d %lu",&read,&write);
printf("read sector=%lu,write=%lu\n",read,write);
break;
}
}
fclose(fp_stat);
read_frme=read-read_sav;
write_frme=write-write_sav;
printf("Disk read = %.3fMB/s\nDisk write = %.3fMB/s \n",(float)read_frme*512/1000000,(float)write_frme*512/1000000);
read_sav=read;
write_sav=write;
}
//ioend

int main() {
struct itimerval tick ;
int res ;

/*設signal*/
signal(SIGALRM, stat) ;        /* SIGALRM handeler */
/* setting first time interval */
tick.it_value.tv_sec = 1 ;         // sec
tick.it_value.tv_usec = 0 ;        // usec
/* setting next time interval */
tick.it_interval.tv_sec = 1;          
tick.it_interval.tv_usec = 0 ;    
res = setitimer(ITIMER_REAL, &tick, NULL);//settimer
if(res)
fprintf(stderr, "Error: timer setting faul.\n") ;
else
printf("Timer start...\n") ;
/*set END*/
getchar();

return 0 ;
}
