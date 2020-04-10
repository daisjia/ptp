#include"sys.h"
extern Redis *redis;
map<int, CliData*>Sys::clidata;
map<int, int> onlinefriend;
map<int, int>Sys::user;
Socket* Sys::_ser;

map<int, struct event*> myevents; 

void DealCli(int fd, short event, void* arg)
{
	struct sockaddr_in caddr = *(struct sockaddr_in*)arg;
	char message[1024] = { 0 };
	int res = recv(fd, message, 1024, 0);
	if (res <= 0)
	{
		/*
		map<int, int>::iterator it = Sys::user.find(fd);
		if (it != Sys::user.end())
		{
			Sys::clidata[it->second]->state = 0;
		}
		*/
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "client exit!" << endl;
		event_del(myevents[fd]);
		myevents.erase(fd);
		close(fd);
		string cmd = "hset user:" + to_string(onlinefriend[fd]) + " online " + to_string(0);
		redis->ExeCmd(cmd.data());
		onlinefriend.erase(fd);
	}
	else
	{
		Json::Value val;
		Json::Reader read;
		if (-1 == read.parse(message, val))
		{
			char DateTime[_DATETIME_SIZE];
			GetDateTime(DateTime);
			cout << "===Author: Qinlu===" << " " << DateTime << "json parse fail!" << endl;
			return;
		}
		int type = val["type"].asInt();
		
		switch (type)
		{
		case REGISTER: Register(fd, message, caddr);  break;
		case LOGIN: Login(fd, message, caddr); break;
		case GETFRIEND: GetFriend(fd, message, caddr); break;
		case GETIPPORT: GetIpPort(fd, message, caddr); break;
		case EXIT: Exit(fd, message, caddr); break;
		case SENDGROUP : SendGroup(message); break;
		default:
			break;
		}
	}
}

void Connect(int fd, short event, void* arg)
{
	Sys* sys = (Sys*)arg;
	struct sockaddr_in caddr;
	int clifd = Sys::_ser->Connect(caddr);
	if (clifd == -1)
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "connect fail!" << endl;
		return;
	}
	static int i = 0;
	struct event* ev = event_new(sys->clibase[i % 4], clifd, EV_READ | EV_PERSIST, DealCli, (void*)&caddr);
	myevents.insert(make_pair(clifd, ev));  //key val
	i++;
	if (ev == NULL)
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "event new fail!" << endl;
	}

	event_add(ev, NULL);
}

Sys::Sys(const char* ip, const int port)
{
	_ser = new Socket(ip, port);
	base = event_base_new();
	clibase.insert(make_pair(0, event_base_new()));
	clibase.insert(make_pair(1, event_base_new()));
	clibase.insert(make_pair(2, event_base_new()));
	clibase.insert(make_pair(3, event_base_new()));
	struct event* env = event_new(base, _ser->GetFd(), EV_READ | EV_PERSIST, Connect, this);
	event_add(env, NULL);
}

void Sys::Run()
{
	pthread_t id;
	for (int i = 0; i < 4; ++i)
	{
		pthread_create(&id, NULL, pthread_run, clibase[i]);
	}
	event_base_dispatch(base);
}

void* pthread_run(void*arg)
{
	int pair[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == -1)
	{
		cout << "socketpair create fail ==>> void TcpServer::GetSockPair()" << endl;
	}
	struct event_base* base = (struct event_base*)arg;
	struct event* env = event_new(base, pair[0], EV_READ | EV_PERSIST, Connect, NULL);
	event_add(env, NULL);
	event_base_dispatch(base);
}

