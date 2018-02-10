/*
 * main.cpp
 *
 *  Created on: 2017年11月21日
 *      Author: dongyao
 */


// CMDSocketServer.cpp : 定义控制台应用程序的入口点。
//
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <cv.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <list>
#include <unistd.h>
#include <fstream>
//#include "json.h"
using namespace std;

// 服务器监听端口
#define MYPORT 43222
//全局变量
int sockConn;
int sockSrv;
struct sockaddr_in addrSrv, addrClient;
int recv_byte_num;

typedef list<int> SOCKET_LIST;
typedef int SOCKET_INT;
// client socket list across multi-thread.
SOCKET_LIST client_socket_list;
SOCKET_LIST logined_client_list;
// the Mutex for access the prefore list.
//HANDLE listMutex;
pthread_t tid1, tid2;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// 数据发送线程的函数 to all client
void* ThreadSendAllProc(void*);
void* ThreadRecvProc(void*);
SOCKET_INT pop_list(SOCKET_LIST *);
void push_list(SOCKET_LIST *, SOCKET_INT);

#define max(A,B) (A > B ? A : B)
#define mat(a, i, j) (*(a + (n*(i)+j)))

//服务器主函数
int main()
{
	
	//建立服务器socket
	sockSrv = socket(AF_INET, SOCK_STREAM, 0); //if fail, return -1
											   //绑定socket
	{
		addrSrv.sin_family = AF_INET;
		addrSrv.sin_addr.s_addr = INADDR_ANY;
		addrSrv.sin_port = htons(MYPORT);
	}
	bind(sockSrv, (struct sockaddr *)&addrSrv, sizeof(struct sockaddr));
	//监听
	listen(sockSrv, 7); // 只监听的一个
						//创建键盘扫描及数据发送线程
						//CreateThread(NULL,0, ThreadSendAllProc ,NULL,0,NULL);
						//接受连接
						//sockConn=accept(sockSrv,(SOCKADDR*)&addrClient, &IpAddr_len);
	int IpAddr_len = sizeof(struct sockaddr_in);

	while (true) {
		cout << "等待连接" << endl;
		sockConn = accept(sockSrv,(struct sockaddr *)&addrClient, (socklen_t *)&IpAddr_len);
		//显示对方IP地址
		cout << "连接" << (inet_ntoa(addrClient.sin_addr)) << ':' << addrClient.sin_port;
		cout << "\t代号：" << sockConn << endl;
		// 添加到client_socket_list中
		push_list(&client_socket_list, sockConn);
		// 创建数据接收线程

     //	CreateThread(NULL, 0, ThreadRecvProc, NULL, 0, NULL);

		pthread_create(&tid1, NULL, ThreadRecvProc, NULL);
		
		char sendbuf[] = "yes";
		send(sockConn, sendbuf, strlen(sendbuf) + 1, 0);
	}

	return 0;
}

wstring wstr, wstr2;
string test;
// 创建数据接收线程
void* ThreadRecvProc(void*) {
	// 获取 client socket
	SOCKET_INT socketConn = pop_list(&client_socket_list);
	if (socketConn == -1) return 0;
	// 加入到登录的列表中
	logined_client_list.push_back(socketConn);

	int len = 0;
	char *len_char = (char *)malloc(sizeof(char) * 4);
	// 进入循环接收数据
	char buff[1024];
	int sum = 0;
	ofstream file;
	file.open("file.png", ios::binary);
	while ((len = recv(sockConn, buff, 1024, 0))>0)
	{
		sum += len;
		/*cout << "length:" << len << endl;
		cout << buff << endl;*/

		file.write(buff, len);
	//	ZeroMemory(buff, 1024);
		memset(buff,0,1024);

	}
	file.close();

	close(socketConn); //线程结束，关闭socket
	cout << "客户端" << socketConn << " 断开" << endl;
	logined_client_list.remove(socketConn);

}

// TODO
// 数据发送线程的函数
void* ThreadSendAllProc(void*)
{
	//声明i为迭代器
	SOCKET_LIST::iterator socket_iterator;
	while (1)
	{
		char sendBuf[1024];
		memset(sendBuf, 0, 1024); //初始化缓冲区
								  //扫描键盘
								  //scanf("%[^\n]",&sendBuf);
		fflush(stdin); //清除缓冲区残余信息
					   //send(sockClient,sendBuf,strlen(sendBuf)+1,0); //发送数据
		sendBuf[0] = 'y'; sendBuf[1] = 'e'; sendBuf[2] = 's';
		for (socket_iterator = logined_client_list.begin(); socket_iterator != logined_client_list.end(); socket_iterator++) {
			send(*socket_iterator, sendBuf, strlen(sendBuf) + 1, 0); //发送数据

		}
		if (logined_client_list.empty())
			cout << "无客户端连接" << endl;
	}
}

// 获取队列中的头元素
SOCKET_INT pop_list(SOCKET_LIST * object) {
	//WaitForSingleObject(listMutex, INFINITE);
	pthread_join(tid1,NULL);
	SOCKET_INT result;
	if (object->empty()) { //list为空
		result = -1; // fail
	}
	else {
		result = object->front();
		object->pop_front();
	}
	//ReleaseMutex(listMutex);
	pthread_mutex_unlock(&mutex);
	return result;
}
// push进去
void push_list(SOCKET_LIST * object, SOCKET_INT s) {
//	WaitForSingleObject(listMutex, INFINITE);
	pthread_join(tid1,NULL);
	object->push_back(s);
	//ReleaseMutex(listMutex);
	pthread_mutex_unlock(&mutex);
}
