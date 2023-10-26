#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include<string>
#include<map>

#define NET_PACKET_DATA_SIZE 5000

typedef std::map<int, std::pair<std::string, int>> FDMAPIP; // socketFd， client name, 未接收到心跳的次数

enum Type {
	HEART = 1,
	OTHER = 2,
};

// 支线程传递的参数结构体
struct CustomParam {
	int fd; // 客户端用
	FDMAPIP *mmap; // 服务端用
};

// 网络数据包包头
struct NetPacketHeader{
	Type type;
	unsigned short wDataSize; // 数据包大小，包含包头的长度和数据长度
};

// 网络数据包体
struct NetPacket{
	NetPacketHeader Header;	// 包头
	unsigned char Data[NET_PACKET_DATA_SIZE]; // 数据
};



#endif