void Register(int fd, string message, struct sockaddr_in& caddr)
{
	Json::Value val;
	Json::Value rsp;
	Json::Reader read;
	if (-1 == read.parse(message, val))
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "json parse fail!" << endl;
		return;
	}
	int userid = val["userid"].asInt();

	string cmd = "exists user:" + to_string(userid);
	int res = redis->Exist(cmd.data());

	// map<int, CliData*>::iterator it = Sys::clidata.find(userid);
	// if (it != Sys::clidata.end())
	if(res == 1)
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "  userid: " << userid << " register fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "用户已经存在";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	else if(res == -1)
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu==="<< " " << DateTime << "  userid: " << userid << " register fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "注册失败，服务器出错";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	int passwd = val["passwd"].asInt();
	string name = val["name"].asString();
	int cliport = (rand() % (20000 - 5000)) + 5000;
	cmd.clear();
	cmd = "hmset user:" + to_string(userid) + " id " + to_string(userid) + " name " + name + " passwd " + to_string(passwd) + " port " + to_string(cliport) + " online " + to_string(0);
	bool flag = redis->Insert(cmd.data());
	if(!flag)
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu==="<< " " << DateTime << "  userid: " << userid << " register fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "注册失败，服务器出错";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	rsp["res"] = true;
	//rsp["port"] = cliport;
	rsp["message"] = "注册成功";
	Sys::_ser->Send(fd, rsp.toStyledString());
	// CliData* data = new CliData();
	// data->name = name;
	// data->caddr = caddr;
	// data->caddr.sin_port = htons(cliport);
	// data->passwd = passwd;
	// data->state = 0;
	// data->userid = userid;
	// Sys::clidata.insert(make_pair(userid, data));
	char DateTime[_DATETIME_SIZE];
	GetDateTime(DateTime);
	cout << "===Author: Qinlu===" << " Time: " << DateTime << "  userid: "<<userid << " register success!" << endl;
}

void Login(int fd, string message, struct sockaddr_in& caddr)
{
	Json::Value val;
	Json::Reader read;
	Json::Value rsp;
	if (-1 == read.parse(message, val))
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "json parse fail!" << endl;
		return;
	}
	int userid = val["userid"].asInt();
	int pw = val["passwd"].asInt();
	string cmd = "exists user:" + to_string(userid);
	int res = redis->Exist(cmd.data());
	if(res == -1)
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu==="<< " " << DateTime << "  userid: " << userid << " Login fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "登录失败，服务器出错";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	if(res == 1)
	{
		cmd.clear();
		cmd = "hgetall user:" + to_string(userid);
		map<string, string> result;
		bool flag = redis->Query(cmd.data(), result);
		if(flag)
		{
			if(atoi(result["id"].data()) == userid && atoi(result["passwd"].data()) == pw)
			{
				if(atoi(result["online"].data()) == 1)
				{
					rsp["res"] = false;
					rsp["message"] = "用户已经登录成功，不可重复登录";
					Sys::_ser->Send(fd, rsp.toStyledString());
					return;
				}
				cmd.clear();
				cmd = "hset user:" + to_string(userid) + " online " + to_string(1);
				redis->ExeCmd(cmd.data());
				rsp["res"] = true;
				rsp["port"] = atoi(result["port"].data());
				rsp["name"] = result["name"];
				rsp["message"] = "登录成功";
				Sys::_ser->Send(fd, rsp.toStyledString());
				onlinefriend.insert(make_pair(fd, userid));
				return;
			}
			else
			{
				rsp["res"] = false;
				rsp["message"] = "登录失败，密码错误";
				Sys::_ser->Send(fd, rsp.toStyledString());
				return;
			}
			
		}
		else
		{
			char DateTime[_DATETIME_SIZE];
			GetDateTime(DateTime);
			cout << "===Author: Qinlu==="
				 << " " << DateTime << "  userid: " << userid << " Login fail!" << endl;
			rsp["res"] = false;
			rsp["message"] = "登录失败，服务器出错";
			Sys::_ser->Send(fd, rsp.toStyledString());
			return;
		}
		
	}
	else
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu==="<< " " << DateTime << "  userid: " << userid << " not exist!" << endl;
		rsp["res"] = false;
		rsp["message"] = "用户不存在";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	
	/*
	map<int, CliData*>::iterator it = Sys::clidata.find(userid);
	if (it == Sys::clidata.end())
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "  userid: " << userid << " not exist!" << endl;
		rsp["res"] = false;
		rsp["message"] = "user not exist!";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	int passwd = val["passwd"].asInt();
	if (passwd == it->second->passwd)
	{
		if (it->second->state == 1)	
		{
			rsp["res"] = false;
			rsp["message"] = "login fail, user alreadly online!";
			Sys::_ser->Send(fd, rsp.toStyledString());
			return;
		}
		it->second->state = 1;
		Sys::user.insert(make_pair(fd, userid));	
		rsp["res"] = true;
		rsp["message"] = "login success!";
		Sys::_ser->Send(fd, rsp.toStyledString());
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " Time: " << DateTime << "  userid: " << userid << " login success!" << endl;
	}
	else
	{
		rsp["res"] = false;
		rsp["message"] = "passwd error!";
		Sys::_ser->Send(fd, rsp.toStyledString());
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " Time: " << DateTime << "  userid: " << userid << " login fail!" << endl;
	}
	*/
}

