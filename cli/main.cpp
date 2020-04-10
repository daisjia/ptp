#include"client.h"
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

int main()
{
	int port;
	cout << "please input port: ";
	cin >> port;
	Client* cli = new Client("127.0.0.1", port);
	cli->Run();
	return 0;
}

