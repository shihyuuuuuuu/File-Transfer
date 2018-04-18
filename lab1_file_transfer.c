#include "lab1_file_transfer.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define TCP 0
#define UDP 1
#define SEND 0
#define RECV 1
#define BUFFERSIZE 10000
#define BIG 1
#define SMALL 0

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
		printf("Sending file ");
		action = SEND;
    }else if(strcmp(argv[2], "recv") == 0){
		printf("Receiving file\n");
		action = RECV;
    }else{
        printf("Unvalid send/recv!\n");
		exit(0);
    }
	
	bzero(&info, sizeof(info)); //initialize all bits of "info" to zero
	info.sin_family = AF_INET; //IPV4
	info.sin_addr.s_addr = inet_addr(argv[3]); //ip
	info.sin_port = htons(atoi(argv[4])); //port
	
	if(action == SEND){
		strncpy(fileName, argv[5], strlen(argv[5]));
		printf("\"%s\"\n", fileName);
	}
}

int tcp(){
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("Fail to create a socket.");
		exit(1);
	}
	else
		printf("Socket created.\n");

	if(action == RECV){
		
		int err = connect(sockfd, (struct sockaddr *)&info, sizeof(info));
		char buffer[BUFFERSIZE] = {};
		char name[30];
	    int leng = recvfrom(sockfd, name, sizeof(name), 0, NULL, NULL);
		ssize_t rec = 0;
		
		/*Connect to the server.*/
		if(err == -1)
			printf("Connection error.\n");
		
		fp = fopen(name, "w");
		while((rec = recvfrom(sockfd, buffer, sizeof(buffer), MSG_WAITALL, NULL, NULL)) > 0){
			//printf("bytes read: %zd bytes\n", rec);
			fwrite(buffer, 1, rec, fp);
		}
			
		printf("close Socket\n");
		close(sockfd);
		return 0;

	}else if(action == SEND){
		
		int forClientSockfd = -1;
		int err = bind(sockfd, (struct sockaddr *)&info, sizeof(info));
		int lisErr = listen(sockfd, 5);
		struct sockaddr_in clientInfo;
		socklen_t addrlen = sizeof(clientInfo);
		bzero(&clientInfo, sizeof(clientInfo));

		/*Bind itself to the socket.*/
		if(err == -1){
			printf("Binding error.\n");
			perror("Binding error ");
		}else
			printf("Binding Success.\n");
		
		/*Listen to the socket.*/
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

		/*Send a file.*/
		fp = fopen(fileName,"rb");
		if(fp == NULL){
			printf("Open failure.\n");
		}else{
			printf("Open success.\n");
			sendFile(forClientSockfd, fp);
		}
			

		return 0;
	}
}

int udp(){
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("Socket failed");
		exit(1);
	}
	int len = sizeof(info);
	
	if(action == RECV){
		
		char ask[] = {"request"};
		char buffer[BUFFERSIZE];
		char name[20];
		int  rec;
		
		/*Send a request to server */
		if((sendto(sockfd, ask, sizeof(ask), 0, (struct sockaddr *)&info, sizeof(info))) >= 0)
			printf("send request\n");
		recvfrom(sockfd, name, sizeof(name), 0, (struct sockaddr *)&info, &len);
		fp = fopen(name, "w");
		
		sendto(sockfd, ask, sizeof(ask), 0, (struct sockaddr *)&info, sizeof(info));
		
		while((rec = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&info, &len)) > 0){
			/* my thought: recvfrom can't know whether the remote had closed the socket or not*/
			if((rec == 8) && ('e' == buffer[0]) && ('i' == buffer[1]) && ('k' == buffer[2]) && ('4' == buffer[3])
					&& ('m' == buffer[4]) && ('x' == buffer[5]) && ('i' == buffer[6]) && ('e' == buffer[7]))
				break;

			fwrite(buffer, 1, rec, fp);
			sendto(sockfd, ask, sizeof(ask), 0, (struct sockaddr *)&info, sizeof(info));
		}
		//printf("buffer: %s\n", buffer);	
		printf("close Socket\n");
		close(sockfd);

	}else if(action == SEND){
		int err;
		if((err = bind(sockfd, (struct sockaddr *)&info, sizeof(info))) == -1){
			perror("Binding error ");
			exit(1);
		}else
			printf("Binding Success.\n");

		fp = fopen(fileName,"rb");
		if(fp == NULL){
			perror("Open failure.");
		}else{
			printf("Open success.\n");
			sendFile(sockfd, fp);
		}
		
		return 0;
	}
}