void SendGroup(string message)
{
	Json::Value val;
	Json::Reader reader;
	if (-1 == reader.parse(message, val))
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu==="
			 << " " << DateTime << "json parse fail!" << endl;
		return;
	}

	int userid = val["id"].asInt();
	string name = val["name"].asString();
	vector<string> online;
	redis->Keys("keys user:*", online);
	for (int i = 0; i < online.size(); ++i)
	{
		string cmd = "hgetall " + online[i];
		map<string, string> result;
		bool flag = redis->Query(cmd.data(), result);
		if(!flag)
			continue;

		if(atoi(result["id"].data()) == userid || atoi(result["online"].data()) == 0)
			continue;

		int tmpfd = socket(AF_INET, SOCK_STREAM, 0);
		if(tmpfd <= 0)
			continue;
		sockaddr_in caddr;
		caddr.sin_family = AF_INET;
		caddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		caddr.sin_port = htons(atoi(result["port"].data()));
		int res = connect(tmpfd, (sockaddr *)&caddr, sizeof(caddr));
		if(res == -1)
		{
			cout << "222222222222222" << endl;
			continue;
		}
		cout << "111111111111111111" << endl;
		Json::Value req;
		req["type"] = MESSAGE;
		req["id"] = userid;
		req["name"] = name;
		req["message"] = val["message"].asString();
		send(tmpfd, req.toStyledString().data(), req.toStyledString().size(), 0);
		close(tmpfd);
	}
}

void GetFriend(int fd, string message, struct sockaddr_in& caddr)
{
	Json::Value val;
	Json::Reader read;
	Json::Value rsp;
	if (-1 == read.parse(message, val))
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "json parse fail!" << endl;
		return;
	}
	int userid = val["userid"].asInt();
	string data;

	vector<string> online;
	if(!redis->Keys("keys user:*", online))
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu==="
			 << " " << DateTime << "  userid: " << userid << " getfriend fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "获取好友失败，服务器出错";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}

	//for (; it != onlinefriend.end(); ++it)
	for (int i = 0; i < online.size(); ++i)
	{
		string cmd = "hgetall " + online[i];
		map<string, string> result;
		result.clear();
		bool flag = redis->Query(cmd.data(), result);
		if(flag)
		{
			if(atoi(result["online"].data()) == 0 || atoi(result["id"].data()) == userid)
				continue;

			string tmp = "用户名 : " + result["id"] + "    " + "姓名 : " + result["name"] +  "\n";
			data += tmp;
		}
		else
		{
			char DateTime[_DATETIME_SIZE];
			GetDateTime(DateTime);
			cout << "===Author: Qinlu==="<< " " << DateTime << "  userid: " << userid << " getfriend fail!" << endl;
			rsp["res"] = false;
			rsp["message"] = "获取好友失败，服务器出错";
			Sys::_ser->Send(fd, rsp.toStyledString());
			return;
		}
	}

	rsp["res"] = true;
	rsp["message"] = data.data();
	Sys::_ser->Send(fd, rsp.toStyledString());
	char DateTime[_DATETIME_SIZE];
	GetDateTime(DateTime);
	cout << "===Author: Qinlu==="<< " Time: " << DateTime << "  userid: " << userid << " getfriend success!" << endl;

	/*
	map<int, CliData *>::iterator it = Sys::clidata.find(userid);
	if (it == Sys::clidata.end())
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "  userid: " << userid << " getfriend fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "please register first!";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	if(it->second->state == 0)
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "  userid: " << userid << " getfriend fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "please login first!";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	
	char buff[1024 * 1024] = { 0 };
	it = Sys::clidata.begin();
	
	for (; it != Sys::clidata.end(); it++)
	{
		if (it->second->state == 1 && it->first != val["userid"].asInt())
		{
			char buff1[100] = { 0 };
			sprintf(buff1, "userid: %d   name: %s\n", it->second->userid, it->second->name.c_str());
			strcat(buff, buff1);	
		}
	}
	rsp["res"] = true;
	rsp["message"] = buff;
	Sys::_ser->Send(fd, rsp.toStyledString());
	char DateTime[_DATETIME_SIZE];
	GetDateTime(DateTime);
	cout << "===Author: Qinlu===" << " Time: " << DateTime << "  userid: " << userid << " getfriend success!" << endl;
	*/
}

