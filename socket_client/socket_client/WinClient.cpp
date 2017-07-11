#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include "stdafx.h"
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <conio.h>

void blobstore(char buffer[], int buffer_len, int hsock);
void messagestore(char buffer[], int buffer_len, int hsock);
void calendarstore(char buffer[], int buffer_len, int hsock);
void blob_signup(char buffer[], int buffer_len, int hsock);
void blob_signin(char buffer[], int buffer_len, int hsock);
void addFiles(int hsock);
void downloadFile(int hsock);
void displayFile(int hsock);
void deleteFile(int hsock);

int getsocket()
{
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		return -1;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		return -1;
	}
	free(p_int);

	return hsock;
}

void socket_client()
{

	//The port and address you want to connect to
	int host_port= 1101;
	char* host_name="127.0.0.1";

	//Initialize socket support WINDOWS ONLY!
	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "Could not find sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set any options

	//Connect to the server
	struct sockaddr_in my_addr;

	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = inet_addr(host_name);

	//if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR ){
	//	fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
	//	goto FINISH;
	//}

	//Now lets do the client related stuff
	char buffer[1024];
	int buffer_len = 1024;
	int bytecount;
	int c;

//	while(true) {

		int hsock = getsocket();
		//add error checking on hsock...
		if( connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR ){
			fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
			goto FINISH;
		}
		while (1)
		{
			memset(buffer, '\0', buffer_len);
			/*printf("Enter your message to send here\n");
			for(char* p=buffer ; (c=getch())!=13 ; p++){
			printf("%c", c);
			*p = c;
			}

			if( (bytecount=send(hsock, buffer, strlen(buffer),0))==SOCKET_ERROR){
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			goto FINISH;
			}
			printf("Sent bytes %d\n", bytecount);
			*/
			if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
				goto FINISH;
			}
			//printf("Recieved bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
			printf("%s\n", buffer);
			char choice[30];
			scanf("%s", choice);
			strcpy(buffer, choice);
			if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
				goto FINISH;
			}
			char ch = choice[0] - '0';
			switch (ch)
			{
			case 1:
				messagestore(buffer, buffer_len,hsock);
				break;
			case 2:
				blobstore(buffer, buffer_len, hsock);
				break;
			case 3:
				calendarstore(buffer, buffer_len, hsock);
				break;

			}
		}
		closesocket(hsock);
	//}

	//closesocket(hsock);
FINISH:
;
}

void messagestore(char buffer[],int buffer_len,int hsock)
{
	while (1)
	{
		int bytecount = 0;
		memset(buffer, '\0', buffer_len);
		/*printf("Enter your message to send here\n");
		for(char* p=buffer ; (c=getch())!=13 ; p++){
		printf("%c", c);
		*p = c;
		}

		if( (bytecount=send(hsock, buffer, strlen(buffer),0))==SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		goto FINISH;
		}
		printf("Sent bytes %d\n", bytecount);
		*/
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
		//printf("Recieved bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
		printf("%s\n", buffer);
		//char ch[2];
		//memset(ch, '\0', 2);
		memset(buffer, '\0', buffer_len);
		fflush(stdin);
		gets(buffer);
		//strcpy(buffer, ch);
		if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
	}
}

void calendarstore(char buffer[], int buffer_len, int hsock)
{
	while (1)
	{
		int bytecount = 0;
		memset(buffer, '\0', buffer_len);
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
		printf("%s\n", buffer);
		char ch[1024];
		memset(ch, '\0', 1024);
		scanf("%s", ch);
		memset(buffer, '\0', buffer_len);
		strcpy(buffer, ch);
		if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
	}
}

void blobstore(char buffer[],int buffer_len,int hsock)
{
	while (1)
	{
		int bytecount = 0;
		memset(buffer, '\0', buffer_len);
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
		printf("%s\n", buffer);
		char ch[2];
		memset(ch, '\0',2);
		scanf("%s", ch);
		strcpy(buffer, ch);
		if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
		int choice = ch[0] - '0';
		switch (choice)
		{
		case 1:
			blob_signup(buffer, buffer_len, hsock);
			break;
		case 2:
			blob_signin(buffer, buffer_len, hsock);
			break;
		case 3:
			return;
		}
	}
}

void blob_signup(char buffer[], int buffer_len, int hsock)
{
	memset(buffer, '\0', buffer_len);
	int bytecount = 0;
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	char username[28];
	memset(username, '\0', 28);
	printf("%s\n", buffer);
	scanf("%s", username);
	memset(buffer, '\0', buffer_len);
	strcpy(buffer, username);
	if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
}

