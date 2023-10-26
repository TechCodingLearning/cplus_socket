#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<vector>
#include"protocol.h"
using namespace std;

#define BUF_SIZE 512
#define ERR_EXIT(m) \
	do \
	{	\
		perror(m);	\
		exit(EXIT_FAILURE);	\
	} while (0)

void* send_heart(void *arg) {
	printf("the heartbeat sending thread started.\n");
	struct CustomParam *param = (struct CustomParam*) arg;
	// int count = 0;
	while(true) {
		NetPacket sendPacket;
		sendPacket.Header.type = HEART;
		sendPacket.Header.wDataSize = sizeof(NetPacketHeader);
		send(param->fd, &sendPacket, sendPacket.Header.wDataSize, 0);
		sleep(3);
	}
}

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

	struct CustomParam param;
	param.fd = m_sockfd;
	pthread_t id;
	int ret = ::pthread_create(&id, nullptr, send_heart, &param);
	if(ret != 0) {
		printf("create thread faild.\n");
	}
	ret = pthread_detach(id);
	if(ret != 0) {
		printf("failed to detach thread.\n");
	}

	// 发送并接收数据
	char buffer[BUF_SIZE];
	while(true){
		
		memset(buffer, 0, sizeof(buffer)); // 重置缓冲区

		// 发送并接收数据
		printf("client send:\n");
		scanf("%s", buffer);
		int dataSize = strlen(buffer); // 未必填满BUF_SIZE, strlen到\0结束
		
		NetPacket sendPacket;
		sendPacket.Header.type = OTHER;
		sendPacket.Header.wDataSize = dataSize + sizeof(NetPacketHeader);
		
		memcpy(sendPacket.Data, buffer, dataSize); // 数据拷贝

		send(m_sockfd, &sendPacket, sendPacket.Header.wDataSize, 0);
		// recv(m_sockfd, buffer, sizeof(buffer), 0);
		// printf("client recv:%s\n", buffer);
	}

	// 断开连接
	close(m_sockfd);
	printf("client socket closed!!!\n");

	return 0;
}