void GetIpPort(int fd, string message, struct sockaddr_in& caddr)
{
	Json::Value val;
	Json::Reader read;
	Json::Value rsp;
	if (-1 == read.parse(message, val))
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "json parse fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "请求消息错误";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	int userid = val["userid"].asInt();
	int friendid = val["friendid"].asInt();
	if(userid == friendid)
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu==="
			 << " " << DateTime << "  userid: " << userid << " GetIpPort fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "不能给自己发消息";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	
	string cmd = "exists user:" + to_string(friendid);
	int res = redis->Exist(cmd.data());
	if(res == -1)
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu==="<< " " << DateTime << "  userid: " << userid << " GetIpPort fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "服务器出错";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	else if(res == 0)
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "  userid: " << userid << " GetIpPort fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "您的好友不存在";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	else if(res == 1)
	{
		cmd.clear();
		cmd = "hgetall user:" + to_string(friendid);
		map<string, string> result;
		bool flag = redis->Query(cmd.data(), result);
		if(flag)
		{
			if(atoi(result["online"].data()) == 0)
			{
				char DateTime[_DATETIME_SIZE];
				GetDateTime(DateTime);
				cout << "===Author: Qinlu==="<< " " << DateTime << "  userid: " << userid << " GetIpPort fail!" << endl;
				rsp["res"] = false;
				rsp["message"] = "您的好友不在线";
				Sys::_ser->Send(fd, rsp.toStyledString());
				return;
			}
			rsp["res"] = true;
			rsp["userid"] = friendid;
			rsp["name"] = result["name"].data();
			rsp["ip"] = "127.0.0.1";
			rsp["port"] = atoi(result["port"].data()); //htons

			Sys::_ser->Send(fd, rsp.toStyledString());
			char DateTime[_DATETIME_SIZE];
			GetDateTime(DateTime);
			cout << "===Author: Qinlu==="<< " Time: " << DateTime << "  userid: " << userid << " GetIpPort success!" << endl;
			return;
		}
		else
		{
			char DateTime[_DATETIME_SIZE];
			GetDateTime(DateTime);
			cout << "===Author: Qinlu==="<< " " << DateTime << "  userid: " << userid << " GetIpPort fail!" << endl;
			rsp["res"] = false;
			rsp["message"] = "服务器出错";
			Sys::_ser->Send(fd, rsp.toStyledString());
			return;
		}
		
	}

	/*
		map<int, CliData *>::iterator it = Sys::clidata.find(userid);
	if (it == Sys::clidata.end())
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "  userid: " << userid << " GetIpPort fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "please register first!";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}
	if (it->second->state == 0)
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "  userid: " << userid << " GetIpPort fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "please login first!";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}

	it = Sys::clidata.find(friendid);
	if (it == Sys::clidata.end())
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "  userid: " << userid << " GetIpPort fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "your friend not exist!";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}

	if (it->second->state == 0)
	{
		char DateTime[_DATETIME_SIZE];
		GetDateTime(DateTime);
		cout << "===Author: Qinlu===" << " " << DateTime << "  userid: " << userid << " GetIpPort fail!" << endl;
		rsp["res"] = false;
		rsp["message"] = "your friend not online!";
		Sys::_ser->Send(fd, rsp.toStyledString());
		return;
	}

	rsp["res"] = true;
	rsp["userid"] = it->second->userid;
	rsp["name"] = it->second->name;
	rsp["ip"] = inet_ntoa(it->second->caddr.sin_addr);
	rsp["port"] = ntohs(it->second->caddr.sin_port);  //htons

	Sys::_ser->Send(fd, rsp.toStyledString());
	char DateTime[_DATETIME_SIZE];
	GetDateTime(DateTime);
	cout << "===Author: Qinlu===" << " Time: " << DateTime << "  userid: " << userid << " GetIpPort success!" << endl;

	*/

}

void Exit(int fd, string message, struct sockaddr_in& caddr)
{


}