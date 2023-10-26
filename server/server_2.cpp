#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
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

	// 初始化socket元素
	struct sockaddr_in server_addr;
	size_t server_len = sizeof(server_addr);
	memset(&server_addr, 0, server_len);

	server_addr.sin_family = AF_INET;
	//server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(39002);

	// 绑定文件描述符和服务器的ip和端口号
	int m_bindfd = bind(m_sockfd, (struct sockaddr *)&server_addr, (socklen_t)server_len);
	if(m_bindfd < 0) {
		ERR_EXIT("bind ip and port fail");
	}

	// 进入监听状态，等待用户发起请求
	int m_listenfd = listen(m_sockfd, 20);
	if(m_listenfd < 0) {
		ERR_EXIT("listen client fail");
	}

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
		memset(buffer, 0, sizeof(buffer)); // 重置缓冲区
		ssize_t recvLen = recv(m_connfd, buffer, sizeof(buffer), 0);
		if(recvLen <= 0) {
			close(m_connfd);
			m_connfd = -1;
			printf("client lose connection!!!\n");
			continue;
		}
		printf("server recv:%s\n", buffer);
		strcat(buffer, "+ACK");
		send(m_connfd, buffer, sizeof(buffer), MSG_NOSIGNAL);
	}

	// 关闭套接字
	close(m_connfd);
	close(m_sockfd);

	return 0;
}
	