void blob_signin(char buffer[], int buffer_len, int hsock)
{
		memset(buffer, '\0', buffer_len);
		int bytecount = 0;
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
		char username[28];
		memset(username, '\0', 28);
		printf("%s\n", buffer);
		scanf("%s", username);
		memset(buffer, '\0', buffer_len);
		strcpy(buffer, username);
		if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
		while (1)
		{
		memset(buffer, '\0', buffer_len);
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
		printf("%s", buffer);
		if (!strcmp(buffer, "\nEnter a valid username\n"))
		{
			return;
		}
		char ch[2];
		memset(ch, '\0', 2);
		scanf("%s", ch);
		//fflush(stdin);
		//memset(buffer, '\0', buffer_len);
		//strcpy(buffer, ch);
		//gets(buffer);
		if ((bytecount = send(hsock, ch, strlen(ch), 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
		int choice = ch[0] - '0';
		switch (choice)
		{
		case 1:
			addFiles(hsock);
			break;
		case 2:
			displayFile(hsock);
			break;
		case 3:
			downloadFile(hsock);
			break;
		case 4:
			deleteFile(hsock);
			break;
		case 5:
			return;
		}
	}
}

void addFiles(int hsock)
{
	char flag[32];
	int bytecount = 0;
	memset(flag, '\0', 32);
	if ((bytecount = recv(hsock, flag, 32, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	printf("%s", flag);
	char filename[16];
	memset(filename, '\0', 16);
	scanf("%s", filename);
	char filepath[100];
	memset(filepath, '\0', 100);
	strcpy(filepath, "C:\\Users\\Bhavisya\\Documents\\Binary File\\");
	strcat(filepath, filename);
	FILE *fp = fopen(filepath, "rb+");
	if ((bytecount = send(hsock, filename, strlen(filename), 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	fseek(fp, 0, SEEK_END);
	int length_of_file = ftell(fp);
	char filelength[32];
	memset(filename, '\0', 16);
	memset(filelength, '\0', 32);
	memset(flag, '\0', 32);
	if ((bytecount = recv(hsock, flag, 32, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	sprintf(filelength, "%d", length_of_file);
	if ((bytecount = send(hsock, filelength, strlen(filelength), 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	memset(flag, '\0', 32);
	if ((bytecount = recv(hsock, flag, 32, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	int sizeread = 0;
	fseek(fp, 0, SEEK_SET);
	char *buffer = (char *)malloc((length_of_file+4)*sizeof(char));
	char temp[2];
	memset(buffer, '\0', (length_of_file+4));
	int index = 0;
	/*while (!feof(fp))
	{
		char ch = fgetc(fp);
		memset(temp, '\0', 2);
		temp[0] = ch;
		strcat(buffer, temp);
		index++;
	}*/
	fread(buffer, length_of_file, 1, fp);
//	buffer[index] = '\0';
	if ((bytecount = send(hsock, buffer, length_of_file, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	char done[15];
	memset(done, '\0', 15);
	if ((bytecount = recv(hsock, done, 15, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
	strcpy(buffer, "aipoindi..");
	if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	printf("%s\n", done);
}

void displayFile(int hsock)
{
	int buffer_len = 1024, bytecount;
	char buffer[1024];
	memset(buffer, '\0', 1024);
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	printf("%s", buffer);
	strcpy(buffer, "aipoindi..");
	if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
}

void downloadFile(int hsock)
{
	char buffer[1000];
	int buffer_len = 1000, bytecount;
	memset(buffer, '\0',buffer_len);
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	printf("%s",buffer);
	memset(buffer, '\0', 1000);
	scanf("%s", buffer);
	if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	char filepath[100];
	memset(filepath, '\0', 100);
	strcpy(filepath, "C:\\Users\\Bhavisya\\Documents\\Binary File\\verify\\");
	strcat(filepath, buffer);
	FILE *temp = fopen(filepath, "wb");
	while (1)
	{
		memset(buffer, '\0', buffer_len);
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
		//buffer[bytecount] = '\0';
		
		if (!strcmp(buffer, "file has been sent successfully"))
			break;
		fwrite(buffer, bytecount, 1, temp);
		memset(buffer, '\0', buffer_len);
		strcpy(buffer, "send file part\n");
		if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			printf("Error");
			return;
		}
	}
	fclose(temp);
	strcpy(buffer, "aipoindi..");
	if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
}

void deleteFile(int hsock)
{
	int buffer_len = 1024, bytecount;
	char buffer[1024];
	memset(buffer, '\0', 1024);
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	printf("%s", buffer);
	memset(buffer, '\0', buffer_len);
	scanf("%s", buffer);
	if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		printf("Error");
		return;
	} 
	memset(buffer, '\0', buffer_len);
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
	printf("%s", buffer);
	
	strcpy(buffer, "aipoindi..");
	if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		printf("Error");
		return;
	}
}