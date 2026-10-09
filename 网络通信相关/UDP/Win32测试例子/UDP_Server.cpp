// UDP_Server.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

static std::string Length2HMSF(int nLength,double dfps)
{
	int HH,MM,SS,FF;
	HH = 0;
	while(nLength >= 60*60*dfps)
	{
		nLength -= 60*60*dfps;
		HH++;
	}
	MM = 0;
	while(nLength >= 60*dfps)
	{
		nLength -= 60*dfps;
		MM++;
	}
	SS = 0;
	while(nLength >= dfps)
	{
		nLength -= dfps;
		SS++;
	}
	FF = nLength; 

	
	std::stringstream ss;
	ss << std::setw(2) << std::setfill('0') << HH;
	ss << std::string(":");
	ss << std::setw(2) << std::setfill('0') << MM;
	ss << std::string(":");
	ss << std::setw(2) << std::setfill('0') << SS;
	//ss << std::string(":");
	//ss << std::setw(2) << std::setfill('0') << FF;


	std::string str = ss.str();
	return str;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// 初始化WinSock
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 创建套接字
	SOCKET m_serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// 设置套接字为广播模式
	int reuse = 1;
	setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
	setsockopt(m_serverSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&reuse, sizeof(reuse));

	// 绑定地址和端口
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(6788);//用以接收客户端的消息
	bind(m_serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

	// 发送广播
	std::string message = "<showinfo><type>1</type><info>00:00:13</info><chl>49</chl><ex1></ex1><num></num></showinfo>";
	sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(6789);//向客户端发送消息,客户端需绑定此端口
	clientAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

	
	int  nTotal = 600; //10分钟
	int  nPlayed = 0;  //已播节目时长
	int  ItemTime[5] = {60,90,180,150,120};//5条节目

	int nCurItemIdx = 0;
	int nItemTotal = 0;
	int nPlayedItem = 0;
	while (true) 
	{
		if (nPlayed >= 590)
		{
			nPlayed = 0;
			nCurItemIdx = 0;
		}

		//检查当前播到的节目Index
		int nCheckCount = 0;
		for (int i = 0; i < 5; i++)
		{
			nCheckCount += ItemTime[i];
			if (nPlayed <= nCheckCount)
			{
				nCurItemIdx = i;
				break;
			}
		}
		

		//已播完整节目
		nPlayedItem = 0;
		for (int i = 0; i < nCurItemIdx; i++)
		{
			nPlayedItem += ItemTime[i];
		}

		

		//播到节目总长
		nItemTotal = 0;
		for (int i = 0; i < nCurItemIdx+1; i++)
		{
			nItemTotal += ItemTime[i];
		}


		//总倒计时
		{
			std::stringstream ss;
			ss << "<showinfo><type>10</type><info>";
			ss <<  Length2HMSF(nTotal-nPlayed, 1.0);
			ss << "</info><chl>49</chl><ex1></ex1><num></num></showinfo>";
			message = ss.str();
			sendto(m_serverSocket, message.c_str(), message.size(), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
			std::cout << "发送总倒计时：" << message << std::endl;
		}

		//单条倒计时
		{
			std::stringstream ss;
			ss << "<showinfo><type>11</type><info>";
			ss <<  Length2HMSF(nItemTotal-nPlayed, 1.0);
			ss << "</info><chl>49</chl><ex1></ex1><num></num></showinfo>";
			message = ss.str();
			sendto(m_serverSocket, message.c_str(), message.size(), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
			std::cout << "发送单条倒计时：" << message << std::endl;
		}
		
		//单条正计时
		{
			std::stringstream ss;
			ss << "<showinfo><type>1</type><info>";
			ss <<  Length2HMSF(nPlayed-nPlayedItem, 1.0);
			ss << "</info><chl>49</chl><ex1></ex1><num></num></showinfo>";
			message = ss.str();
			sendto(m_serverSocket, message.c_str(), message.size(), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
			std::cout << "发送正计时：" << message << std::endl;

		}

		Sleep(1000);
		nPlayed++;
	}

	// 关闭套接字
	closesocket(m_serverSocket);

	// 清理WinSock
	WSACleanup();

	return 0;
}
