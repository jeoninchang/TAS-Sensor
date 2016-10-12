#include <stdio.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include "ibeacon.h"

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <unistd.h>     
#include <time.h>

#define MAXPENDING 5   
#define RCVBUFSIZE 100 
#define MAX 45

char* SERVER_IP = "127.0.0.1"; 
int SERVER_PORT = 3110;

char* GetResult(char *s1);
void SendRequest(char *s1);
void UploadThingData(char *thingname, char *type, char *content);
void get_ibeacon_info(struct ibeacon_info *info);
void DieWithError(char *errorMessage);


int main(int argc, char *argv[])
{
	int dev_id = -1;
	char *hci_dev = "hci1";
	dev_id = init_bluetooth(hci_dev);

	start_lescan_loop(dev_id, get_ibeacon_info); //BLE scan start.
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

void UploadThingData(char *thingname, char *type, char *content)
{
	char *result;
	result = (char*)malloc((sizeof(char*))*(strlen(thingname) + strlen(content) + 1));

	sprintf(result, "%s, %s", thingname, content);

	SendRequest(result);

	free(result);

	result = NULL;
}

void get_ibeacon_info(struct ibeacon_info *info) 
{
	char beacon_name[15]= "";
	char beacon_data[80];

	time_t current_time;
	time(&current_time);
		
	int tx_power = 0;
	int rssi = 0;
	
	if (info->rssi > 128){ 
		rssi = 255 - info->rssi+1;
	}else {
		rssi = info->rssi+1;
	}

	if (info->tx_power > 128){
		tx_power = 255 - info->tx_power+1;
	}else {
		tx_power = info->tx_power+1;
	}
	rssi=rssi*(-1);
	tx_power=tx_power*(-1);
	sprintf(beacon_data, "%d,%d/%d", rssi, tx_power, current_time);

	if(info->mac[0] == 0x00 && info->mac[1] == 0x19 && info->mac[2] == 0x86 && info->mac[3] == 0x00 && info->mac[4] == 0x15 && info->mac[5] == 0x36) {
		strcpy(beacon_name, "rasp_sender");
	}

//00:19:86:00:15:36	

	//Scanned data upload.
	if (info->mac[0] == 0xf3 && info->mac[1] == 0x2c && info->mac[2] == 0x43 && info->mac[3] == 0x0a && info->mac[4] == 0x1a && info->mac[5] == 0xa8){
		strcpy(beacon_name, "beacon1");
	}

	else if (info->mac[0] == 0xc1 && info->mac[1] == 0x49 && info->mac[2] == 0x4e && info->mac[3] == 0xd0 && info->mac[4] == 0xf4 && info->mac[5] == 0x0d){
		strcpy(beacon_name, "beacon2");
	}

	else if (info->mac[0] == 0xfe && info->mac[1] == 0x7d && info->mac[2] == 0x0c && info->mac[3] == 0x27 && info->mac[4] == 0xed && info->mac[5] == 0xc1){
		strcpy(beacon_name, "beacon3");
	}

	else if (info->mac[0] == 0xea && info->mac[1] == 0xcb && info->mac[2] == 0x22 && info->mac[3] == 0xbf && info->mac[4] == 0x32 && info->mac[5] == 0x46){
		strcpy(beacon_name, "beacon4");
	}

	else if (info->mac[0] == 0xee && info->mac[1] == 0x6e && info->mac[2] == 0x8c && info->mac[3] == 0x4c && info->mac[4] == 0xd3 && info->mac[5] == 0xab){
		strcpy(beacon_name, "beacon5");
	}
	else if (info->mac[0] == 0xcb && info->mac[1] == 0xec && info->mac[2] == 0xca && info->mac[3] == 0xea && info->mac[4] == 0x6c && info->mac[5] == 0x4b){
		strcpy(beacon_name, "beacon6");
	}

	else if (info->mac[0] == 0xde && info->mac[1] == 0x5d && info->mac[2] == 0x1d && info->mac[3] == 0x65 && info->mac[4] == 0x4f && info->mac[5] == 0x57){
		strcpy(beacon_name, "beacon7");
	}

	else if (info->mac[0] == 0xd0 && info->mac[1] == 0x9f && info->mac[2] == 0xfc && info->mac[3] == 0x4e && info->mac[4] == 0x24 && info->mac[5] == 0x63){
		strcpy(beacon_name, "beacon8");
	}

	else if (info->mac[0] == 0xed && info->mac[1] == 0x8d && info->mac[2] == 0x36 && info->mac[3] == 0x76 && info->mac[4] == 0xee && info->mac[5] == 0x97){
		strcpy(beacon_name, "beacon9");
	}

	else if (info->mac[0] == 0xf2 && info->mac[1] == 0x54 && info->mac[2] == 0x7b && info->mac[3] == 0x5f && info->mac[4] == 0x23 && info->mac[5] == 0xb3){
		strcpy(beacon_name, "beacon10");
	}
	
	else if (info->mac[0] == 0xe3 && info->mac[1] == 0xc5 && info->mac[2] == 0x91 && info->mac[3] == 0x4e && info->mac[4] == 0x88 && info->mac[5] == 0xf0){
		strcpy(beacon_name, "beacon11");
	}

	if (info->mac[0] == 0xe1 && info->mac[1] == 0x0c && info->mac[2] == 0x36 && info->mac[3] == 0xc2 && info->mac[4] == 0xee && info->mac[5] == 0x8d){
		strcpy(beacon_name, "beacon12");
	}

	else if (info->mac[0] == 0xd2 && info->mac[1] == 0xb7 && info->mac[2] == 0x36 && info->mac[3] == 0x09 && info->mac[4] == 0x39 && info->mac[5] == 0x27){
		strcpy(beacon_name, "beacon13");
	}

	else if (info->mac[0] == 0xcb && info->mac[1] == 0x04 && info->mac[2] == 0xb8 && info->mac[3] == 0x8c && info->mac[4] == 0xc6 && info->mac[5] == 0x45){
		strcpy(beacon_name, "beacon14");
	}

	else if (info->mac[0] == 0xf1 && info->mac[1] == 0xd5 && info->mac[2] == 0xd9 && info->mac[3] == 0x8c && info->mac[4] == 0xd0 && info->mac[5] == 0x96){
		strcpy(beacon_name, "beacon15");
	}

	else if (info->mac[0] == 0xde && info->mac[1] == 0xc7 && info->mac[2] == 0xc0 && info->mac[3] == 0x14 && info->mac[4] == 0x94 && info->mac[5] == 0x22){
		strcpy(beacon_name, "beacon16");
	}




 	else if (info->mac[0] == 0xf2 && info->mac[1] == 0xa4 && info->mac[2] == 0x1e && info->mac[3] == 0x07 && info->mac[4] == 0x97 && info->mac[5] == 0x33){
		strcpy(beacon_name, "beacon17");
	}




	if (info->mac[0] == 0xeb && info->mac[1] == 0x2d && info->mac[2] == 0xde && info->mac[3] == 0x43 && info->mac[4] == 0x1c && info->mac[5] == 0xc0){
		strcpy(beacon_name, "beacon18");
	}


	else if (info->mac[0] == 0xdf && info->mac[1] == 0x38 && info->mac[2] == 0xbd && info->mac[3] == 0xff && info->mac[4] == 0xc7 && info->mac[5] == 0x9b){
		strcpy(beacon_name, "beacon19");
	}

	else if (info->mac[0] == 0xe3 && info->mac[1] == 0xc7 && info->mac[2] == 0x8d && info->mac[3] == 0xd3 && info->mac[4] == 0x89 && info->mac[5] == 0xb0){
		strcpy(beacon_name, "beacon20");
	}

	else if (info->mac[0] == 0xf0 && info->mac[1] == 0x59 && info->mac[2] == 0xf6 && info->mac[3] == 0x07 && info->mac[4] == 0x01 && info->mac[5] == 0xd7){
		strcpy(beacon_name, "beacon21");
	}

	else if (info->mac[0] == 0xe1 && info->mac[1] == 0x82 && info->mac[2] == 0xe9 && info->mac[3] == 0x9c && info->mac[4] == 0xea && info->mac[5] == 0x8d){
		strcpy(beacon_name, "beacon22");
	}

	else if (info->mac[0] == 0xf0 && info->mac[1] == 0x59 && info->mac[2] == 0x05 && info->mac[3] == 0x5a && info->mac[4] == 0x14 && info->mac[5] == 0x8a){
		strcpy(beacon_name, "beacon23");
	}

	else if (info->mac[0] == 0xc4 && info->mac[1] == 0x2f && info->mac[2] == 0x7a && info->mac[3] == 0xd3 && info->mac[4] == 0xb3 && info->mac[5] == 0x84){
		strcpy(beacon_name, "beacon24");
	}

	else if (info->mac[0] == 0xfc && info->mac[1] == 0x58 && info->mac[2] == 0x14 && info->mac[3] == 0xae && info->mac[4] == 0x1d && info->mac[5] == 0xc8){
		strcpy(beacon_name, "beacon25");
	}

	else if (info->mac[0] == 0xDC && info->mac[1] == 0x1C && info->mac[2] == 0x1D && info->mac[3] == 0x35 && info->mac[4] == 0x97 && info->mac[5] == 0xFD){
		strcpy(beacon_name, "beacon26");
	}

	else if (info->mac[0] == 0xEB && info->mac[1] == 0x18 && info->mac[2] == 0xFE && info->mac[3] == 0xA3 && info->mac[4] == 0xD5 && info->mac[5] == 0xBD){
		strcpy(beacon_name, "beacon27");
	}

	else if (info->mac[0] == 0xC2 && info->mac[1] == 0x52 && info->mac[2] == 0xCB && info->mac[3] == 0x56 && info->mac[4] == 0xB9 && info->mac[5] == 0xAC){
		strcpy(beacon_name, "beacon28");
	}

	else if (info->mac[0] == 0xD5 && info->mac[1] == 0x5C && info->mac[2] == 0x27 && info->mac[3] == 0x15 && info->mac[4] == 0x79 && info->mac[5] == 0xA2){
		strcpy(beacon_name, "beacon29");
	}

	else if (info->mac[0] == 0xD6 && info->mac[1] == 0xEE && info->mac[2] == 0x9F && info->mac[3] == 0x22 && info->mac[4] == 0x42 && info->mac[5] == 0x70){
		strcpy(beacon_name, "beacon30");
	}

	else if (info->mac[0] == 0xFF && info->mac[1] == 0x6D && info->mac[2] == 0xCE && info->mac[3] == 0x0B && info->mac[4] == 0xA0 && info->mac[5] == 0xA6){
		strcpy(beacon_name, "beacon31");
	}

	else if (info->mac[0] == 0xCD && info->mac[1] == 0x12 && info->mac[2] == 0x10 && info->mac[3] == 0x44 && info->mac[4] == 0x6E && info->mac[5] == 0x1D){
		strcpy(beacon_name, "beacon32");
	}

	else if (info->mac[0] == 0xF5 && info->mac[1] == 0x1D && info->mac[2] == 0xF3 && info->mac[3] == 0xB4 && info->mac[4] == 0xA1 && info->mac[5] == 0xD2){
		strcpy(beacon_name, "beacon33");
	}

	else if (info->mac[0] == 0xC3 && info->mac[1] == 0xB0 && info->mac[2] == 0x8E && info->mac[3] == 0xBA && info->mac[4] == 0xF1 && info->mac[5] == 0xA7){
		strcpy(beacon_name, "beacon34");
	}

	if (info->mac[0] == 0xF9 && info->mac[1] == 0xD7 && info->mac[2] == 0x20 && info->mac[3] == 0xB8 && info->mac[4] == 0x2B && info->mac[5] == 0x10){
		strcpy(beacon_name, "beacon35");
	}

	else if (info->mac[0] == 0xC6 && info->mac[1] == 0xE6 && info->mac[2] == 0xC4 && info->mac[3] == 0xAF && info->mac[4] == 0x57 && info->mac[5] == 0x4F){
		strcpy(beacon_name, "beacon36");
	}

	else if (info->mac[0] == 0xE4 && info->mac[1] == 0x5D && info->mac[2] == 0x56 && info->mac[3] == 0xD0 && info->mac[4] == 0x99 && info->mac[5] == 0x46){
		strcpy(beacon_name, "beacon37");
	}

	else if (info->mac[0] == 0xEA && info->mac[1] == 0xAA && info->mac[2] == 0xC7 && info->mac[3] == 0x57 && info->mac[4] == 0x96 && info->mac[5] == 0x28){
		strcpy(beacon_name, "beacon38");
	}

	else if (info->mac[0] == 0xF7 && info->mac[1] == 0xE9 && info->mac[2] == 0xE8 && info->mac[3] == 0xEC && info->mac[4] == 0xB8 && info->mac[5] == 0xC5){
		strcpy(beacon_name, "beacon39");
	}

	else if (info->mac[0] == 0xC6 && info->mac[1] == 0x26 && info->mac[2] == 0x72 && info->mac[3] == 0x61 && info->mac[4] == 0xD1 && info->mac[5] == 0xA1){
		strcpy(beacon_name, "beacon40");
	}

	else if (info->mac[0] == 0xFA && info->mac[1] == 0x68 && info->mac[2] == 0x4E && info->mac[3] == 0x46 && info->mac[4] == 0x67 && info->mac[5] == 0x24){
		strcpy(beacon_name, "beacon41");
	}

	else if (info->mac[0] == 0xEB && info->mac[1] == 0x93 && info->mac[2] == 0x75 && info->mac[3] == 0x84 && info->mac[4] == 0x54 && info->mac[5] == 0x3D){
		strcpy(beacon_name, "beacon42");
	}

	else if (info->mac[0] == 0xD7 && info->mac[1] == 0x06 && info->mac[2] == 0xC8 && info->mac[3] == 0xA7 && info->mac[4] == 0x6A && info->mac[5] == 0xF8){
		strcpy(beacon_name, "beacon43");
	}

	else if (info->mac[0] == 0xE1 && info->mac[1] == 0x15 && info->mac[2] == 0x96 && info->mac[3] == 0xD4 && info->mac[4] == 0x09 && info->mac[5] == 0x56){
		strcpy(beacon_name, "beacon44");
	}

	else if (info->mac[0] == 0xD6 && info->mac[1] == 0xD7 && info->mac[2] == 0xA6 && info->mac[3] == 0x5E && info->mac[4] == 0x05 && info->mac[5] == 0x83){
		strcpy(beacon_name, "beacon45");
	}


	if(strcmp(beacon_name, "") != 0){
		printf("%s, %s\n", beacon_name, beacon_data);
		UploadThingData(beacon_name, "beacon", beacon_data);
	}

	
}

