#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>	
#include <time.h>
int CPUPipe[2],memPipe[2],FSWPipe[2],FSRPipe[2];

void *CPUUsage(void *threadid)
{
	while(1){
		char temp[10];
		int user1,user2,nice1,nice2,system1,system2;
		FILE* procFile;
		close(CPUPipe[0]);
		
		//The 4 first values in /proc/stat are cpu user, nice, system, and idle times
		procFile = fopen("/proc/stat", "r");
		fscanf(procFile, "%s\t%d\t%d\t%d\n", temp, &user1, &nice1, &system1);
		fclose(procFile);

		sleep(1);

		procFile = fopen("/proc/stat", "r");
		fscanf(procFile, "%s\t%d\t%d\t%d\n", temp, &user2, &nice2, &system2);
		fclose(procFile);
		
		int res = (user2+system2)-(user1+system1);
		char ret[100];
		sprintf ( ret, "%d", res );
		int retsize=strlen(ret);
		
		
		write(CPUPipe[1], ret, strlen(ret)+1);
		//pthread_exit(NULL);
	}
}
void *MemUsage(void *threadid)
{
	char ctemp[100];
	int total;
	int free;
	FILE* memFile;
	close(memPipe[0]);

	memFile = fopen("/proc/meminfo", "r");
	fscanf(memFile, "%s\t%d\t%s\n%s\t%d", ctemp, &total, ctemp, ctemp, &free );
	fclose(memFile);
	
	int res=(total-free)*100/total;
	char ret[100];
	sprintf ( ret, "%d", res );
	int retsize=strlen(ret);	

	write(memPipe[1], ret, strlen(ret)+1);
	pthread_exit(NULL);
}
void *FSW(void *threadid)
{
	close(FSWPipe[0]);

	FILE* FSWFile;
	char temp[1024],ctemp[100],ret[100];
	int fsw1,fsw2;

	FSWFile = fopen("/proc/diskstats", "r");
	while (1){
		fgets(temp, 1024, FSWFile);
		if (strstr(temp, "sda "))
			break;
	}
	sscanf(temp, "\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d", ctemp, ctemp, ctemp, ctemp, ctemp, ctemp, ctemp, &fsw1);
	fclose(FSWFile);
	sleep(1);

	FSWFile = fopen("/proc/diskstats", "r");
	while (1){
		fgets(temp, 1024, FSWFile);
		if (strstr(temp, "sda "))
			break;
	}
	sscanf(temp, "\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d", ctemp, ctemp, ctemp, ctemp, ctemp, ctemp, ctemp, &fsw2);
	fclose(FSWFile);

	int res=fsw2-fsw1;
	sprintf ( ret, "%d", res );
	int retsize=strlen(ret);

	write(FSWPipe[1], ret, strlen(ret)+1);
	pthread_exit(NULL);
}
void *FSR(void *threadid)
{
	close(FSRPipe[0]);

	FILE* FSRFile;
	char temp[1024],ctemp[100],ret[100];
	int fsr1,fsr2;

	FSRFile = fopen("/proc/diskstats", "r");
	while (1){
		fgets(temp, 1024, FSRFile);
		if (strstr(temp, "sda "))
			break;
	}
	sscanf(temp, "\t%s\t%s\t%s\t%d", ctemp, ctemp, ctemp, &fsr1);
	fclose(FSRFile);
	sleep(1);

	FSRFile = fopen("/proc/diskstats", "r");
	while (1){
		fgets(temp, 1024, FSRFile);
		if (strstr(temp, "sda "))
			break;
	}
	sscanf(temp, "\t%s\t%s\t%s\t%d", ctemp, ctemp, ctemp, &fsr2);
	fclose(FSRFile);

	int res=fsr2-fsr1;
	sprintf ( ret, "%d", res );
	int retsize=strlen(ret);

	write(FSRPipe[1], ret, strlen(ret)+1);
	pthread_exit(NULL);
}
int main(void)
{
    pid_t process;
    pipe(CPUPipe);
    pipe(memPipe);
    pipe(FSWPipe);
    pipe(FSRPipe);
    

    process = fork();

    if(process >= 0){
        if(process == 0){
			//Create 4 threads
			pthread_t cpu, mem, fsw, fsr;
			long t;
			int ptcpu, ptmem, ptfsw, ptfsr;
			while (1){
				ptcpu = pthread_create(&cpu, NULL, CPUUsage, (void *)t);
				ptmem = pthread_create(&mem, NULL, MemUsage, (void *)t);
				ptfsw = pthread_create(&fsw, NULL, FSW, (void *)t);
				ptfsr = pthread_create(&fsr, NULL, FSR, (void *)t);
				sleep(5);
			}
			
        }
        else{
			//Gets info through 4 pipes
			char readbuffer[80];
			while (1){
				close(CPUPipe[1]);
				close(memPipe[1]);
				close(FSWPipe[1]);
				close(FSRPipe[1]);
				//wait for the cpu usage to be calculated
				sleep(1);
				
				time_t t;
				t=time(0);
				printf("%s",ctime(&t));
				read(CPUPipe[0], readbuffer, sizeof(readbuffer));
				printf("CPU Usage: %s%\n", readbuffer);
				read(memPipe[0], readbuffer, sizeof(readbuffer));
				printf("Memory Usage: %s%\n", readbuffer);
				read(FSWPipe[0], readbuffer, sizeof(readbuffer));
				printf("FS Write: %s B/sec\n", readbuffer);
				read(FSRPipe[0], readbuffer, sizeof(readbuffer));
				printf("FS Read: %s B/sec\n", readbuffer);
				printf("------------------------------\n");
				sleep(4);
			}
        }
    }
    else{
        printf("\n Fork failed, quitting!\n");
        return 1;
    }

    return 0;
}
