#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include"protocol.h"
using namespace std;

#define EPOLL_SIZE 1023 // epoll监听客户端数
#define MAX_EVENTS 64
#define BUF_SIZE 2*NET_PACKET_DATA_SIZE
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

	// 初始化socket元素
	struct sockaddr_in server_addr;
	size_t server_len = sizeof(server_addr);
	memset(&server_addr, 0, server_len);

	server_addr.sin_family = AF_INET;
	//server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(39002);

	// 绑定文件描述符和服务器的ip和端口号
	int m_bindfd = ::bind(m_sockfd, (struct sockaddr *)&server_addr, (socklen_t)server_len);
	if(m_bindfd < 0) {
		ERR_EXIT("bind ip and port fail");
	}

	// 进入监听状态，等待用户发起请求
	int m_listenfd = listen(m_sockfd, 20);
	if(m_listenfd < 0) {
		ERR_EXIT("listen client fail");
	}

	printf("client accept success\n");

	// 定义客户端的套接字
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int m_connfd = accept(m_sockfd, (struct sockaddr*)&client_addr, &client_len);

	// 接收客户端数据，并响应
	char buffer[BUF_SIZE];
	while(true){
		if(m_connfd < 0) {
			m_connfd = accept(m_sockfd, (struct sockaddr*)&client_addr, &client_len);
			printf("client accept success again!!!\n");
		}

		// 休眠10s才能有粘包现象出现
		// sleep(10);
		memset(buffer, 0, sizeof(buffer)); // 重置缓冲区
		int nRecvSize = 0; // 一次接收到的数据大小
		int sumRecvSize = 0; // 总共收到的数据大小
		int packetSize = 0; // 数据包长度

		bool disConn = false;

		// 先从缓冲中取出包头
		while(sumRecvSize != sizeof(NetPacketHeader)) {
			nRecvSize = recv(m_connfd, buffer + sumRecvSize, sizeof(NetPacketHeader) - sumRecvSize, 0);
			if(nRecvSize == 0) {
				close(m_connfd);
				m_connfd = -1;
				printf("client close connection!!!\n");
				disConn = true;
				break;
			}
			sumRecvSize += nRecvSize;
		}

		if(disConn) { // 已关闭连接
			continue;
		}

		NetPacketHeader *pHead = (NetPacketHeader*) buffer;
		packetSize = pHead -> wDataSize; // 客户端发送的数据包长度
		
		// 从缓冲中取出数据包体
		while(sumRecvSize != packetSize) {
			nRecvSize = recv(m_connfd, buffer + sumRecvSize, packetSize - sumRecvSize, 0);
			if(nRecvSize == 0) {
				close(m_connfd);
				m_connfd = -1;
				printf("client lose connection!!!\n");
				disConn = true;
				break;
			}else if(nRecvSize < 0) {
				ERR_EXIT("recv fail");
			}
			printf("server recv:%s, size:%d\n", buffer + sumRecvSize, nRecvSize);
			sumRecvSize += nRecvSize;
		}

		if(disConn) {
			continue;
		}
	}

	// 关闭套接字
	close(m_connfd);
	close(m_sockfd);

	printf("server socket closed!!!\n");

	return 0;
}
