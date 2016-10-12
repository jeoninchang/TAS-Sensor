#include <stdio.h>

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <unistd.h>     
#include <stdint.h>
#include <errno.h>
#include <setjmp.h>

//wiring Pi
#include <wiringPi.h>
#include <wiringSerial.h>

#define MAXPENDING 5   
#define RCVBUFSIZE 100 
#define MAX 45

#define TRY do{ jmp_buf ex_buf__; if( !setjmp(ex_buf__) ){
#define CATCH } else {
#define ETRY } }while(0)
#define THROW longjmp(ex_buf__, 1)

// Find Serial device on Raspberry with ~ls /dev/tty*
// ARDUINO_UNO "/dev/ttyACM0"
// FTDI_PROGRAMMER "/dev/ttyUSB0"
// HARDWARE_UART "/dev/ttyAMA0"
char device[]= "/dev/ttyACM0";

// filedescriptor
int fd;
unsigned long baud = 9600;
unsigned long time=0;
char buffer[200]="";

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT  3120

char* GetResult(char *s1);
void SendRequest(char *s1);
void UploadThingData(char *thingname, char *content);
void DieWithError(char *errorMessage);
void setup();
void loop();

int main(){
	setup();
	while(1) loop();
	return 0;
}

void DieWithError(char *errorMessage)
{
	perror(errorMessage);
	exit(1);
}

char* GetResult(char *s1)
{
	int sock;
	struct sockaddr_in ServerAddr;
	unsigned short ServerPort;
	unsigned int StringLen;
	int bytesRcvd, totalBytesRcvd;

	char *Buffer[500];

	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		DieWithError("sock() failed");
	}

	memset(&ServerAddr, 0, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	ServerAddr.sin_port = htons(SERVER_PORT);

	if (connect(sock, (struct sockaddr *) &ServerAddr, sizeof(ServerAddr)) < 0){
		DieWithError("connect() failed");
	}

	StringLen = strlen(s1);

	if (send(sock, s1, StringLen, 0) != StringLen){
		DieWithError("send() sent a different number of bytes than expected");
	}

	totalBytesRcvd = 0;

	read(sock, Buffer, StringLen);
	close(sock);

	return *Buffer;

}

void SendRequest(char *s1)
{
	int sock;
	struct sockaddr_in ServerAddr;
	unsigned short ServerPort;
	unsigned int StringLen;
	int bytesRcvd, totalBytesRcvd;

	char *Buffer[300];

	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		DieWithError("sock() failed");
	}

	memset(&ServerAddr, 0, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	ServerAddr.sin_port = htons(SERVER_PORT);

	if (connect(sock, (struct sockaddr *) &ServerAddr, sizeof(ServerAddr)) < 0){
		DieWithError("connect() failed");
	}

	StringLen = strlen(s1);

	if (send(sock, s1, StringLen, 0) != StringLen){
		DieWithError("send() sent a different number of bytes than expected");
	}

	close(sock);
}

void UploadThingData(char *thingname, char *content)
{
	char *result;
	result = (char*)malloc((sizeof(char*))*(strlen(thingname) + strlen(content) + 1));

	sprintf(result, "%s, %s", thingname, content);

	SendRequest(result);

	free(result);

	result = NULL;
}

void setup(){

  printf("%s \n", "Raspberry Startup!");
  fflush(stdout);

  //get filedescriptor
  if ((fd = serialOpen (device, baud)) < 0){
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    exit(1); //error
  }

  //setup GPIO in wiringPi mode
  if (wiringPiSetup () == -1){
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
    exit(1); //error
  }
}

int idx = 0;
int strMatch = 0;

void loop(){
	// Pong every 3 seconds
	if(millis()-time>=3000){
		serialPuts (fd, "Fong!\n");
		// you can also write data from 0-255
		// 65 is in ASCII 'A'
		serialPutchar (fd, 65);
		time=millis();
	}

	// read signal
	if(serialDataAvail (fd)){
		char newChar = serialGetchar (fd);
		//printf("%c", newChar);
		//fflush(stdout);
		
		//TRY{
		if(newChar == '/'){
			printf("\nbuffer : %s\n", buffer);
			printf("buffer[%d] : %c\n", idx, buffer[idx]); 
			//if(buffer[0] == 't'){
				printf("Upload\n");
				UploadThingData("Arduino", buffer);
			//}
			idx = -1;
			memset(buffer, 0, strlen(buffer));
			
			//buffer[idx] = newChar;
			//printf("buffer_after : %s\n", buffer);
		}else{
			buffer[idx] = newChar;
			printf("%c", newChar);
			fflush(stdout);
		}
		

		//}CATCH{
		//	printf("Buffer Error\n");
		//}
		//ETRY;
		idx++;
	}	
}


