#include<iostream>
using namespace std;
#include"sys.h"
#include<signal.h>

class A
{
	public:
	A()
	{
		signal(SIGPIPE, SIG_IGN);
	}
};
A a;

Sys *ser;

void f(int s)
{
	delete ser;
	exit(0);
}

Redis *redis = new Redis;
int main()
{
	signal(SIGINT, f);
	srand(time(NULL));
	int port;
	cout << "please input port: ";
	cin >> port;
	cout << endl;
	ser = new Sys("127.0.0.1", port);
	ser->Run();
	//delete ser;
	return 0;
}