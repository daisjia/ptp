#pragma once
#include<vector>
#include<map>
#include<stdlib.h>
#include<memory.h>
#include<string>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<json/json.h>
#include<sys/types.h>
#include<event.h>
#include<json/json.h>
#include<iostream>
#include <fcntl.h>
#include<unistd.h>
#include"mytime.h"
#include<pthread.h>
using namespace std;
#include<functional>
#include<errno.h>
#include<sys/types.h>
#include<fcntl.h>

#define MESSAGE 100
#define CONNECT 200
#define SENDFILES 300
#define SENDGROUP 400

enum TYPE
{
	REGISTER,
	LOGIN,
	GETFRIEND,
	GETIPPORT,
	EXIT
};

void* pthread_run(void*);
//void Deal(int fd, short event, void* arg);
void Deal(int fd, short event, void *arg);
void Connect(int fd, short event, void *arg);
void PutMenu();

class Client
{
public:
	Client(string ip, int port);
	void Run();
	void Register();
	void Login();
	void GetOnlineFriend();
	void SendMessage();
	void SendFiles();
	void SendGroup();
	void Exit();
	void Start();
	int GetSockFd(int friendid);
	

private:
	int clifd;	//和服务器保持连接的
	int myfd;	//别人连接我的
	int myport;
	string myip;
	struct event_base* base;	
	friend void Connect(int fd, short event, void* arg);
	friend void Deal(int fd, short event, void *arg);
	int myid;
	string myname;
	int mypasswd;

	map<int, int> myfriend;	
};