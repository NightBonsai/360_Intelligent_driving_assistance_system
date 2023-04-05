#include <iostream>
#include "EpollServer.h"

using namespace std;

int main()
{
	EpollServer* epoll = new EpollServer(10086);
	epoll->epollWork();

	return 0;
}