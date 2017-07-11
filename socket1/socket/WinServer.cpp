#include "stdafx.h"
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define BLOCKSIZE 256
#define TOTALMEMORY 16*1024*1024
#define TOTALBLOCKS TOTALMEMORY/BLOCKSIZE
#define	BITMAPBLOCKS TOTALBLOCKS/BLOCKSIZE
#define MAXUSERS 16
#define MAXCATEGORIES 160
#define CATEGORYPERUSER 10
#define ENDOFBITMAP BITMAPBLOCKS*BLOCKSIZE
#define ENDOFMETA ENDOFBITMAP+3*BLOCKSIZE
#define NOOFRESERVEDBLOCKS 343

#define BLOBSIZE 1024*1024*1024
#define BLOB_BLOCK_SIZE 1024
#define BLOB_BLOCKS BLOBSIZE/BLOB_BLOCK_SIZE
#define BLOBUSER 32
#define BLOBFILE 1024
#define BLOB_BITMAP_BLOCKS BLOB_BLOCKS/BLOB_BLOCK_SIZE
#define BLOBMAXUSERS 32
#define ENDOF_BLOB_BITMAP (BLOB_BITMAP_BLOCKS*BLOB_BLOCK_SIZE)
#define ENDOF_BLOB_USERS (ENDOF_BLOB_BITMAP+BLOBUSER*BLOBMAXUSERS)
#define FILEMETA_BLOCKS 4076
#define FILEMETAEND ENDOF_BLOB_USERS+FILEMETA_BLOCKS*BLOB_BLOCK_SIZE
#define BLOBRESERVED BLOB_BITMAP_BLOCKS+1+4076
#define REMAINING_BLOCKS_FOR_FILES (BLOB_BLOCKS-BLOB_BITMAP_BLOCKS-1)
#define REMAINING_BLOCKS_FOR_STORING (BLOB_BLOCKS-(BLOB_BITMAP_BLOCKS+1+FILEMETA_BLOCKS))

#define calendar_total_bytes 100*1024*1024
#define calendar_max_category 10
#define calendar_max_category_size 16
#define end_of_category (calendar_max_category*calendar_max_category_size)
#define start_of_servicemen end_of_category+4
#define size_of_serviceman 64
#define calendar_max_serviceman 200
#define size_of_appointment 32 
#define serivce_men_meta 200*64
#define end_of_servicemen (end_of_category+serivce_men_meta)
#define start_of_appointment (end_of_servicemen+4)

struct user
{
	char name[20];
	int category[10];
	int filledcategories;

};

struct category
{
	char name[24];
	int messageOffsets[25];
	int filledMessages;
};

struct message
{
	char text[192];
	int userOffset;
	int replyOffset;
	char unused[56];
};

struct blobuser
{
	char name[28];
	int noOffiles;
};

struct blobfile
{
	char data[1000];
	char title[16];
	int nextPart;
	int userOffset;
};

struct calendarCategory
{
	char name[12];
	int servicemanOffset;
};

struct serviceman
{
	char name[22];
	char role[23];
	char phone[11];
	int firstappointmentOffset;
	int nextServicemanOffset;
};

struct appointment
{
	char username[16];
	int next = 0;
	int date[3];
};

struct node
{
	char msg[128];
	int msg_id;
	node *next;
}*flist,*alist,*printid;

struct bufserv{
	
		int userId;
		int forumId;
		int msgId;
		int commentId;
		int choice;
		char *forumname;
		char msg[128];
}buf1;



bool flag=true;
int mid = 0;
int count1 =0;
char *Data[100];
int count=1;
int values[100];

DWORD WINAPI SocketHandler(void*);
void replyto_client(char *buf, int *csock);
void communication(char recvbuf[], int recv_byte_cnt,int recvbuf_len, int* csock);
void blob_store_createBitmap(FILE *fp);
void msg_store_createBitmap(FILE *fp);

void messagestore(char recvbuf[], int recvbuf_len, int *csock);
void signup(FILE *fp, char recvbuf[], int recvbuf_len, int *csock);
int isUserPresent(char *name, FILE *fp);
void signin(FILE *fp, char recvbuf[], int recvbuf_len, int *csock);
void dislaycategories(int userOffset, char buf[], FILE *fp);
void addCategory(int userOffset, FILE *fp, char recvbuf[], int recvbuf_len, int *csock);
void displayAllCategories(int userOffset, FILE *fp, char recvbuf[], int recvbuf_len, int *csock);
void selectCategory(int userOffset, int categoryOffset, FILE *fp, char recvbuf[], int recvbuf_len, int *csock);
int all_categoryOffset(FILE *fp, int categoryID);
int cal_categoryOffset(FILE *fp, int categoryID, int userOffset);
void displayMessages(int categoryOffset, FILE *fp, char buf[]);
void addMessages(FILE *fp, int userOffset, int categoryoffset, char recvbuf[], int recvbuf_len, int *csock);
void updateMessageCategory(FILE *fp, int catoffset, int msgoffset);
void enableBitBlock(FILE *fp, int freeblock);
int getFreeBlock(FILE *fp);
int getMessageOffset(FILE *fp, int messageID, int categoryOffset);
void selectMessage(FILE *fp, int messageOffset, int userOffset, int messageID, int categoryOffset, char recvbuf[], int recvbuf_len, int *csock);
void updateReplyMessage(int messageOffset, FILE *fp, int replyoffset);
void addReply(int messageOffset, FILE *fp, int userOffset, char recvbuf[], int recvbuf_len, int *csock);
void deleteMessage(int categoryOffset, int messageID, int messageOffset, FILE *fp);
void deleteReply(int messageOffset, int categoryOffset, FILE *fp, char recvbuf[], int recvbuf_len, int *csock);

void blobstore(char recvbuf[], int recvbuf_len, int *csock);
void blobsignin(FILE *fp, char recvbuf[], int recvbuf_len, int *csock);
void blobsignup(FILE *fp, char recvbuf[], int recvbuf_len, int *csock);
int blobgetFreeBlock(FILE *fp);
void blobenableBitBlock(FILE *fp, int freeblock);
int fileNameAvailable(FILE *fp, char *name);
void addFiles(FILE *fp,int userOffset,int *csock);
int blobgetfileMeta(FILE *fp);
void displayFiles(FILE *fp, int userOffset, int *csock);
void downloadFile(FILE *fp, int userOffset, int *csock);
int getFileLocation(FILE *fp, char *name);
void deleteFiles(FILE *fp, int userOffset, int *csock);

void calendarstore(char recvbuf[], int recvbuf_len, int *csock);
void viewAllCategories(FILE *fp, char recvbuf[]);
void selectService(FILE *fp,char recvbuf[],int recvbuf_len,int *csock,char username[]);
void registerServiceman(FILE *fp, char recvbuf[], int recvbuf_len, int *csock,int categoryOffset);
void viewServicemen(FILE *fp, char recvbuf[], int recvbuf_len, int *csock, int categoryOffset,char username[]);
void selectServiceman(FILE *fp, char recvbuf[], int recvbuf_len, int *csock, int categoryOffset,char username[]);
int validate(int intDate[]);
int sixMonthsLater(int intDate[]);
void addAppointment(FILE *fp, char recvbuf[], int recvbuf_len, int *csock, int userOffset,char username[]);
void convert(char *charDate, int intDate[]);

