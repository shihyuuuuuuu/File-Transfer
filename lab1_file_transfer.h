#include <netinet/in.h>
#include <stdio.h>
#include <time.h>

int protocol, action;
FILE* fp;
char fileName[30];
int sockfd;
struct sockaddr_in info;
void setArguments(char** argv);
void sendMsg(int sockfd, char* msg);
void sendFile(int sockfd, FILE* fp);
int tcp();
int udp();
time_t rawtime;
struct tm* timeinfo;
