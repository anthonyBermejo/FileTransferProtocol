#include "Thread.h"

class TcpThread :public Thread
{

	int cs;
public:
	TcpThread(int clientsocket) :cs(clientsocket)
	{}
	virtual void run();
};