#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
using namespace std;

#define PORT 39002
#define MAX_FD_NUM 3
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

	printf("client accept success\n");

	// 定义客户端的套接字
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	// 接收客户端数据，并响应
	char buffer[BUF_SIZE];
	int array_fd[MAX_FD_NUM]; // 在线客户端句柄数组

	// 客户端连接数量
	int client_count = 0;

	struct fd_set tmpfd;
	int max_fd = m_sockfd; // 最大socket
	struct timeval* timeout = nullptr;
	
	for(int i = 0; i < MAX_FD_NUM; i++) {
		array_fd[i] = -1;
	}

	while(true){
		FD_ZERO(&tmpfd);
		FD_SET(m_sockfd, &tmpfd);

		// 所有在线的客户端加入fd中，并找出最大的socket
		for(int i = 0; i < MAX_FD_NUM; i++) {
			if(array_fd[i] > 0) {
				FD_SET(array_fd[i], &tmpfd);
				if(max_fd < array_fd[i]){
					max_fd = array_fd[i];
				}
			}
		}

		int ret = select(max_fd + 1, &tmpfd, nullptr, nullptr, timeout);
		if(ret < 0) {
			ERR_EXIT("select fail");
		}else if(ret == 0) {
			printf("select timeout\n");
			continue;
		}

		// 表示有客户端连接
		if(FD_ISSET(m_sockfd, &tmpfd)){
			int m_connfd = accept(m_sockfd, (struct sockaddr *)&client_addr, &client_len);
			if(m_connfd < 0) {
				ERR_EXIT("server accept fail");
			}

			// 客户端连接数已满
			if(client_count >= MAX_FD_NUM){
				printf("max connections arrive!!!\n");
				close(m_connfd);
				continue;
			}

			// 客户端数量加1
			client_count++;
			printf("we got a new connection, ");
			printf("client_socket=%d, client_count=%d, ip=%s, port=%d\n",
				m_connfd, client_count, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

			for(int i = 0; i < MAX_FD_NUM; i++) {
				if(array_fd[i] == -1) {
					array_fd[i] = m_connfd;
					break;
				}
			}
		}

		// 遍历所有的客户端连接，找到发送数据的那个客户端描述
		for(int i = 0; i < MAX_FD_NUM; i++) {
			if(array_fd[i] < 0) {
				continue;
			}else { // 有客户端发送过来的数据
				if(FD_ISSET(array_fd[i], &tmpfd)) {
					memset(buffer, 0, sizeof(buffer)); // 重置缓冲区
					ssize_t recv_len = recv(array_fd[i], buffer, sizeof(buffer), 0);
					if(recv_len < 0) {
						ERR_EXIT("recv data fail");
					} else if(recv_len == 0){ // 客户端断开连接
						client_count--;
						// 打印断开的客户端数据
						printf("client_socket=[%d] close, client_count=[%d], ip=%s, port=%d\n\n",
								array_fd[i], client_count, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
						close(array_fd[i]);
						FD_CLR(array_fd[i], &tmpfd);
						array_fd[i] = -1;
					}else {
						printf("server recv:%s\n", buffer);
						strcat(buffer, "+ACK");
						send(array_fd[i], buffer, sizeof(buffer), 0);
					}
				}
			}
		}

	}

	// 关闭套接字
	close(m_sockfd);

	printf("server socket closed!!!\n");

	return 0;
}
