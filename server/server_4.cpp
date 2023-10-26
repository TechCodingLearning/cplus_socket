#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/epoll.h>  // linux机器上执行，macos使用kqueue
using namespace std;

#define EPOLL_SIZE 1023 // epoll监听客户端数
#define MAX_EVENTS 64
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

	// 创建一个监听描述符epoll，并将监听套接字加入监听红黑树
	int epollfd = epoll_create(EPOLL_SIZE);
	if(epollfd < 0) {
		ERR_EXIT("epoll create fail");
	}

	struct epoll_event epe;
	epe.events = EPOLLIN;
	epe.data.fd = m_sockfd;

	// 控制epoll文件描述符上的动作
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, m_sockfd, &epe) < 0) {
		ERR_EXIT("epoll control fail");
	}

	struct epoll_event epeList[MAX_EVENTS];

	// 接收客户端数据，并响应
	char buffer[BUF_SIZE];

	while(true){
		int ret = epoll_wait(epollfd, epeList, MAX_EVENTS, -1); // 监听等待就绪描述符加入就绪队列
		if(ret < 0) {
			ERR_EXIT("epoll fail");
		}else if(ret == 0) {
			printf("epoll timeout\n");
			continue;
		}

		for(int i = 0; i < ret; i++) {
			// 客户端请求连接
			if(epeList[i].data.fd == m_sockfd) {
				int m_connfd = accept(m_sockfd,	(struct sockaddr*)&client_addr, &client_len);
				if(m_connfd < 0) {
					ERR_EXIT("server accept fail");
				}

				// 将客户端新建立的连接添加到epoll的监听中
				struct epoll_event epe;
				epe.events = EPOLLIN | EPOLLRDHUP; // 监听套接字的可读和退出
				epe.data.fd = m_connfd;
				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, m_connfd, &epe) < 0) { // 将新的套接字加入监听
					ERR_EXIT("epoll control fail, accept client");
				}

				printf("we got a new connection, client_socket=%d, ip=%s, port=%d\n", m_connfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

			}else if(epeList[i].events & EPOLLIN) {
				memset(buffer, 0, sizeof(buffer)); // 重置缓冲区
				int recv_len = recv(epeList[i].data.fd, buffer, sizeof(buffer), 0);
				if(recv_len < 0) {
					ERR_EXIT("recv data fail");
				}else if(recv_len == 0) { // 客户端断开连接
					// 打开断开的客户端数据
					epoll_ctl(epollfd, EPOLL_CTL_DEL, epeList[i].data.fd, &epe);
					close(epeList[i].data.fd);
				}else {
					printf("server recv:%s\n", buffer);
					strcat(buffer, "+ACK");
					send(epeList[i].data.fd, buffer, sizeof(buffer), 0);
				}
			}else { // 客户端退出
				printf("a client is quit, ip=%s, port=%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				epoll_ctl(epollfd, EPOLL_CTL_DEL, epeList[i].data.fd, &epe);
				close(epeList[i].data.fd);
			}
		}
	}

	// 关闭套接字
	close(m_sockfd);

	printf("server socket closed!!!\n");

	return 0;
}
