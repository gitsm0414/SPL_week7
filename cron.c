#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

int main(void) {
    unsigned int pid;
    time_t t;
    struct tm* tm;
    int fd;
    char* argv[3];
    char buf[512];
    int fd0, fd1, fd2;
    
    int status; // exit status of child process
    pid_t wpid; //return value of waitpid
    ssize_t res; //result of read function
    char* tmp; //temporary pointer of buf
    char* token;
    fd = open("./crontab", O_RDWR);
    
    if (errno == ENOENT) {
        perror("crontab does not exist");
        exit(1);
    }

	switch (fork()) {
		case -1: return -1;
		case 0: break;
		default: _exit(0);
	}

    if(setsid() < 0) {
        perror("Failed to create a new session");
        exit(2);
    }

    if(chdir("/") < 0) {
        perror("Failed to change directory to root directory");
        exit(3);
    }

    umask(0);

    for (int i = 0; i < 3; i++) close(i);

    fd0 = open("/dev/null", O_RDWR);
    fd1 = open("/dev/null", O_RDWR);
    fd2 = open("/dev/null", O_RDWR);

    openlog("cron", LOG_PID, LOG_CRON);
    
    if((res = read(fd, buf, sizeof(buf))) < 0){
	syslog(LOG_EMERG, "crontab reading errer")
	exit(4);
    }
    tmp = buf;
    
    int i = 0;
    //reading and saving arguments
    while((token = strtok_r(tmp, " ", &tmp))){
	*(argv+i) = token;
	i++;
    }

    int min = -1;
    int hour = -1;

    if(*argv[0]!='*')
	    min = strtol(argv[0], NULL, 10);
    if(*argv[1]!='*')
	    hour = strtol(argv[1], NULL, 10);

    

    while (1) {
	t = time(NULL);
	tm = localtime(&t);
	
        if(min==-1 && hour==-1){
	    pid = fork();
	    if(pid == 0){
		if(execl("/bin/sh", "/bin/sh", "-c", argv[2], (char*)NULL)==-1){
		    exit(5);
		}
		exit(0);
	    }	
	}
	else if(min==-1){
		if(hour == tm->tm_hour){
		    pid = fork();
		    if(pid == 0){
	                if(execl("/bin/sh", "/bin/sh", "-c", argv[2], (char*)NULL)==-1){
		    	    exit(5);
			}
			exit(0);
	    	    }
		}
	}else if(hour==-1){
		if(min == tm->tm_min){
		    pid = fork();
	    	    if(pid == 0){
	 	        if(execl("/bin/sh", "/bin/sh", "-c", argv[2], (char*)NULL)==-1){
		    	    exit(5);
			}
		        exit(0);
	    	    } 
		}
	}else{
		if(hour==tm->tm_hour && min==tm->tm_min){
		    pid = fork();
	    	    if(pid == 0){
		    	if(execl("/bin/sh", "/bin/sh", "-c", argv[2], (char*)NULL)==-1){
			    exit(5);
			}
			exit(0);
		    }	    
		}
	}
	
        sleep(60 - tm->tm_sec % 60);

	wpid = waitpid(pid, &status, WNOHANG);
	if(wpid > 0){
	    if(status==5) exit(1); //if child process exited with error, then terminate cron
	}
    }

    return 0;
}
