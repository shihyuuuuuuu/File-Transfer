#include "lab1_file_transfer.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define TCP 0
#define UDP 1
#define SEND 0
#define RECV 1

int main(int argc, char** argv){
	
	setArguments(argv);
	switch(protocol){
		case TCP:
			tcp();
			break;
		case UDP:
			udp();
			break;
		default:
			break;
	}
}



void setArguments(char** argv){
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
		printf("Fail to create a socket.\n");
	else
		printf("Socket created.\n");
    
	if(strcmp(argv[1], "tcp") == 0){
		printf("Using TCP...\n");
		protocol = TCP;
    }else if(strcmp(argv[1], "udp") == 0){
		printf("Using UDP...\n");
		protocol = UDP;
    }else{
        printf("Unvalid protocol!\n");
    }
    
	if(strcmp(argv[2], "send") == 0){
		printf("Sending file\n");
		action = SEND;
    }else if(strcmp(argv[2], "recv") == 0){
		printf("Receiving file\n");
		action = RECV;
    }else{
        printf("Unvalid send/recv!\n");
    }
	
	bzero(&info, sizeof(info)); //initialize all bits of "info" to zero
	info.sin_family = AF_INET; //IPV4
	info.sin_addr.s_addr = inet_addr(argv[3]); //ip
	info.sin_port = htons(atoi(argv[4])); //port
	
	if(action == SEND){
		strncpy(fileName, argv[5], strlen(argv[5]));
		printf("filename = %s\n", fileName);
	}
	//printf("ip = %lu\n", info.sin_addr.s_addr);
	printf("port = %u\n", ntohs(info.sin_port));
}

int tcp(){

	if(action == RECV){
		char message[] = {"Hi, there."};
		char receiveMessage[100] = {};
		
		/*Connect to the server.*/
		int err = connect(sockfd, (struct sockaddr *)&info, sizeof(info));
		if(err == -1)
			printf("Connection error.\n");
		
		/*Send a message to the server.*/
		sendMsg(sockfd, message);


		/*Receive a message from the server.*/
		recv(sockfd, receiveMessage, sizeof(receiveMessage),0);
		printf("%s", receiveMessage);
			
		printf("close Socket\n");
		close(sockfd);
		return 0;

	}else if(action == SEND){
		char inputBuffer[256] = {};
		char message[] = {"Hi, this is server.\n"};
		
		int forClientSockfd = -1;
		struct sockaddr_in clientInfo;
		socklen_t addrlen = sizeof(clientInfo);
		bzero(&clientInfo, sizeof(clientInfo));

		/*Bind itself to the socket.*/
		int err = bind(sockfd, (struct sockaddr *)&info, sizeof(info));
		if(err == -1){
			printf("Binding error.\n");
			perror("Binding error ");
		}else
			printf("Binding Success.\n");
		
		/*Listen to the socket.*/
		int lisErr = listen(sockfd, 5);
		if(lisErr == 0)
			printf("Listening...\n");
		else
			printf("You didn't hear anything...\n");
		
		/*Keep waiting until someone come and then accept it.*/
		while(1){
			forClientSockfd = accept(sockfd, (struct sockaddr*)&clientInfo, &addrlen);
			if(forClientSockfd != -1){
				printf("Accept one request.\n");
				break;
			}
		}

		/*Receive a message.*/
		recv(forClientSockfd, inputBuffer, sizeof(inputBuffer), 0);
		printf("%s\n", inputBuffer);

		/*Send a message.*/
		sendMsg(forClientSockfd, message);

		/*Send a file.*/
		
		FILE* fp = fopen(fileName,"r");
		if(fp == NULL){
			printf("Open failure.\n");
		}else{
			printf("Open success.\n");
			//sendFile(forClientSockfd, fp);
		}
			

		return 0;
	}
}

int udp(){

}

void sendMsg(int sockfd, char* msg){
	int sendErr = send(sockfd, msg, strlen(msg), 0);
	if(sendErr != -1){
		printf("Message sent.\n");
	}else{
		perror("Sending error ");
	}
}

void recvMsg(){

}



/*
1. server: send  client: receive
2. open file
3. send and recv
4. udp
*/
