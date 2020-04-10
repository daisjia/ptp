#include "client.h"
using namespace std;

map<int, struct event*> myevents;	//存储事件，用来断开连接删除事件的
void f(int fd, short event, void *arg){}
void* pthread_run(void* arg)
{
	int pair[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == -1)
	{
		cout << "socketpair create fail ==>> void TcpServer::GetSockPair()" << endl;
	}
	struct event_base* base = (struct event_base*)arg;
	struct event* env = event_new(base, pair[0], EV_READ | EV_PERSIST, f, NULL);
	event_add(env, NULL);
	event_base_dispatch(base);
}

void Deal(int fd, short event, void *arg)
{
	Client *mythis = (Client *)arg;
	char message[1024 * 1024] = { 0 };
	int res = recv(fd, message, 1024 * 1024, 0);
	if (res <= 0)
	{
		map<int, struct event*>::iterator it = myevents.find(fd);
		if(it != myevents.end())
		{
			event_del(myevents[fd]);
			myevents.erase(fd);
			close(fd);
		}

		map<int, int>::iterator itt = mythis->myfriend.begin();
		for (; itt != mythis->myfriend.end(); itt++)
		{
			if(fd == itt->second)
			{
				close(itt->first);
				mythis->myfriend.erase(itt);
				break;
			}
		}
		return;
	}

	Json::Value val;
	Json::Reader reader;
	reader.parse(message, val);
	if (val["type"] == CONNECT)
	{
		mythis->myfriend.insert(make_pair(val["id"].asInt(), fd));
		PutMenu();
		return;
	}
	else if (val["type"].asInt() == MESSAGE)
	{
		cout << endl;
		cout << "\033[35m->[userid] : "<<val["id"].asInt() <<"\033[0m" << endl;
		cout << "\033[35m->[name] : " << val["name"].asString().data() << "\033[0m" << endl;
		cout << "\033[35m->[message] : " << val["message"].asString().data() << "\033[0m" << endl;
	}
	else if(val["type"].asInt() == SENDFILES)
	{
		cout << endl;
		cout << "\033[35m->[userid] : " << val["id"].asInt() << "\033[0m" << endl;
		cout << "\033[35m->[name] : " << val["name"].asString().data() << "\033[0m" << endl;

		string filename = val["filename"].asString();
		int filefd = open(filename.data(), O_RDWR | O_CREAT | O_TRUNC, 0666);
		if(filefd == -1)
		{
			cout << "接收文件失败" << endl;
			PutMenu();
			return;
		}
		int ret = write(filefd, val["message"].asString().data(), val["message"].asString().size());
		close(filefd);
		if(ret <= 0)
		{
			cout << "接收文件失败" << endl;
			PutMenu();
			return;
		}
		cout << "\033[35m->[message] : 文件: " << filename << " 接收成功"
			 << "\033[0m" << endl;
	}
	PutMenu();
}

void Connect(int fd, short event, void *arg)
{
	Client* cli = (Client*)arg;
	struct sockaddr_in caddr;
	socklen_t len = sizeof(caddr);
	int clifd = accept(fd, (struct sockaddr*) & caddr, &len);
	if (clifd == -1)
	{
		cout << "accept fail!" << endl;
		return;
	}

	struct event* ev = event_new(cli->base, clifd, EV_READ | EV_PERSIST, Deal, (void*)cli);

	if (ev == NULL)
	{
		cout<<"event new fail!" << endl;
		return;
	}
	myevents.insert(make_pair(clifd, ev)); //将fd ev 事件保存起来，以后方便断开连接移除
	event_add(ev, NULL);
}

Client::Client(string ip, int port)
{
	myip = ip;
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_addr.s_addr = inet_addr(ip.c_str());
	saddr.sin_port = htons(port);
	saddr.sin_family = AF_INET;
	clifd = socket(AF_INET, SOCK_STREAM, 0);
	myfd = socket(AF_INET, SOCK_STREAM, 0);

	int res = connect(clifd, (struct sockaddr*) & saddr, sizeof(saddr));
	if (res == -1)
	{
		cout << "connect error!" << endl;
		exit(0);
	}
	base = event_base_new();
	pthread_t id;
	pthread_create(&id, NULL, pthread_run, base);
}