void sendFile(int sockfd, FILE* fp){
	char buffer[BUFFERSIZE];
	
	/*Calculate the file size.*/
	fseek(fp, 0L, SEEK_END);
	printf("file size: %ld bytes\n", ftell(fp));
	double fileSize = ftell(fp);
	rewind(fp);
	
	double level = fileSize/20;
	double fivePercent = fileSize/20;
	int percent = 0;
	double sizeSent = 0;
	int bytesent = 0;
	
	if(protocol == TCP){
		int result = BUFFERSIZE, round = 0, size = BIG;
		
		send(sockfd, fileName, sizeof(fileName), 0);
		
		while(1){
			if(size == BIG){
				result = fread(buffer, 1, BUFFERSIZE, fp);
				if(result != BUFFERSIZE){
					fseek(fp, round*BUFFERSIZE, SEEK_SET);
					result = fread(buffer, 1, 1, fp);
					size = SMALL;
				}
			}else if(size == SMALL){
				result = fread(buffer, 1, 1, fp);
			}
			//printf("I read %d bytes\n", result);
	
			if(size == BIG){
				bytesent = send(sockfd, buffer, BUFFERSIZE, 0);
				round++;
			}else if(size == SMALL){
				bytesent = send(sockfd, buffer, 1, 0);
			}

			time(&rawtime);
			timeinfo = localtime(&rawtime);
			sizeSent += bytesent;
			if(sizeSent >= level){
				level += fivePercent;
				percent += 5;
				printf("%d%%  %s", percent, asctime(timeinfo));
			}

			//printf("byte sent: %d byte\n", bytesent);
			
			if(feof(fp)){
				close(sockfd);
				break;
			}
		}
	}else if(protocol == UDP){
		int len = sizeof(info);
		int count = 0, size = BIG, round = 0;
		size_t result;
		bool first = true;
		char buffer1[100] = "";

		
		while(1){
		
			if(size == BIG){
				result = fread(buffer, 1, BUFFERSIZE, fp);
				if(result != BUFFERSIZE){
					fseek(fp, round*BUFFERSIZE, SEEK_SET);
					fread(buffer, 1, 1, fp);
					//printf("remain: %lu bytes\n", result);
					size = SMALL;
				}
			}else if(size == SMALL){
				fread(buffer, 1, 1, fp);
			}
			
			recvfrom(sockfd, buffer1, sizeof(buffer1), 0, (struct sockaddr *)&info, &len);
			if(strcmp(buffer1, "request") == 0 && first){
				sendto(sockfd, fileName, sizeof(fileName), 0, (struct sockaddr *)&info, sizeof(info));
				first = false;
				fseek(fp, 0, SEEK_SET);
			}else if(strcmp(buffer1, "request") == 0){
				if(size == BIG){
					bytesent = sendto(sockfd, buffer, BUFFERSIZE, 0, (struct sockaddr *)&info, sizeof(info));
					round++;
				}else{
					bytesent = sendto(sockfd, buffer, 1, 0, (struct sockaddr *)&info, sizeof(info));
				}
			}
			
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			sizeSent += bytesent;
			if(sizeSent >= level){
				level += fivePercent;
				percent += 5;
				printf("%d%%  %s", percent, asctime(timeinfo));
			}
			
			if(feof(fp)){
				strcpy(buffer, "eik4mxie");
				sendto(sockfd, buffer, 8, 0, (struct sockaddr *)&info, sizeof(info));
				close(sockfd);
				break;
			}
		}
	}
}
