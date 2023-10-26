#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include"protocol.h"
using namespace std;

#define BUF_SIZE 512
#define ERR_EXIT(m) \
	do \
	{	\
		perror(m);	\
		exit(EXIT_FAILURE);	\
	} while (0)

int main() {
	// 创建套接字
	int m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(m_sockfd < 0) {
		ERR_EXIT("create socket fail");
	}

	// 服务器的ip为本地，端口号
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(39002);

	// 向服务器发送连接请求
	int m_connectfd = connect(m_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(m_connectfd < 0) {
		ERR_EXIT("connect server fail");
	}

	// 发送并接收数据
	char buffer[BUF_SIZE] = "sad";
	int dataSize = strlen(buffer);
	
	NetPacket sendPacket;
	sendPacket.Header.type = OTHER;
	sendPacket.Header.wDataSize = dataSize + sizeof(NetPacketHeader);
	
	memcpy(sendPacket.Data, buffer, dataSize); // 数据拷贝

	for(int i = 0; i < 3; i++) {
		send(m_sockfd, &sendPacket, sendPacket.Header.wDataSize, 0);
	}
	printf("client send succ!!!\n");

	// 断开连接
	close(m_sockfd);
	printf("client socket closed!!!\n");

	return 0;
}