void Client::Run()
{
	while (1)
	{
		// cout << "     ==========================" << endl;
		// cout << "     *..... REGISTER : 1 .....*" << endl;
		// cout << "     *....... LOGIN: 2 .......*" << endl;
		// cout << "     *....... EXIT: 3  .......*" << endl;
		// cout << "     ==========================" << endl;

		time_t nSeconds;
		struct tm *pTM;
		time(&nSeconds);
		pTM = localtime(&nSeconds);
		char Buffer[100] = {0};
		sprintf(Buffer, "*--------%04d-%02d-%02d %02d:%02d:%02d---------*", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);

		std::cout << "\033[36m            ------------\033[0m" << std::endl;
		std::cout << "\033[36m            --菜    单--\033[0m" << std::endl;
		std::cout << "\033[36m            ------------\033[0m" << std::endl;
		std::cout << "\033[36m" << Buffer << "\033[0m" << std::endl;
		std::cout << "\033[36m*------------------------------------*\033[0m" << std::endl;
		std::cout << "\033[36m*----------- 注    册 : 1 -----------*\033[0m" << std::endl;
		std::cout << "\033[36m*----------- 登    录 : 2 -----------*\033[0m" << std::endl;
		std::cout << "\033[36m*----------- 退    出 : 3 -----------*\033[0m" << std::endl;
		std::cout << "\033[36m*------------------------------------*\033[0m" << std::endl;
		int choice = 0;
		cout << "-->请输入选择: ";

		cin >> choice;
		if (std::cin.fail())
		{
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();
			std::cout << "\033[31m[提  示]: 输入错误\033[0m" << std::endl;
			continue;
		}

		if (choice == 1)
		{
			Register();
		}
		else if (choice == 2)
		{
			Login();
		}
		else if (choice == 3)
		{
			return ;
		}
		else
		{
			cout << "\033[31m[提  示]: 输入错误\033[0m" << endl;
		}
	}
}

void Client::Register()
{
	int userid;
	string name;
	int passwd;
	cout << "-->请输入用户名: ";
	cin >> userid;
	cout << "-->请输入姓名: ";
	cin >> name;
	cout << "-->请输入密码: ";
	cin >> passwd;
	myid = userid;
	myname = name;
	mypasswd = passwd;
	Json::Value val;
	Json::Reader read;
	val["userid"] = userid;
	val["type"] = REGISTER;
	val["name"] = name;
	val["passwd"] = passwd;
	int res = send(clifd, val.toStyledString().c_str(), val.toStyledString().size(), 0);
	if(res <= 0)
	{
		cout << "服务器退出" << endl;
		sleep(1);
		exit(0);
	}
	char buff[1024] = { 0 };
	res = recv(clifd, buff, 1023, 0);
	if (res <= 0)
	{
		cout << "服务器退出" << endl;
		sleep(1);
		exit(0);
	}
	string message = buff;
	Json::Value rsp;
	if (-1 == read.parse(message, rsp))
	{
		cout << "json parse error!" << endl;
	}
	if (res == -1)
	{
		cout << "recv error!" << endl;
		return;
	}

	if (rsp["res"].asBool())
	{
		// myport = rsp["port"].asInt();
		// struct sockaddr_in saddr;
		// memset(&saddr, 0, sizeof(saddr));
		// saddr.sin_addr.s_addr = inet_addr(myip.c_str());
		// saddr.sin_port = htons(myport);
		// saddr.sin_family = AF_INET;
		// res = bind(myfd, (struct sockaddr*) & saddr, sizeof(sockaddr));
		// if (res == -1)
		// {
		// 	cout << "bind error!" << endl;
		// 	return;
		// }
		// struct event* env = event_new(base, myfd, EV_READ | EV_PERSIST, Connect, this);
		// listen(myfd, 20);
		// event_add(env, NULL);
		cout << "\033[34m[提  示]: " << rsp["message"].asString()<< "\033[0m" << endl;
	}
	else
	{
		cout << "\033[34m[提  示]: " << rsp["message"].asString()<< "\033[0m" << endl;
	}
}

void Client::Login()
{
	int userid;
	int passwd;
	cout << "-->请输入用户名: ";
	cin >> userid;
	cout << "-->请输入密码: ";
	cin >> passwd;
	Json::Value val;
	val["userid"] = userid;
	val["type"] = LOGIN;
	val["passwd"] = passwd;
	int res = send(clifd, val.toStyledString().c_str(), val.toStyledString().size(), 0);
	if(res <= 0)
	{
		cout << "服务器退出" << endl;
		sleep(1);
		exit(0);
	}
	char buff[1024] = { 0 };
	res = recv(clifd, buff, 1023, 0);
	if (res <= 0)
	{
		cout << "服务器退出" << endl;
		sleep(1);
		exit(0);
	}
	string str = buff;
	Json::Value rsp;
	Json::Reader read;
	read.parse(str, rsp);

	if (rsp["res"].asBool())
	{
		myport = rsp["port"].asInt();
		struct sockaddr_in saddr;
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_addr.s_addr = inet_addr(myip.c_str());
		saddr.sin_port = htons(myport);
		saddr.sin_family = AF_INET;
		res = bind(myfd, (struct sockaddr *)&saddr, sizeof(sockaddr));
		if (res == -1)
		{
			cout << "bind error!" << endl;
			exit(0);
		}
		struct event *env = event_new(base, myfd, EV_READ | EV_PERSIST, Connect, this);
		listen(myfd, 20);
		event_add(env, NULL);
		
		cout << "\033[34m[提  示]: " << rsp["message"].asString() << "\033[0m" << endl;
		myid = userid;
		myname = rsp["name"].asString();
		mypasswd = passwd;
		Start();
	}
	else
	{
		cout << "\033[34m[提  示]: " << rsp["message"].asString() << "\033[0m" << endl;
	}
}

