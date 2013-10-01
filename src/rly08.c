/* rly08.c */
/* (c) 2008 by Javier Infante -- jabi(at)irontec.com */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<termios.h>
#include<sys/types.h>

#define VERSION "1.0"

static void PrintVersion() {
	printf("[rly08 version %s]",VERSION);
	printf("\n\n");
}

static void Help() {
	PrintVersion();
	printf("Usage: ./rly08 [port /dev/ttyN] [dir n] command [parameters]:\n\n");
	printf("\tif no port is specified /dev/ttyUSB0 will be used.\n\n");
	printf("\tif no dir is specified (1-8) 1 will be used.\n\n");
	printf("Commands:\n");
	printf("=========\n\n");
	printf("\ton [1,2,3,n]\n");
	printf("\t\tThis command will turn on the specified reles.\n");	
	printf("\t\tExample: ./rly08 on 4,5,6 (will turn on reles 4, 5 and 6 and do nothing with the rest).\n");
	printf("\toff [1,2,3,n]\n");
	printf("\t\tThis command will turn off the specified reles.\n");
	printf("\t\tExample: ./rly08 off 4,5,6 (will turn off reles 4, 5 and 6 and do nothing with the rest).\n");
	printf("\tset 1111111\n");
	printf("\t\tThis command must have 8 binary digit as input. Each bit will represent the state of the associated rele.\n");
	printf("\t\tExample: ./rly08 set 11110000  (will turn on reles 1-4, and turn off reles 5-8).\n");
	printf("\tget n\n");
	printf("\t\tThis command will display the state of the specified rele and exit.\n");
	printf("\t\tExample: ./rly08 get 3  (will display 0 or 1, and also will use it as the exit code).\n");
	printf("\tversion\n");
	printf("\t\tThis command will display Module ID and Software version of the rele and exit.\n");
	printf("\t\tExample: ./rly08 version\n");
	printf("\n\n");
}


int main(int argc,char *argv[])
{
	int fd,i,start,dir,base;
	unsigned char params[3];
	unsigned char buffer[3];
	char port[25];
	struct termios options;


	if (argc == 1) {
		PrintVersion();
		printf("Too few parameters!\n");
		printf("Try -h for help!\n\n");
		
		exit(1);
	}

	if (strcasecmp(argv[1], "--help") == 0 || strcasecmp(argv[1], "-h") == 0 || strcasecmp(argv[1], "help") == 0) {
		Help();
		exit(0);
	}


	if (strcasecmp(argv[1], "port")==0) {
		strcpy(port,argv[2]);
		start = 3;
	} else {
		strcpy(port,"/dev/ttyUSB0");
		start = 1;
	}

	if (strcasecmp(argv[start], "dir")==0) {
		dir = atoi(argv[start+1]);
		if ( (dir>0) && (dir<9) ) params[0]=dir;
		else params[0] = 1;
		start = start+2;
	} else {
		params[0]=1;
	}

	fd = open(port,O_RDWR | O_NOCTTY | O_NDELAY);
	
	if(fd == -1) {
		printf("Unable to open %s\n",port);
		exit(2);
	}


	fcntl(fd,F_SETFL,0);
	tcgetattr(fd,&options); // get default values

	cfsetispeed(&options,B19200);
	cfsetospeed(&options,B19200);
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag |= CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag &= ~CRTSCTS; // sin control HW control
	options.c_iflag &= ~(IXON | IXOFF | IXANY);
	options.c_iflag = IGNPAR;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;
	options.c_cc[VMIN] = 1;
	tcsetattr(fd,TCSANOW,&options); // set new values
	

	
	if (strcasecmp(argv[start], "version") == 0) {
			params[1]=90;
			write(fd,params,2);
			i = read(fd,buffer,2);
			if(i<0) {
				fprintf(stderr,"Read failed\n");
				exit(3);
			}
			printf("Module ID: %x\nSoft version: %x\n",buffer[0],buffer[1]);
	} else if ((strcasecmp(argv[start], "on") == 0) || (strcasecmp(argv[start], "off") == 0)) {
			
			if (strcasecmp(argv[start], "on") == 0) base = 100;
			else base = 110;
			
			for(i=0;i<strlen(argv[start+1]);i++) {
				switch(atoi(&argv[start+1][i])) {
					case 1: case 2: case 3:	case 4:case 5:case 6:case 7:case 8:
						params[1] = base+atoi(&argv[start+1][i]);
						write(fd,params,2);
					break;
					default:continue;
				}
			
			}	
	} else if (strcasecmp(argv[start], "get") == 0) {
			if (argc<=(start+1)) {
				printf("This command must have the specified rele as input [1-9 | 0 for all in binary mode].\n");
				exit(5);
			}
			params[1]=91;
			write(fd,params,2);
			i = read(fd,buffer,1);
			if(i<0) {
				fprintf(stderr,"Read failed\n");
				exit(3);
			}
			switch(atoi(argv[start+1])) {
					case 1: case 2: case 3:	case 4:case 5:case 6:case 7:case 8:
						buffer[0] = (buffer[0]>>(atoi(argv[start+1])-1)) & 1;
						printf("%i\n",buffer[0]);
						exit(buffer[0]);
					break;
					default:
						for(i=0;i<8;i++) {
							printf("%d",buffer[0] & 1);
							buffer[0] = buffer[0]>>1; 
						}
						printf("\n");
						exit(0);
					break;
				}		
	} else if (strcasecmp(argv[start], "set") == 0) {
			if ( (argc<=(start+1)) || (strlen(argv[start+1])!=8)) {
				printf("This command must have 8 binary digits as input.\n");
				exit(5);
			}
			
			params[1] = 92;
			params[2] = 0;
         for(i=0;i<8;i++) {
            if(argv[start+1][i]==49) { // 49 == "1"
            	params[2] += 1<<i;
            }
			}
			write(fd,params,3);
   }	

	close(fd);
	exit(0);

}
