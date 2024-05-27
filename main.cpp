#include <iostream>
#include "ServerInstance.h"
#include "NetworkIncludes.h"


int main()
{
	ServerInstance server(60000);
	server.start();

	while (1)
	{
		server.update(-1, true);
	}
	return 0;
}