void Client::Start()
{
	while (1)
	{
		PutMenu();
		int choice;
		std::string str;
		cin >> choice;
		if (std::cin.fail())
		{
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();
			std::cout << "\033[31m[提  示]: 输入错误\033[0m" << std::endl;
			continue;
		}

		if (choice == 1)
		{
			GetOnlineFriend();
		}
		else if (choice == 2)
		{
			SendMessage();
		}
		else if(choice == 3)
		{
			SendFiles();
		}
		else if(choice == 4)
		{
			SendGroup();
		}
		else if (choice == 5)
		{
			return;
		}
		else
		{
			std::cout << "\033[31m[提  示]: 输入错误\033[0m" << std::endl;
		}
	}
}

void Client::GetOnlineFriend()
{
	Json::Value val;
	val["userid"] = myid;
	val["type"] = GETFRIEND;
	int res = send(clifd, val.toStyledString().c_str(), val.toStyledString().size(), 0);
	if(res <= 0)
	{
		cout << "服务器退出" << endl;
		sleep(1);
		exit(0);
	}
	char buff[1024] = { 0 };
	res = recv(clifd, buff, 1023, 0);
	if (res <= 0)
	{
		cout << "服务器退出" << endl;
		sleep(1);
		exit(0);
	}
	Json::Value rsp;
	Json::Reader read;
	string str = buff;
	read.parse(buff, rsp);
	
	if (rsp["res"].asBool())
	{
		cout<<endl;
		cout<<"*------------------------------------*"<<endl;
		cout << "\033[36m" << rsp["message"].asString() << "\033[0m";
		cout<<"*------------------------------------*"<<endl;
		cout<<endl;
	}
	else
	{
		cout << "\033[34m[提  示]: " << rsp["message"].asString() << "\033[0m" << endl;
	}
}

void Client::SendFiles()
{
	GetOnlineFriend();
	int friendid;
	cout << "-->你要发送好友的用户名: ";
	cin >> friendid;
	if (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();
		std::cout << "\033[31m[提  示]: 输入错误\033[0m" << std::endl;
		return;
	}
	if(myid == friendid)
	{
		std::cout << "\033[31m[提  示]: 不能给自己发送文件\033[0m" << std::endl;
		return;
	}

	map<int, int>::iterator it = myfriend.find(friendid);
	int fd = 0;
	if (it == myfriend.end())
	{
		fd = GetSockFd(friendid);
		if (fd == -1)
		{
			return;
		}
	}
	else
	{
		fd = it->second;
	}

	char buff[1024 * 1024] = {0};
	string mes;
	string file;
	cout << "-->请输入文件路径+文件名: ";
	cin >> file;

	int i = 0;
	for (i = file.size() - 1; i >= 0; --i)
	{
		if(file[i] == '/')
			break;
	}
	string filename = file.substr(i + 1);

	FILE *fp = fopen(file.data(), "r");
	if(fp == NULL)
	{
		cout << "文件不存在，打开文件失败";
		return;
	}

	string Buffer;
	while(!feof(fp))
	{
		char buff[100] = {0};
		fgets(buff, 100, fp);
		Buffer.append(buff);
	}
	fclose(fp);
	/*
	int filefd = open(file.data(), O_RDWR);
	cout << "fd : " << filefd << endl;
	if(filefd == -1)
	{
		cout << "文件不存在，打开文件失败";
		return;
	}

	int len = lseek(filefd, 0, SEEK_END);
	cout << "len : " << len << endl;
	lseek(filefd, 0, SEEK_SET);
	char *Buffer = new char[len + 1];
	memset(Buffer, 0, len + 1);
	int ret = read(fd, Buffer, len);
	cout << "ret : " << ret << endl;
	cout << "errno : " << errno << endl;
	if(ret <= 0)
	{
		cout << "打开文件失败" << endl;
		close(filefd);
		delete Buffer;
		return;
	}
*/
	Json::Value req;
	req["type"] = SENDFILES;
	req["filename"] = filename;
	req["id"] = myid;
	req["name"] = myname;
	req["message"] = Buffer;
	int res = send(fd, req.toStyledString().c_str(), req.toStyledString().size(), 0);

	if (res <= 0)
	{
		map<int, struct event *>::iterator it = myevents.find(fd);
		if (it != myevents.end())
		{
			event_del(myevents[fd]);
			myevents.erase(fd);
		}
		if (myfriend[friendid] > 2)
		{
			close(myfriend[friendid]);
			myfriend.erase(friendid);
		}
	}
}

