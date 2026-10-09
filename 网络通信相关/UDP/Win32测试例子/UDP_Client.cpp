// UDP_Client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <string>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	// 初始化WinSock
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 创建套接字
	SOCKET m_clientSocket = socket(AF_INET, SOCK_DGRAM, 0/*IPPROTO_UDP*/);

	// 设置套接字为广播模式
	int reuse = 1;
	setsockopt(m_clientSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
	setsockopt(m_clientSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&reuse, sizeof(reuse));

	// 绑定地址和端口
	sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(6789);//参考服务端发送端口,需与之相同
	clientAddr.sin_addr.s_addr = INADDR_ANY;
	bind(m_clientSocket, (sockaddr*)&clientAddr, sizeof(clientAddr));

	// 接收广播
	char buffer[1024];
	while (true) 
	{
		memset(buffer, 0, sizeof(buffer));
		recvfrom(m_clientSocket, buffer, sizeof(buffer), 0, NULL, NULL);
		std::cout << "收到广播消息：" << buffer << std::endl;
	}

	// 关闭套接字
	closesocket(m_clientSocket);

	// 清理WinSock
	WSACleanup();

	return 0;
}