void socket_server() {

	//The port you want the server to listen on
	int host_port= 1101;

	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "No sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set options
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		goto FINISH;
	}
	free(p_int);

	//Bind and listen
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = INADDR_ANY ;
	
	/* if you get error in bind 
	make sure nothing else is listening on that port */
	if( bind( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
		fprintf(stderr,"Error binding to socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	if(listen( hsock, 10) == -1 ){
		fprintf(stderr, "Error listening %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	//Now lets do the actual server stuff

	int* csock;
	sockaddr_in sadr;
	int	addr_size = sizeof(SOCKADDR);
	
	while(true){
		printf("waiting for a connection\n");
		csock = (int*)malloc(sizeof(int));
		
		if((*csock = accept( hsock, (SOCKADDR*)&sadr, &addr_size))!= INVALID_SOCKET ){
			//printf("Received connection from %s",inet_ntoa(sadr.sin_addr));
			CreateThread(0,0,&SocketHandler, (void*)csock , 0,0);
		}
		else{
			fprintf(stderr, "Error accepting %d\n",WSAGetLastError());
		}
	}

FINISH:
;
}


void process_input(char *recvbuf, int recv_buf_cnt, int* csock) 
{

	char replybuf[1024]={'\0'};
	printf("%s",recvbuf);
	replyto_client(replybuf, csock);
	replybuf[0] = '\0';
}

void replyto_client(char *buf, int *csock) {
	int bytecount;
	
	if((bytecount = send(*csock, buf,strlen(buf), 0))==SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		//free (csock);
	}
	printf("replied to client: %s\n",buf);
}

void dwnldto_client(char *buf, int *csock)
{
	int bytecount;
	if ((bytecount = send(*csock, buf, 1000, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
	}
	printf("dnwld content : %s\n", buf);
}

DWORD WINAPI SocketHandler(void* lp){
    int *csock = (int*)lp;

	char recvbuf[1024];
	int recvbuf_len = 1024;
	int recv_byte_cnt=0;

	memset(recvbuf, 0, recvbuf_len);
	/*if((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0))==SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		free (csock);
		return 0;
	}
	*/
	communication(recvbuf,0,recvbuf_len,csock);
	//printf("Received bytes %d\nReceived string \"%s\"\n", recv_byte_cnt, recvbuf);
	//process_input(recvbuf, recv_byte_cnt, csock);

    exit(0);
}

void communication(char recvbuf[], int recv_byte_cnt,int recvbuf_len,int* csock)
{
	char buf[1024];
	strcpy(buf,"1.Message Store\n2.Blob Store\n3.Calendar store\n");
	replyto_client(buf, csock);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	/*if (recv_byte_cnt != 1)
	{
		strcpy(buf, "Invalid data entered\n Exiting..");
		replyto_client(buf, csock);
		return;
	}*/
	int choice;
	sscanf(recvbuf, "%d", &choice);
	switch (choice)
	{
	case 1:
		messagestore(recvbuf, recvbuf_len, csock);
		break;
	case 2:
		blobstore(recvbuf, recvbuf_len, csock);
		break;
	case 3:
		calendarstore(recvbuf, recvbuf_len, csock);
	}
}

void msg_store_createBitmap(FILE *fp)
{
	fseek(fp, 0, SEEK_SET);
	char msg;
	memset(&msg,'1', sizeof(char));
	fwrite(&msg, sizeof(char), 1, fp);
	memset(&msg, '0', sizeof(char));
	for (int i = 1; i<TOTALBLOCKS; i++)
	{
		fwrite(&msg, sizeof(char), 1, fp);
	}
	int zero = 0;
	fwrite(&zero, sizeof(int), 1, fp);
}

void blob_store_createBitmap(FILE *fp)
{
	fseek(fp, 0, SEEK_SET);
	char block[1024];
	struct blobuser u;
	strcpy(u.name, "empty");
	u.noOffiles = 0;
	memset(block, '0', 1024);
	for (int i = 0; i<1024; i++)
	{
		fwrite(&block, sizeof(block), 1, fp);
	}
	int temp = ftell(fp);
	for (int i = 0; i<32; i++)
	{
		fwrite(&u, sizeof(user), 1, fp);
	}
	fseek(fp, ENDOF_BLOB_USERS, SEEK_SET);
	int buffer[256];
	for (int i = 0; i<256; i++)
	{
		buffer[i] = 1;
	}
	int no_of_files = 0;
	fwrite(&no_of_files, sizeof(int), 1, fp);
	for (int i = 0; i<4075; i++)
	{
		fwrite(&buffer, sizeof(buffer), 1, fp);
	}
	fseek(fp, 0, SEEK_SET);
	char ch = '1';
	fwrite(&ch, sizeof(char), 1, fp);
}

void messagestore(char recvbuf[], int recvbuf_len, int *csock)
{
	FILE *fp = fopen("data.bin", "rb+");
	if (fp == NULL)
	{
		char ch[50];
		strcpy(ch, "fsutil file createnew data.bin 16777216");
		system(ch);
		fp = fopen("data.bin", "rb+");
	}
	char buf[200];
	char existing_file;
	fread(&existing_file, sizeof(char), 1, fp);
	if (existing_file != '1')
	{
		msg_store_createBitmap(fp);
	}
	fseek(fp, 0, SEEK_SET);
	fread(&existing_file, sizeof(char), 1, fp);
	printf("%c", existing_file);
	while (1)
	{
		strcpy(buf, "\n1.Sign in\n2.Sign up\n3.Exit\nEnter your choice: ");
		replyto_client(buf, csock);
		int choice,recv_byte_cnt;
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
		sscanf(recvbuf, "%d", &choice);
		switch (choice)
		{
		case 1:
			signin(fp, recvbuf, recvbuf_len, csock);
			break;
		case 2:
			signup(fp, recvbuf, recvbuf_len, csock);
			break;
		case 3:
			return;
		}
	}
}

/*void blob(FILE *fp, char recvbuf[], char recvbuf[], int recvbuf_len)
{

}*/
void signup(FILE *fp, char recvbuf[], int recvbuf_len, int *csock)
{
	struct user userdata;
	memset(&userdata, '0', sizeof(userdata));
	char username[20];
	int recv_byte_cnt = 0;
	strcpy(username, "\nEnter username: ");
	replyto_client(username, csock);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	strcpy(userdata.name, recvbuf);
	userdata.filledcategories = 0;
	for (int i = 0; i<CATEGORYPERUSER; i++)
		userdata.category[i] = 0;
	fseek(fp, ENDOFBITMAP, 0);
	int noofusersinfile;
	fread(&noofusersinfile, sizeof(noofusersinfile), 1, fp);
	fseek(fp, ENDOFMETA + sizeof(struct user)*(noofusersinfile), SEEK_SET);
	int temp = ftell(fp);
	fwrite(&userdata, sizeof(struct user), 1, fp);
	fseek(fp, ENDOFBITMAP, 0);
	noofusersinfile += 1;
	fwrite(&noofusersinfile, sizeof(int), 1, fp);
	fseek(fp, (noofusersinfile - 1)*sizeof(int), SEEK_CUR);
	fwrite(&temp, sizeof(int), 1, fp);

}

void signin(FILE *fp, char recvbuf[], int recvbuf_len, int *csock)
{
	int recv_byte_cnt = 0;
	char username[20], buf[1024];
	strcpy(buf, "\nEnter user name:\n");
	replyto_client(buf, csock);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	sscanf(recvbuf, "%s", username);
	int userOffset = isUserPresent(username, fp);
	if (!userOffset)
	{
		strcpy(buf, "\nEnter valid username\nDo you want to continue?(y/n)");
		replyto_client(buf, csock);
		memset(recvbuf, '\0', recvbuf_len);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
		if (!strcmp(recvbuf, "y"))
			return;
		else
			exit(0);
	}
	//printf("%d", userOffset);
	int temp = ftell(fp);
	while (1)
	{
		strcpy(buf, "\nYour categories:\n");
		if (temp != ftell(fp))
			fseek(fp, temp, 0);
		dislaycategories(userOffset, buf, fp);
		strcat(buf, "\n1.View all categories\n2.Add category\n3.Select category\n4.Back\n");
		int choice, categoryID, categoryOffset;
		strcat(buf, "Enter your choice: ");
		replyto_client(buf, csock);
		memset(recvbuf, '\0', recvbuf_len);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
		sscanf(recvbuf,"%d", &choice);
		switch (choice)
		{
		case 1:
			displayAllCategories(userOffset, fp, recvbuf, recvbuf_len, csock);
			break;
		case 2:
			addCategory(userOffset, fp, recvbuf, recvbuf_len, csock);
		break;
		case 3:
			memset(buf, '\0', 1024);
			strcpy(buf,"\nEnter the category id: ");
			replyto_client(buf, csock);
			if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
				//free(csock);
				return;
			}
			sscanf(recvbuf, "%d", &categoryID);
			categoryOffset = cal_categoryOffset(fp, categoryID, userOffset);
			selectCategory(userOffset, categoryOffset, fp,recvbuf,recvbuf_len,csock);
			break;
		case 4:
			return;
		}
	}
}
int isUserPresent(char *name, FILE *fp)
{
		fseek(fp, ENDOFBITMAP, 0);
		struct user userdata;
		int noofusersinfile;
		fread(&noofusersinfile, sizeof(noofusersinfile), 1, fp);
		fseek(fp, ENDOFMETA, 0);
		for (int index = 0; index < noofusersinfile; index++)
		{
			memset(&userdata, 0, sizeof(userdata));
			fread(&userdata, sizeof(userdata), 1, fp);
			if (!strcmp(userdata.name, name))
			{
				//printf("%d\n", ftell(fp));
				fseek(fp, -64, SEEK_CUR);
				int offset;
				//printf("%d\n", ftell(fp));
				offset = ftell(fp);
				return offset;
			}
		}
		return 0;
}

void dislaycategories(int userOffset,char buf[], FILE *fp)
{
	struct user userdata;
	struct category cat;
	fread(&userdata, sizeof(userdata), 1, fp);
	if (userdata.filledcategories == 0)
		strcpy(buf,"No categories added\n");
	else
	{
		for (int index = 0; index < userdata.filledcategories; index++)
		{
			char temp[50];
			int i = 0;
			memset(temp, '\0', 50);
			fseek(fp, userdata.category[index], 0);
			fread(&cat, sizeof(cat), 1, fp);
			temp[i++] = (index + 1) + 48;
			temp[i++] = ':';
			temp[i++] = ':';
			strcat(temp, cat.name);
			strcat(temp, "\n");
			strcat(buf, temp);
		}
	}
}

void displayAllCategories(int userOffset, FILE *fp, char recvbuf[], int recvbuf_len, int *csock)
{
	struct category cat;
	char buf[1024];
	memset(buf, '\0', 1024);
	int noofcategories;
	fseek(fp, ENDOFBITMAP + (17 * sizeof(int)), 0);
	fread(&noofcategories, sizeof(noofcategories), 1, fp);
	strcpy(buf,"\nList of categories\n");
	fseek(fp, ENDOFMETA + (4 * BLOCKSIZE), 0);
	for (int index = 0; index < noofcategories; index++)
	{
		char temp[50];
		memset(temp, '\0', 50);
		int i = 0;
		memset(&cat, 0, sizeof(struct category));
		fread(&cat, sizeof(struct category), 1, fp);
		temp[i++] = (index + 1) + 48;
		temp[i++] = ':';
		temp[i++] = ':';
		strcat(temp,cat.name);
		strcat(temp, "\n");
		strcat(buf, temp);
	}
	strcat(buf,"1.Add category\n2.Select Category\n3.Back\n");
	strcat(buf,"Enter your choice\n");
	replyto_client(buf, csock);
	int choice, recv_byte_cnt;
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	sscanf(recvbuf, "%d", &choice);
	int categoryID, categoryOffset;
	switch(choice)
	{
	case 1:
		addCategory(userOffset, fp,recvbuf,recvbuf_len,csock);
		break;
	case 2:
		memset(buf, '\0', 1024);
		strcpy(buf,"Enter the category ID: ");
		replyto_client(buf, csock);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
		sscanf(recvbuf,"%d", &categoryID);
		categoryOffset = all_categoryOffset(fp, categoryID);
		selectCategory(userOffset, categoryOffset, fp,recvbuf,recvbuf_len,csock);
		break;
	case 3:
		return;
	}
}

void addCategory(int userOffset, FILE *fp, char recvbuf[], int recvbuf_len, int *csock)
{
	struct user userdata;
	struct category cat;
	char buf[1024];
	//strcpy(buf, "No of existing categories:: ");
	memset(&cat, 0, sizeof(cat));
	fseek(fp, ENDOFBITMAP + (17 * sizeof(int)), 0);
	//printf("%d\n", ftell(fp));
	int noofcategories;
	fread(&noofcategories, sizeof(noofcategories), 1, fp);
	strcpy(buf,"\nEnter the name of category\n");
	replyto_client(buf, csock);
	int recv_byte_cnt;
	memset(recvbuf, '\0', recvbuf_len);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	strcpy(cat.name, recvbuf);
	fseek(fp, ENDOFMETA + (4 * BLOCKSIZE) + (128 * noofcategories), 0);
	//printf("%d\n", ftell(fp));
	int offset = ftell(fp);
	fwrite(&cat, sizeof(cat), 1, fp);
	//printf("%d\n", ftell(fp));
	fseek(fp, userOffset, 0);
	fread(&userdata, sizeof(struct user), 1, fp);
	if (userdata.filledcategories == 10)
		printf("Categories overflow\n");
	else
	{
		userdata.filledcategories = userdata.filledcategories + 1;
		userdata.category[userdata.filledcategories - 1] = offset;
		//printf("%d\n", ftell(fp));
		fseek(fp, -64, SEEK_CUR);
		fwrite(&userdata, sizeof(struct user), 1, fp);
		//printf("%d\n", ftell(fp));
		fseek(fp, ENDOFBITMAP + (17 * sizeof(int)), 0);
		//printf("%d\n", ftell(fp));
		fread(&noofcategories, sizeof(noofcategories), 1, fp);
		noofcategories = noofcategories + 1;
		fseek(fp, ENDOFBITMAP + (17 * sizeof(int)), 0);
		//printf("%d", ftell(fp));
		fwrite(&noofcategories, sizeof(int), 1, fp);
		//printf("%d", ftell(fp));
	}
}

int cal_categoryOffset(FILE *fp, int categoryID, int userOffset)
{
	if (categoryID < 1)
	{
		printf("\nInvalid id\n");
		return -1;
	}
	fseek(fp, ENDOFBITMAP + 17 * (sizeof(int)), 0);
	int noofcategories;
	fread(&noofcategories, sizeof(noofcategories), 1, fp);
	if (categoryID > noofcategories)
	{
		printf("Invalid id\n");
		return -1;
	}
	categoryID--;
	fseek(fp, userOffset, 0);
	struct user userdata;
	fread(&userdata, sizeof(userdata), 1, fp);
	int categoryOffset = userdata.category[categoryID];
	//printf("\n%d", categoryOffset);
	return categoryOffset;
}

int all_categoryOffset(FILE *fp, int categoryID)
{
	if (categoryID < 1)
	{
		printf("\nInvalid id\n");
		return -1;
	}
	fseek(fp, ENDOFBITMAP + 17 * (sizeof(int)), 0);
	int noofcategories;
	fread(&noofcategories, sizeof(noofcategories), 1, fp);
	if (categoryID > noofcategories)
	{
		printf("\nInvalid id\n");
		return -1;
	}
	categoryID--;
	fseek(fp, ENDOFMETA + (4 * BLOCKSIZE) + categoryID * 128, 0);
	int categoryOffset = ftell(fp);
	//printf("\n%d", categoryOffset);
	return categoryOffset;
}

void selectCategory(int userOffset, int categoryOffset, FILE *fp, char recvbuf[], int recvbuf_len, int *csock)
{
	char buf[1024];
	memset(buf, '\0', 1024);
	displayMessages(categoryOffset, fp,buf);
	strcat(buf,"\n1.Add Message\n2.Select Message\n3.Back\n");
	int choice, messageID, messageOffset,recv_byte_cnt;
	strcat(buf,"Enter your choice: ");
	replyto_client(buf, csock);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	sscanf(recvbuf, "%d", &choice);
	switch (choice)
	{
	case 1:
		addMessages(fp, userOffset, categoryOffset ,recvbuf,recvbuf_len,csock);
		break;
	case 2:
		memset(buf, '\0', 1024);
		strcpy(buf,"\nEnter the message id:\n");
		replyto_client(buf, csock);
		memset(recvbuf, '\0', 1024);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
		sscanf(recvbuf,"%d", &messageID);
		messageOffset = getMessageOffset(fp, messageID, categoryOffset);
		selectMessage(fp, messageOffset, userOffset, messageID, categoryOffset,recvbuf,recvbuf_len,csock);
		break;
	case 3:
		return;
	}
}

void displayMessages(int categoryOffset, FILE *fp,char buf[])
{
	fseek(fp, categoryOffset, 0);
	struct category cat;
	fread(&cat, sizeof(category), 1, fp);
	if (cat.filledMessages == 0)
	{
		strcpy(buf, "\nNo messages added yet\n");
		return;
	}
	else
	{
		strcpy(buf, "\nThe messages in this category\n");
		int temp = 0;
		for (int i = 0, temp = 0; temp<cat.filledMessages; i++)
		{
			if (cat.messageOffsets[i] == 0)
				continue;
			char m[192];
			memset(m, '\0', 192);
			int index = 0;
			fseek(fp, cat.messageOffsets[i], 0);
			struct message msg;
			fread(&msg, sizeof(struct message), 1, fp);
			m[index++] = '\n';
			m[index++] = (i + 1) + 48;
			m[index++] = ':';
			m[index++] = ':';
			strcat(m, msg.text);
			strcat(buf, m);
			temp++;
		}
		strcat(buf, "\n");
	}
}

void addMessages(FILE *fp, int userOffset, int categoryoffset,char recvbuf[],int recvbuf_len,int *csock)
{
	char buf[1024];
	memset(buf, '\0', 1024);
	fseek(fp, categoryoffset, 0);
	struct category cat;
	memset(&cat, 0, sizeof(cat));
	fread(&cat, sizeof(cat), 1, fp);
	int freeblock = getFreeBlock(fp);
	if (freeblock == -1)
	{
		strcpy(buf,"\nMemory full\n");
		replyto_client(buf, csock);
		return;
	}

	fseek(fp, freeblock*BLOCKSIZE, SEEK_SET);
	strcpy(buf,"Enter message:\n");
	replyto_client(buf, csock);
	struct message newmsg;
	memset(&newmsg, 0, sizeof(newmsg));
	//fflush(stdin);
	//gets(newmsg.text);
	int recv_byte_cnt = 0;
	memset(recvbuf, '\0', 1024);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	strcpy(newmsg.text, recvbuf);
	newmsg.userOffset = userOffset;
	newmsg.replyOffset = 0;

	int messageoffset = ftell(fp);
	fwrite(&newmsg, sizeof(struct message), 1, fp);
	enableBitBlock(fp, freeblock);
	updateMessageCategory(fp, categoryoffset, messageoffset);
}

void updateMessageCategory(FILE *fp, int catoffset, int msgoffset)
{
	fseek(fp, catoffset, SEEK_SET);
	struct category cat;
	fread(&cat, sizeof(struct category), 1, fp);
	cat.filledMessages = cat.filledMessages + 1;
	for (int i = 0; i<25; i++)
	{
		if (cat.messageOffsets[i] == 0)
		{
			cat.messageOffsets[i] = msgoffset;
			break;
		}
	}
	fseek(fp, catoffset, SEEK_SET);
	fwrite(&cat, sizeof(struct category), 1, fp);
}
void enableBitBlock(FILE *fp, int freeblock)
{
	fseek(fp, freeblock, SEEK_SET);
	fputc('1', fp);
}

int getFreeBlock(FILE *fp)
{
	fseek(fp, NOOFRESERVEDBLOCKS, SEEK_SET);
	for (int i = 0; i<(BITMAPBLOCKS - 1)*BLOCKSIZE; i++)
	{
		char ch = fgetc(fp);
		if (ch == '0')
		{
			ungetc(ch, fp);
			return ftell(fp);
		}
	}
	return -1;
}

int getMessageOffset(FILE *fp, int messageID, int categoryOffset)
{
	if (messageID < 1)
	{
		printf("Invalid message id\n");
		return -1;
	}
	fseek(fp, categoryOffset, 0);
	messageID--;
	struct category cat;
	fread(&cat, sizeof(struct category), 1, fp);
	int messageOffset = cat.messageOffsets[messageID];
	return messageOffset;
}

void selectMessage(FILE *fp, int messageOffset, int userOffset, int messageID, int categoryOffset,char recvbuf[],int recvbuf_len,int *csock)
{
	char buf[1024];
	memset(buf, '\0', 1024);
	fseek(fp, messageOffset, 0);
	struct message msg;
	fread(&msg, sizeof(struct message), 1, fp);
	if (msg.replyOffset == 0)
		strcpy(buf,"No replies yet\n");
	int index = 0;
	while (msg.replyOffset != 0)
	{
		char temp[200];
		int i= 0;
		memset(temp, '\0', 200);
		fseek(fp, msg.replyOffset, 0);
		fread(&msg, sizeof(struct message), 1, fp);
		temp[i++] = '\n';
		temp[i++] = (index + 1) + 48;
		temp[i++] = ')';
		temp[i++] = ' ';
		strcat(temp, msg.text);
		strcat(buf, temp);
		index++;
	}
	strcat(buf,"\n1.Add Reply\n2.Delete message\n3.Delete Reply\n4.Back\n\n");
	int choice,recv_byte_cnt;
	strcat(buf,"Enter the choice\n");
	replyto_client(buf, csock);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	sscanf(recvbuf, "%d", &choice);
	switch (choice)
	{
	case 1:
		addReply(messageOffset, fp, userOffset,recvbuf,recvbuf_len,csock);
		break;
	case 2:
		deleteMessage(categoryOffset, messageID, messageOffset, fp);
		break;
	case 3:
		deleteReply(messageOffset, categoryOffset, fp,recvbuf,recvbuf_len,csock);
		break;
	case 4:
		return;

	}
}

void updateReplyMessage(int messageOffset, FILE *fp, int replyoffset)
{
	fseek(fp, messageOffset, SEEK_SET);
	struct message msg;
	memset(&msg, 0, sizeof(struct message));
	int temp = ftell(fp);
	fread(&msg, sizeof(message), 1, fp);
	while (msg.replyOffset != 0)
	{
		temp = msg.replyOffset;
		fseek(fp, msg.replyOffset, SEEK_SET);
		fread(&msg, sizeof(message), 1, fp);
	}
	msg.replyOffset = replyoffset;
	fseek(fp, temp, SEEK_SET);
	fwrite(&msg, sizeof(message), 1, fp);
}

void addReply(int messageOffset, FILE *fp, int userOffset,char recvbuf[],int recvbuf_len,int *csock)
{
	char buf[1024];
	memset(buf, '\0', 1024);
	strcpy(buf,"Enter the reply\n");
	replyto_client(buf, csock);
	int recv_byte_cnt;
	memset(recvbuf, '\0', 1024);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	struct message msg;
	memset(&msg, 0, sizeof(struct message));
	int freeblock = getFreeBlock(fp);
	if (freeblock == -1)
	{
		memset(buf, '\0', 1024);
		strcpy(buf,"\nMemory full\n");
		replyto_client(buf, csock);
		return;
	}

	fseek(fp, freeblock*BLOCKSIZE, SEEK_SET);
	struct message newreply;
	memset(&newreply, 0, sizeof(newreply));
	//fflush(stdin);
	//gets(newreply.text);
	strcpy(newreply.text, recvbuf);
	newreply.userOffset = userOffset;
	newreply.replyOffset = 0;
	int replyOffset = ftell(fp);
	fwrite(&newreply, sizeof(struct message), 1, fp);
	enableBitBlock(fp, freeblock);
	updateReplyMessage(messageOffset, fp, replyOffset);
}

void deleteMessage(int categoryOffset, int messageID, int messageOffset, FILE *fp)
{
	int replyBitLocation;
	int messagelocation = messageOffset / BLOCKSIZE;
	fseek(fp, messagelocation, 0);
	fputc('0', fp);
	fseek(fp, messageOffset, SEEK_SET);
	struct message msg;
	fread(&msg, sizeof(message), 1, fp);
	while (msg.replyOffset)
	{
		replyBitLocation = msg.replyOffset / BLOCKSIZE;
		fseek(fp, replyBitLocation, 0);
		fputc('0', fp);
		fseek(fp, msg.replyOffset, SEEK_SET);
		fread(&msg, sizeof(message), 1, fp);
	}
	struct category cat;
	fseek(fp, categoryOffset, 0);
	fread(&cat, sizeof(struct category), 1, fp);
	cat.filledMessages = cat.filledMessages - 1;
	messageID--;
	replyBitLocation = msg.replyOffset / BLOCKSIZE;
	//enableBitBlock(fp,bitlocation);
	cat.messageOffsets[messageID] = 0;
	fseek(fp, categoryOffset, 0);
	fwrite(&cat, sizeof(struct category), 1, fp);
}

void deleteReply(int messageOffset, int categoryOffset, FILE *fp, char recvbuf[], int recvbuf_len, int *csock)
{
	int replyID,recv_byte_cnt;
	char buf[1024];
	memset(buf, '\0', 1024);
	struct message msg;
	fseek(fp, messageOffset, 0);
	fread(&msg, sizeof(struct message), 1, fp);
	if (msg.replyOffset == 0)
	{
		strcpy(buf,"No replies to delete\n");
		replyto_client(buf, csock);
		return;
	}
	strcpy(buf,"Enter the reply ID: ");
	replyto_client(buf, csock);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	sscanf(recvbuf,"%d", &replyID);
	if (replyID < 0)
	{
		printf("Invalid rely ID\n");
		return;
	}
	int prev = messageOffset;
	int current = msg.replyOffset;
	fseek(fp, msg.replyOffset, 0);
	fread(&msg, sizeof(struct message), 1, fp);
	int countreplies = 1, next = 0;
	while (countreplies<replyID && msg.replyOffset != 0)
	{
		prev = current;
		current = msg.replyOffset;
		fseek(fp, msg.replyOffset, 0);
		fread(&msg, sizeof(struct message), 1, fp);
		countreplies++;
	}
	if (countreplies != replyID)
	{
		memset(buf, '\0', 1024);
		strcpy(buf,"Selected reply Id doesnt exist\n");
		return;
	}
	next = msg.replyOffset;
	int replyLocation = current / BLOCKSIZE;
	fseek(fp, replyLocation, 0);
	fputc('0', fp);
	fseek(fp, prev, 0);
	fread(&msg, sizeof(struct message), 1, fp);
	msg.replyOffset = next;
	fseek(fp, prev, 0);
	fwrite(&msg, sizeof(struct message), 1, fp);
}

void blobstore(char recvbuf[], int recvbuf_len, int *csock)
{
	FILE *fp = fopen("blob.bin", "rb+");
	if (fp == NULL)
	{
		char ch[50];
		strcpy(ch, "fsutil file createnew blob.bin 1073741824");
		system(ch);
		fp = fopen("blob.bin", "rb+");
	}
	char buf[200];
	char existing_file;
	fread(&existing_file, sizeof(char), 1, fp);
	if (existing_file != '1')
		blob_store_createBitmap(fp);
	while (1)
	{
		memset(buf, '\0', 200);
		strcpy(buf, "\n1.Sign up\n2.Sign in\n3.Exit\nEnter your choice: ");
		replyto_client(buf, csock);
		int choice, recv_byte_cnt;
		memset(recvbuf, '\0', recvbuf_len);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
		sscanf(recvbuf, "%d", &choice);
		switch (choice)
		{
		case 1:
			blobsignup(fp, recvbuf, recvbuf_len, csock);
			break;
		case 2:
			blobsignin(fp, recvbuf, recvbuf_len, csock);
			break;
		case 3:
			return;
		}
	}
}

void blobsignup(FILE *fp, char recvbuf[], int recvbuf_len, int *csock)
{
	struct blobuser userdata;
	memset(&userdata, 0, sizeof(userdata));
	char username[50];
	int recv_byte_cnt = 0;
	strcpy(username, "\nEnter username: ");
	replyto_client(username, csock);
	memset(recvbuf, '\0', recvbuf_len);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	int flag = 0;
	fseek(fp, ENDOF_BLOB_BITMAP, 0);
	int temp=ftell(fp);
	fread(&userdata, sizeof(userdata), 1, fp);
	while (strcmp(userdata.name , "empty"))
	{
		if (!strcmp(userdata.name, recvbuf))
		{
			flag = 1;
			break;
		}
		fread(&userdata, sizeof(userdata), 1, fp);
		
	}
	if (flag == 1)
	{
		memset(username, '\0', 20);
		strcpy(username, "\nUser name already exists\n");
		replyto_client(username, csock);
		return;
	}
	int temp1 = ftell(fp);
	memset(userdata.name, '\0', 28);
	strcpy(userdata.name, recvbuf);
	userdata.noOffiles = 0;
	fseek(fp,-sizeof(struct blobuser), SEEK_CUR);
	int temp3 = ftell(fp);
	fwrite(&userdata, sizeof(struct blobuser), 1, fp);
}

void blobsignin(FILE *fp, char recvbuf[], int recvbuf_len, int *csock)
{
		int choice;
		struct blobuser userdata;
		memset(&userdata, 0, sizeof(userdata));
		char buf[1024];
		int recv_byte_cnt = 0;
		memset(buf, '\0', 1024);
		strcpy(buf, "\nEnter username: ");
		replyto_client(buf, csock);
		memset(recvbuf, '\0', 1024);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
		fseek(fp, ENDOF_BLOB_BITMAP, 0);
		int count = 0;
		fread(&userdata, sizeof(userdata), 1, fp);
		while (strcmp(userdata.name, recvbuf))
		{
			if (!strcmp(userdata.name, "empty"))
			{
				count = 1;
				break;
			}
			fread(&userdata, sizeof(userdata), 1, fp);
		}
		if (count)
		{
			strcpy(buf, "\nEnter a valid username\n");
			replyto_client(buf, csock);
			return;
		}
		fseek(fp, -sizeof(struct blobuser), SEEK_CUR);
		int userOffset = ftell(fp);
		while (1)
		{
			choice = -1;
			memset(buf, '\0',1024);
			strcpy(buf, "\n1.Add files\n2.Display files\n3.Download files\n4.Delete files\n5.Back\n");
			strcat(buf, "Enter your choice\n");
			replyto_client(buf, csock);
			char choose[3];
			memset(buf, '\0', 1024);
			memset(choose, '\0', 3);
			if ((recv_byte_cnt = recv(*csock,choose,3, 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
				//free(csock);
				return;
			}
			sscanf(choose, "%d", &choice);
			switch (choice)
			{
			case 1:
				addFiles(fp, userOffset, csock);
				break;
			case 2:
				displayFiles(fp, userOffset, csock);
				break;
			case 3:
				downloadFile(fp, userOffset, csock);
				break;
			case 4:
				deleteFiles(fp, userOffset, csock);
				break;
			case 5:
				return;
			}
		}
}

int blobgetFreeBlock(FILE *fp)
{
	fseek(fp,BLOBRESERVED, SEEK_SET);
	int location = ftell(fp);
	for (int i = 0; i<(BLOB_BLOCKS - 1)*BLOB_BLOCK_SIZE; i++)
	{
		char ch = fgetc(fp);
		if (ch == '0')
		{
			ungetc(ch, fp);
			return ftell(fp);
		}
	}
	return -1;
}
void blobenableBitBlock(FILE *fp, int freeblock)
{
	int location = ftell(fp);
	fseek(fp, freeblock, SEEK_SET);
	fputc('1', fp);
	fseek(fp, location, SEEK_SET);
}

int fileNameAvailable(FILE *fp, char *name)
{
	fseek(fp, ENDOF_BLOB_USERS, SEEK_SET);
	int location = ftell(fp);
	int no_of_files;
	fread(&no_of_files, sizeof(int), 1, fp);
	int value;
	for (int i = 0; i < no_of_files; i++)
	{
		fread(&value, sizeof(int), 1, fp);
		location = ftell(fp);
		fseek(fp, value, SEEK_SET);
		struct blobfile file;
		memset(&file, 0, sizeof(struct blobfile));
		fread(&file, sizeof(struct blobfile), 1, fp);
		if (!strcmp(file.title, name))
		{
			int check = ftell(fp);
			return ftell(fp)-1024;
		}
		fseek(fp, location, SEEK_SET);
	}
	return 1;
}

int blobgetfileMeta(FILE *fp)
{
	fseek(fp, ENDOF_BLOB_USERS, SEEK_SET);
	int value;
	int no_of_files;
	fread(&no_of_files, sizeof(int), 1, fp);
	while(ftell(fp)<5223424)
	{
		fread(&value, sizeof(int), 1, fp);
		if (value == 1)
			return ftell(fp)-4;
	}
	return -1;
}

void addFiles(FILE *fp,int userOffset,int *csock)
{
	char recvbuf[32];
	int length_of_file = 0, recv_byte_cnt = 0, prev=0;
	char filename[16];
	int recvbuf_len = 32;
	memset(recvbuf, '\0', 32);
	strcpy(recvbuf, "Enter the filename:");
	replyto_client(recvbuf, csock);
	memset(filename, '\0', 16);
	if ((recv_byte_cnt = recv(*csock, filename, 16, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	filename[recv_byte_cnt] = '\0';
	int validname = fileNameAvailable(fp, filename);
	if (validname!=1)
		return;
	int startOffset = blobgetfileMeta(fp);
	strcpy(recvbuf, "send the len");
	replyto_client(recvbuf, csock);
	memset(recvbuf, '\0', 32);
	if ((recv_byte_cnt = recv(*csock, recvbuf, 32, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	recvbuf[recv_byte_cnt] = '\0';
	sscanf(recvbuf, "%d", &length_of_file);
	memset(recvbuf, '\0', 32);
	strcpy(recvbuf, "send");
	replyto_client(recvbuf, csock);
	int no_of_blocks = (length_of_file / 1000)+1;
	//if (length_of_file % 1000 != 0)
		//no_of_blocks += 1;
	char *recvdata = (char *)malloc((length_of_file+4)*sizeof(char));
	memset(recvdata, '\0', length_of_file);
	if ((recv_byte_cnt = recv(*csock, recvdata,length_of_file, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	recvdata[recv_byte_cnt] = '\0';
	fseek(fp, ENDOF_BLOB_USERS, SEEK_SET);
	int no_of_files;
	fread(&no_of_files, sizeof(int), 1, fp);
	struct blobfile file;
	for (int i = 0; i < no_of_blocks; i++)
	{
		//memset(recvdata, '\0', length_of_file + 2);
		memset(&file, 0, sizeof(struct blobfile));
		int freeblock = blobgetFreeBlock(fp);
		if (freeblock == -1)
		{
			printf("Memory not available\n");
			exit(0);
		}
		int location = ftell(fp);
		fseek(fp, freeblock*BLOB_BLOCK_SIZE, SEEK_SET);
		location = ftell(fp);
		int temp = ftell(fp);
		if (i == 0)
		{
			fseek(fp, startOffset, SEEK_SET);
			location = ftell(fp);
			int offset = temp;
			fwrite(&offset, sizeof(int), 1, fp); 
			location = ftell(fp);
			fseek(fp, temp, SEEK_SET);
			location = ftell(fp);
		}
		else
		{
			location = ftell(fp);
			fseek(fp, prev, SEEK_SET);
			location = ftell(fp);
			fread(&file, sizeof(struct blobfile), 1, fp);
			file.nextPart = temp;
			location = ftell(fp);
			fseek(fp, -1024, SEEK_CUR);
			location = ftell(fp);
			fwrite(&file, sizeof(struct blobfile), 1, fp);
			memset(&file, 0, sizeof(struct blobfile));
			location = ftell(fp);
			fseek(fp, temp, SEEK_SET);
			location = ftell(fp);
		}
		blobenableBitBlock(fp, freeblock);
		strcpy(file.title, filename);
		file.userOffset = userOffset;
		for (int j = (i * 1000),index=0; j < (i + 1) * 1000 && j < length_of_file; j++,index++)
		{
			file.data[index] = recvdata[j];
		}
		location = ftell(fp); 
		fwrite(&file, sizeof(struct blobfile), 1, fp);
		fseek(fp, -1024, SEEK_CUR);
		location = ftell(fp);
		prev = ftell(fp);
	}
	fseek(fp, ENDOF_BLOB_USERS, SEEK_SET);
	no_of_files += 1;
	fwrite(&no_of_files, sizeof(int), 1, fp);
	char done[15];
	memset(done, '\0', 15);
	strcpy(done, "reading done");
	replyto_client(done, csock); 
	memset(recvbuf, '\0', recvbuf_len);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
}

void displayFiles(FILE *fp, int userOffset, int *csock)
{
	fseek(fp, ENDOF_BLOB_USERS, SEEK_SET);
	int no_of_files,recv_byte_cnt;
	char buffer[1024];
	memset(buffer, '\0', 1024);
	strcpy(buffer, "Available Files are\n");
	fread(&no_of_files, sizeof(int), 1, fp);
	for (int index = 0; index < no_of_files; index++)
	{
		int value;
		fread(&value, sizeof(int), 1, fp);
		int location = ftell(fp);
		if (value == 1)
			continue;
		fseek(fp, value, SEEK_SET);
		struct blobfile file;
		memset(&file,0,sizeof(struct blobfile));
		fread(&file, sizeof(struct blobfile), 1, fp);
		if (file.userOffset == userOffset)
		{
			char temp[3];
			memset(temp, '\0', 3);
			sprintf(temp, "%d", index + 1);
			strcat(buffer, temp);
			strcat(buffer, file.title);
			strcat(buffer, "\n");
		}
		fseek(fp, location, SEEK_SET);
	}
	replyto_client(buffer, csock);
	memset(buffer, '\0', 1024);
	int buffer_len = 1024;
	if ((recv_byte_cnt = recv(*csock,buffer,buffer_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
}

void downloadFile(FILE *fp, int userOffset, int *csock)
{
	char recvbuf[1024];
	int recvbuf_len = 1024,recv_byte_cnt;
	memset(recvbuf, '\0', recvbuf_len);
	strcpy(recvbuf, "Enter the filename:");
	replyto_client(recvbuf, csock);
	memset(recvbuf, '\0', recvbuf_len);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	recvbuf[recv_byte_cnt] = '\0';
	struct blobfile file;
	memset(&file, 0, sizeof(struct blobfile));
	int location_of_file = fileNameAvailable(fp, recvbuf);
	fseek(fp, location_of_file, SEEK_SET);
	fread(&file, sizeof(struct blobfile), 1, fp);
	while (file.nextPart != 0)
	{
		//memset(recvbuf, '\0', recvbuf_len);
		//strcpy(recvbuf, file.data);
		//recvbuf[1000] = '\0';
		dwnldto_client(file.data,csock);
		fseek(fp, file.nextPart, SEEK_SET);
		memset(&file,0,sizeof(struct blobfile));
		fread(&file, sizeof(struct blobfile), 1, fp);
		memset(recvbuf, '\0', recvbuf_len);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
	}
	memset(recvbuf,'\0', recvbuf_len);
	//strcpy(recvbuf, file.data);
	dwnldto_client(file.data, csock);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		free(csock);
		return;
	}
	memset(recvbuf, '\0', recvbuf_len);
	strcpy(recvbuf, "file has been sent successfully");
	replyto_client(recvbuf, csock);
	memset(recvbuf, '\0', recvbuf_len);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
}

int getFileLocation(FILE *fp, char *name)
{
	fseek(fp, ENDOF_BLOB_USERS, SEEK_SET);
	int no_of_files,number=0,offset=0;
	fread(&no_of_files, sizeof(int), 1, fp);
	for (int index = 0; index < no_of_files; index++)
	{
		int value;
		fread(&value, sizeof(int), 1, fp);
		number++;
		if (value == 1)
			continue;
		int location = ftell(fp);
		fseek(fp, value, SEEK_SET);
		struct blobfile file;
		memset(&file, 0, sizeof(struct blobfile));
		fread(&file, sizeof(struct blobfile), 1, fp);
		if (!strcmp(file.title, name))
		{
			offset = ftell(fp) - 1024;
			fseek(fp, location - 4, SEEK_SET);
			int available = 1;
			fwrite(&available, sizeof(int), 1, fp);
			break;
		}
		fseek(fp, location, SEEK_SET);
	}
	if (number == no_of_files)
	{
		no_of_files = no_of_files - 1;
		fseek(fp, ENDOF_BLOB_USERS, SEEK_SET);
		fwrite(&no_of_files, sizeof(int), 1, fp);
	}
	return offset;
}

void deleteFiles(FILE *fp, int userOffset, int *csock)
{
	char recvbuf[1024];
	int recvbuf_len = 1024, recv_byte_cnt;
	memset(recvbuf, '\0', recvbuf_len);
	strcpy(recvbuf, "Enter the filename:");
	replyto_client(recvbuf, csock);
	memset(recvbuf, '\0', recvbuf_len);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	recvbuf[recv_byte_cnt] = '\0';
	struct blobfile file;
	memset(&file, 0, sizeof(struct blobfile));
	int location_of_file = getFileLocation(fp, recvbuf);
	fseek(fp, location_of_file, SEEK_SET);
	fread(&file, sizeof(struct blobfile), 1, fp);
	while (file.nextPart != 0)
	{
		int next = file.nextPart;
		int freeblock = location_of_file / 1024;
		fseek(fp, freeblock, SEEK_SET);
		char ch = '0';
		fwrite(&ch, sizeof(char), 1, fp);
		fseek(fp, next, SEEK_SET);
		fread(&file, sizeof(struct blobfile), 1, fp);
		location_of_file = ftell(fp) - 1024;
	}
	int freeblock = location_of_file / 1024;
	fseek(fp, freeblock, SEEK_SET);
	char ch = '0';
	fwrite(&ch, sizeof(char), 1, fp);
	memset(recvbuf, '\0', recvbuf_len);
	strcpy(recvbuf, "Deletion Successful\n");
	replyto_client(recvbuf, csock); memset(recvbuf, '\0', recvbuf_len);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
}

void calendarstore(char recvbuf[], int recvbuf_len, int *csock)
{
	FILE *fp = fopen("calendar.bin", "rb+");
	if (fp == NULL)
	{
		char ch[50];
		strcpy(ch, "fsutil file createnew calendar.bin 104857600");
		system(ch);
		fp = fopen("calendar.bin", "rb+");
		char services[10][12]={"Doctors","Engineers","Technicians","Tutors","Writers","Coaches","Players","Merchants","FreeLancers","Artists"};
		struct calendarCategory cat;
		for(int i=0;i<10;i++)
		{
			memset(&cat, 0, sizeof(struct calendarCategory));
			strcpy(cat.name,services[i]);
			fwrite(&cat, sizeof(struct calendarCategory), 1, fp);
		}
		int n=0;
		fwrite(&n,sizeof(int),1,fp);
	}
	memset(recvbuf, '\0', recvbuf_len);
	char username[16];
	strcpy(recvbuf, "Enter the username:\n");
	replyto_client(recvbuf, csock);
	int recv_byte_cnt = 0;
	memset(username, '\0', 16);
	if ((recv_byte_cnt = recv(*csock, username, 16, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	while (1)
	{
		fseek(fp, 0, SEEK_SET);
		memset(recvbuf, '\0', recvbuf_len);
		strcpy(recvbuf, "Categories:\n");
		viewAllCategories(fp, recvbuf);
		strcat(recvbuf, "\n\nOptions\n\n1.Select a category\n2.Exit");
		replyto_client(recvbuf, csock);
		char ch[3];
		memset(ch, '\0', 3);
		if ((recv_byte_cnt = recv(*csock, ch, 3, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
		int choice;
		sscanf(ch, "%d", &choice);
		switch (choice)
		{
		case 1:
			selectService(fp, recvbuf, recvbuf_len, csock,username);
			break;
		case 2:
			return;
		}
	}
}

void viewAllCategories(FILE *fp,char recvbuf[])
{
	struct calendarCategory cat;
	for (int i = 0; i < 10; i++)
	{
		char temp[4];
		memset(temp, '\0', 4);
		memset(&cat, 0, sizeof(calendarCategory));
		fread(&cat, sizeof(calendarCategory), 1, fp);
		sprintf(temp, "%d", i+1);
		strcat(recvbuf, temp);
		strcat(recvbuf, ":");
		strcat(recvbuf, cat.name);
		strcat(recvbuf, "\n");
	}
}

void selectService(FILE *fp, char recvbuf[], int recvbuf_len, int *csock,char username[])
{
	memset(recvbuf, '\0', 1024);
	strcpy(recvbuf, "Enter the categoryID\n");
	replyto_client(recvbuf, csock);
	memset(recvbuf, '\0', 1024);
	int recv_byte_cnt = 0;
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	int categoryID;
	sscanf(recvbuf, "%d", &categoryID);
	if (categoryID > 10 || categoryID<1)
	{
		memset(recvbuf, '\0', 1024);
		strcpy(recvbuf, "Entered category ID is invlaid");
		strcat(recvbuf, "Do you want to continue?(y/n)\n");
		replyto_client(recvbuf, csock);
		memset(recvbuf, '\0', 1024);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
		if (!strcmp(recvbuf, "y"))
			return;
		else
			exit(0);
	}
	int categoryOffset;
	fseek(fp, 0, SEEK_SET);
	struct calendarCategory cat;
	memset(&cat, 0, sizeof(cat));
	for (int i = 0; i < categoryID - 1; i++)
	{
		fread(&cat, sizeof(calendarCategory), 1, fp);
	}
	categoryOffset = ftell(fp);
	while (1)
	{
		memset(recvbuf, '\0', 1024);
		strcpy(recvbuf, "\nOptions\n\n1.Register a service person\n2.View registered servicemen\n3.Back\n");
		strcat(recvbuf, "Enter your choice");
		replyto_client(recvbuf, csock);
		memset(recvbuf, '\0', 1024);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
		int choice;
		sscanf(recvbuf, "%d", &choice);
		switch (choice)
		{
		case 1:
			registerServiceman(fp, recvbuf, recvbuf_len, csock, categoryOffset);
			break;
		case 2:
			viewServicemen(fp, recvbuf, recvbuf_len, csock, categoryOffset,username);
			break;
		case 3:
			return;
		}
	}
}

void registerServiceman(FILE *fp, char recvbuf[], int recvbuf_len, int *csock, int categoryOffset)
{
	memset(recvbuf, '\0', 1024);
	fseek(fp, categoryOffset, SEEK_SET);
	struct calendarCategory cat;
	struct serviceman man,new_man;
	int no_of_servicemen,prev=categoryOffset;
	memset(&cat, 0, sizeof(struct calendarCategory));
	fread(&cat, sizeof(struct calendarCategory), 1, fp);
	if (cat.servicemanOffset != 0)
	{
		fseek(fp, cat.servicemanOffset, SEEK_SET);
		prev = cat.servicemanOffset;
		memset(&man, 0, sizeof(struct serviceman));
		fread(&man, sizeof(struct serviceman), 1, fp);
		while (man.nextServicemanOffset != 0)
		{
			prev = man.nextServicemanOffset;
			fseek(fp, man.nextServicemanOffset, SEEK_SET);
			fread(&man, sizeof(struct serviceman), 1, fp);
		}
	}
	fseek(fp, end_of_category, SEEK_SET);
	fread(&no_of_servicemen, sizeof(int), 1, fp);
	fseek(fp, no_of_servicemen * 64, SEEK_CUR);
	int location = ftell(fp);
	strcpy(recvbuf, "Enter the name:\n");
	replyto_client(recvbuf, csock);
	int recv_byte_cnt;
	memset(recvbuf, '\0', 1024);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	memset(&new_man, 0, sizeof(struct serviceman));
	strcpy(new_man.name, recvbuf);
	memset(recvbuf, '\0', 1024);
	strcpy(recvbuf, "\nEnter the role: ");
	replyto_client(recvbuf, csock);
	memset(recvbuf, '\0', 1024);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	strcpy(new_man.role, recvbuf);
	memset(recvbuf, '\0', 1024);
	strcpy(recvbuf, "\nEnter the phonenumber: ");
	replyto_client(recvbuf, csock);
	memset(recvbuf, '\0', 1024);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	strcpy(new_man.phone, recvbuf);
	new_man.firstappointmentOffset = 0;
	new_man.nextServicemanOffset = 0;
	fwrite(&new_man, sizeof(struct serviceman), 1, fp);
	fseek(fp, prev, SEEK_SET);
	if (prev == categoryOffset)
	{
		fread(&cat, sizeof(calendarCategory), 1, fp);
		cat.servicemanOffset = location;
		fseek(fp, -16, SEEK_CUR);
		fwrite(&cat, sizeof(struct calendarCategory), 1, fp);
	}
	else
	{
		fread(&man, sizeof(struct serviceman), 1, fp);
		man.nextServicemanOffset = location;
		fseek(fp, -64, SEEK_CUR);
		fwrite(&man, sizeof(struct serviceman), 1, fp);
	}
	location = ftell(fp);
	fseek(fp, end_of_category, SEEK_SET);
	location = ftell(fp);
	no_of_servicemen += 1;
	fwrite(&no_of_servicemen, sizeof(int), 1, fp);
}

void viewServicemen(FILE *fp, char recvbuf[], int recvbuf_len, int *csock, int categoryOffset,char username[])
{
	struct serviceman man;
	struct calendarCategory cat;
	int recv_byte_cnt;
	memset(&man, 0, sizeof(struct serviceman));
	memset(&cat, 0, sizeof(struct calendarCategory));
	memset(recvbuf, '\0', 1024);
	fseek(fp, categoryOffset, SEEK_SET);
	fread(&cat, sizeof(calendarCategory), 1, fp);
	if (!cat.servicemanOffset)
	{
		    strcpy(recvbuf, "\nNo servicemen are added yet in this category\n\n");
			strcat(recvbuf, "Register the service men to view\n\nDo you want to continue(y/n)?");
			replyto_client(recvbuf, csock);
			memset(recvbuf, '\0', 1024);
			if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
				//free(csock);
				return;
			}
			if (!strcmp(recvbuf, "y"))
				return;
			else
				exit(0);
	}
	else
	{
		int sno = 1;
		char temp[4];
		fseek(fp, cat.servicemanOffset, SEEK_SET);
		fread(&man, sizeof(struct serviceman), 1, fp);
		strcpy(recvbuf, "\n\nName \t\t Role\n");
		while (man.nextServicemanOffset != 0)
		{
			memset(temp, '\0', 4);
			sprintf(temp, "%d", sno);
			strcat(recvbuf, temp);
			strcat(recvbuf, "::");
			strcat(recvbuf, man.name);
			strcat(recvbuf, "\t");
			strcat(recvbuf, man.role);
			strcat(recvbuf, "\n");
			fseek(fp, man.nextServicemanOffset, SEEK_SET);
			fread(&man, sizeof(struct serviceman), 1, fp);
			sno += 1;
		}
		memset(temp, '\0', 4);
		sprintf(temp, "%d", sno);
		strcat(recvbuf, temp);
		strcat(recvbuf, "::");
		strcat(recvbuf, man.name);
		strcat(recvbuf, "\t");
		strcat(recvbuf, man.role);
		strcat(recvbuf, "\n");
	}
	strcat(recvbuf, "\n\nOptions\n\n1.Select a serviceman\n2.Back\n");
	
	replyto_client(recvbuf, csock);
	memset(recvbuf, '\0', 1024);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	int choice;
	sscanf(recvbuf, "%d", &choice);
	switch (choice)
	{
		case 1:
			selectServiceman(fp,recvbuf,recvbuf_len,csock,categoryOffset,username);
		case 2:
			return;
	}
}

int getNumberOfServicemen(FILE *fp, int categoryOffset)
{
	fseek(fp, categoryOffset, SEEK_SET);
	struct calendarCategory cat;
	memset(&cat, 0, sizeof(struct calendarCategory));
	fread(&cat, sizeof(struct calendarCategory), 1, fp);
	int number = 0;
	if (cat.servicemanOffset == 0)
		number = 0;
	else
	{
		struct serviceman man;
		memset(&man, 0, sizeof(struct serviceman));
		fseek(fp, cat.servicemanOffset, SEEK_SET);
		fread(&man, sizeof(struct serviceman), 1, fp);
		number = 1;
		while (man.nextServicemanOffset != 0)
		{
			number++;
			fseek(fp, man.nextServicemanOffset, SEEK_SET);
			fread(&man, sizeof(struct serviceman), 1, fp);
		}
	}
	return number;
}
void selectServiceman(FILE *fp, char recvbuf[], int recvbuf_len, int *csock,int categoryOffset,char username[])
{
	int manOffset,manID;
	memset(recvbuf, '\0', 1024);
	strcpy(recvbuf, "Enter the ServicemenID\n");
	replyto_client(recvbuf, csock);
	memset(recvbuf, '\0', 1024);
	int recv_byte_cnt = 0;
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	sscanf(recvbuf, "%d", &manID);
	int total_no_of_service_men = getNumberOfServicemen(fp,categoryOffset);
	if (manID<1 || manID>total_no_of_service_men)
	{
		memset(recvbuf, '\0', 1024);
		strcpy(recvbuf, "Entered serviceman ID is invlaid");
		strcat(recvbuf, "Do you want to continue?(y/n)\n");
		replyto_client(recvbuf, csock);
		memset(recvbuf, '\0', 1024);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
		if (!strcmp(recvbuf, "y"))
			return;
		else
			exit(0);
	}
	fseek(fp, categoryOffset, SEEK_SET);
	struct calendarCategory cat;
	memset(&cat, 0, sizeof(struct calendarCategory));
	fread(&cat, sizeof(struct calendarCategory), 1, fp);
	fseek(fp, cat.servicemanOffset, SEEK_SET);
	struct serviceman man;
	memset(&man, 0, sizeof(struct serviceman));
	fread(&man, sizeof(serviceman), 1, fp);
	int no_of_servicemen_in_the_category = 1;
	manOffset = cat.servicemanOffset;
	while (no_of_servicemen_in_the_category < manID && man.nextServicemanOffset != 0)
	{
		no_of_servicemen_in_the_category++;
		manOffset = man.nextServicemanOffset;
		fseek(fp, man.nextServicemanOffset, SEEK_SET);
		fread(&man, sizeof(struct serviceman), 1, fp);
	}
	if (man.firstappointmentOffset == 0)
	{
		strcpy(recvbuf, "\nNo appointments add so far\n");
	}
	else
	{
		strcpy(recvbuf, "\nThe appointments are\n\n");
		fseek(fp, man.firstappointmentOffset, SEEK_SET);
		struct appointment app;
		memset(&app, 0, sizeof(struct appointment));
		fread(&app, sizeof(struct appointment), 1, fp);
		while (app.next != 0)
		{
			strcat(recvbuf, app.username);
			strcat(recvbuf, "booked an appointment on ");
			char temp[11];
			memset(temp, '\0', 11);
			sprintf(temp, "%d/%d%d", app.date[0], app.date[1], app.date[2]);
			strcat(recvbuf, temp);
			strcat(recvbuf, "\n");
			fseek(fp, app.next, SEEK_SET);
			fread(&app, sizeof(struct appointment), 1, fp);
		}
		strcat(recvbuf, app.username);
		strcat(recvbuf, " booked an appointment on ");
		char temp[11];
		memset(temp, '\0', 11);
		sprintf(temp, "%d/%d/%d", app.date[0], app.date[1], app.date[2]);
		strcat(recvbuf, temp);
		//strcat(recvbuf, "\n");
	}
	strcat(recvbuf, "\n\n1.Add a an appointment\n2.Back\n");
	replyto_client(recvbuf, csock);
	memset(recvbuf, '\0', 1024);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	int choice;
	sscanf(recvbuf, "%d", &choice);
	switch (choice)
	{
	case 1:
		addAppointment(fp,recvbuf,recvbuf_len,csock,manOffset,username);
	case 2:
		return;
	}
}

void convert(char *charDate, int intDate[])
{
	int j = 0;
	for (int i = 0; charDate[i] != '\0'; i++)
	{
		if (charDate[i] == '/')
		{
			j++;
			continue;
		}
		else
		{
			intDate[j] = intDate[j] * 10 + (charDate[i] - '0');
		}
	}
}

int validate(int intDate[])
{
	SYSTEMTIME str_t;
	GetSystemTime(&str_t);
	int daysInMonths[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int index;
	int monthNo = intDate[1];
	if (intDate[1] == 2) //if the month is feburary
	{
		if (intDate[2] % 400 == 0 || (intDate[2] % 4 == 0 && intDate[2] % 100 != 0))//leap year condition
			daysInMonths[1] = 29;
		else
			daysInMonths[1] = 28;
	}
	if (intDate[0]>daysInMonths[monthNo - 1] || intDate[0]<1 || monthNo>12 || monthNo<1 || intDate[2]<str_t.wYear)//condition for date to be invalid
		return 0;
	else if (intDate[2] == str_t.wYear && intDate[1]<str_t.wMonth)
		return 0;
	else if (intDate[1] == str_t.wMonth && intDate[2] == str_t.wYear && intDate[0]<str_t.wDay)
		return 0;//date is valid
	else
		return 1;

}

int sixMonthsLater(int intDate[])
{
	SYSTEMTIME str_t;
	GetSystemTime(&str_t);
	if (str_t.wMonth += 6 > 12)
	{
		str_t.wYear += 1;
		str_t.wMonth = (str_t.wMonth + 6) % 12;
	}
	if (intDate[2]>str_t.wYear)
		return 0;
	else if (intDate[2] == str_t.wYear && intDate[1]>str_t.wMonth)
		return 0;
	else if (intDate[2] == str_t.wYear && intDate[1] == str_t.wMonth && intDate[0]>str_t.wDay)
		return 0;
	else
		return 1;

}

int isBooked(FILE *fp,int intDate[], int manOffset)
{
	fseek(fp, manOffset, SEEK_SET);
	struct serviceman man;
	memset(&man, 0, sizeof(struct serviceman));
	fread(&man, sizeof(struct serviceman), 1, fp);
	if (man.firstappointmentOffset == 0)
		return 0;
	fseek(fp, man.firstappointmentOffset, SEEK_SET);
	struct appointment app;
	memset(&app, 0, sizeof(struct appointment));
	fread(&app, sizeof(struct appointment), 1, fp);
	while (app.next != 0)
	{
		if (intDate[2] == app.date[2] && intDate[1]==app.date[1] && intDate[0]==app.date[0])
			return 1;
		fseek(fp, app.next, SEEK_SET);
		fread(&app, sizeof(struct appointment), 1, fp);
	}
	if (intDate[2] == app.date[2] && intDate[1] == app.date[1] && intDate[0] == app.date[0])
		return 1;
	return 0;
}

void addAppointment(FILE *fp, char recvbuf[], int recvbuf_len, int *csock, int manOffset,char username[])
{
	memset(recvbuf, '\0', recvbuf_len);
	struct appointment new_app,app;
	memset(&new_app,0,sizeof(struct appointment));
	memset(&app, 0, sizeof(struct appointment));
	struct serviceman man;
	memset(&man, 0, sizeof(struct serviceman));
	strcpy(recvbuf, "\nEnter the date you want to book your ticket (dd/mm/yyyy format)::");
	int intDate[3] = { 0 };
	replyto_client(recvbuf, csock);
	int recv_byte_cnt = 0;
	memset(recvbuf, '\0', 1024);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		//free(csock);
		return;
	}
	convert(recvbuf, intDate);
	//sscanf(recvbuf, "%d/%d/%d", &intDate[0], &intDate[1], &intDate[2]);
	if (validate(intDate) && sixMonthsLater(intDate))
	{
			if (!isBooked(fp, intDate, manOffset))
			{
				strcpy(new_app.username, username);
				new_app.next = 0;
				for (int i = 0; i < 3; i++)
				{
					new_app.date[i] = intDate[i];
				}
			}
			else
			{
				strcpy(recvbuf, "\nEntered date is already booked\n\nDo you want to continue(y/n)");
				replyto_client(recvbuf, csock);
				memset(recvbuf, '\0', 1024);
				if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
					fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
					//free(csock);
					return;
				}
				if (!strcmp(recvbuf, "y"))
					return;
				else
					exit(0);
			}
	}
	else
	{
		strcpy(recvbuf, "\nEntered date is invalid to book\n\nDo you want to continue(y/n)");
		replyto_client(recvbuf, csock);
		memset(recvbuf, '\0', 1024);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			//free(csock);
			return;
		}
		if (!strcmp(recvbuf, "y"))
			return;
		else
			exit(0);
	}
	fseek(fp, manOffset, SEEK_SET);
	int no_of_appointments = 0;
	fseek(fp, end_of_servicemen, SEEK_SET);
	fread(&no_of_appointments, sizeof(int), 1, fp);
	fseek(fp, no_of_appointments * 32, SEEK_CUR);
	int location = ftell(fp);
	fwrite(&new_app, sizeof(struct appointment), 1, fp);
	fseek(fp, manOffset, SEEK_SET);
	fread(&man, sizeof(struct serviceman), 1, fp);
	if (man.firstappointmentOffset == 0)
	{
		man.firstappointmentOffset = location;
		fseek(fp, -64, SEEK_CUR);
		fwrite(&man, sizeof(struct serviceman), 1, fp);
		no_of_appointments += 1;
		fseek(fp, end_of_servicemen, SEEK_SET);
		fwrite(&no_of_appointments, sizeof(int), 1, fp);
	}
	else
	{
		int prev = man.firstappointmentOffset;
		fseek(fp, prev, SEEK_SET);
		fread(&app, sizeof(struct appointment), 1, fp);
		while (app.next != 0)
		{
			prev = app.next;
			fseek(fp, app.next, SEEK_SET);
			fread(&app, sizeof(struct appointment), 1, fp);
		}
		app.next = location;
		fseek(fp, -32, SEEK_CUR);
		fwrite(&app, sizeof(struct appointment), 1, fp);
		no_of_appointments += 1;
		fseek(fp, end_of_servicemen, SEEK_SET);
		fwrite(&no_of_appointments, sizeof(int), 1, fp);
	}
}