void Client::SendMessage()
{
	GetOnlineFriend();
	int friendid;
	cout << "-->你要发送好友的用户名: ";
	cin >> friendid;
	if (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();
		std::cout << "\033[31m[提  示]: 输入错误\033[0m" << std::endl;
		return;
	}

	if(myid == friendid)
	{
		std::cout << "\033[31m[提  示]: 不能给自己发消息\033[0m" << std::endl;
		return;
	}

	map<int, int>::iterator it = myfriend.find(friendid);
	int fd = 0;
	if (it == myfriend.end())
	{
		fd = GetSockFd(friendid);
		if (fd == -1)
		{
			return;
		}
	}
	else
	{
		fd = it->second;
	}
	
	string mes;
	cout << "-->发送消息内容: ";
	cin.get();
	getline(std::cin, mes);
	Json::Value req;
	req["type"] = MESSAGE;
	req["id"] = myid;
	req["name"] = myname;
	req["message"] = mes.data();
	int res = send(fd, req.toStyledString().c_str(), req.toStyledString().size(), 0);

	if (res <= 0)
	{
		map<int, struct event *>::iterator it = myevents.find(fd);
		if (it != myevents.end())
		{
			event_del(myevents[fd]);
			myevents.erase(fd);
		}
		if(myfriend[friendid] > 2)
		{
			close(myfriend[friendid]);
			myfriend.erase(friendid);
		}
	}
}

void Client::SendGroup()
{
	string mes;
	cout << "-->发送消息内容: ";
	cin.get();
	getline(std::cin, mes);
	Json::Value req;
	req["type"] = SENDGROUP;
	req["id"] = myid;
	req["name"] = myname;
	req["message"] = mes.data();
	int res = send(clifd, req.toStyledString().c_str(), req.toStyledString().size(), 0);
}

int Client::GetSockFd(int friendid) 
{
	Json::Value val;
	val["userid"] = myid;
	val["friendid"] = friendid;
	val["type"] = GETIPPORT;
	int res = send(clifd, val.toStyledString().c_str(), val.toStyledString().size(), 0);
	if(res <= 0)
	{
		cout << "服务器退出" << endl;
		sleep(1);
		exit(0);
	}
	char buff[1024 * 1024] = { 0 };
	res = recv(clifd, buff, 1023, 0);
	if (res <= 0)
	{
		cout << "服务器退出" << endl;
		sleep(1);
		exit(0);
	}

	string str = buff;
	//cout << endl;
	Json::Value rsp;
	Json::Reader read;
	read.parse(str, rsp);

	if (!rsp["res"].asBool())
	{
		std::cout << "\033[31m[提  示]: "<<rsp["message"].asString()<<"\033[0m" << std::endl;
		return -1;
	}
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_addr.s_addr = inet_addr(rsp["ip"].asString().c_str());
	saddr.sin_port = htons(rsp["port"].asInt());
	saddr.sin_family = AF_INET;
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	res = connect(fd, (struct sockaddr*) & saddr, sizeof(saddr));
	if (res == -1)
	{
		cout << "connect error!" << endl;
		return -1;
	}

	Json::Value v;
	v["type"] = CONNECT;
	v["id"] = myid;
	send(fd, v.toStyledString().data(), v.toStyledString().size()+1, 0);
	myfriend.insert(make_pair(friendid, fd));
	struct event* env = event_new(base, fd, EV_READ | EV_PERSIST, Deal, this);
	event_add(env, NULL);
	myevents.insert(make_pair(fd, env));
	return fd;
}

void PutMenu()
{
	time_t nSeconds;
	struct tm *pTM;
	time(&nSeconds);
	pTM = localtime(&nSeconds);
	char Buffer[100] = {0};
	sprintf(Buffer, "*--------%04d-%02d-%02d %02d:%02d:%02d---------*", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);

	std::cout << "\033[36m            ------------\033[0m" << std::endl;
	std::cout << "\033[36m            --菜    单--\033[0m" << std::endl;
	std::cout << "\033[36m            ------------\033[0m" << std::endl;
	std::cout << "\033[36m" << Buffer << "\033[0m" << std::endl;
	std::cout << "\033[36m*------------------------------------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- 在线好友 : 1 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- 发送消息 : 2 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- 传输文件 : 3 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- 群发消息 : 4 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- 退    出 : 5 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*------------------------------------*\033[0m" << std::endl;
	cout << "-->请输入选择: "<<flush;
}


