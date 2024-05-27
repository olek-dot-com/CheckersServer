#pragma once
#include "NetworkIncludes.h"

enum class CustomMsgTypes : uint8_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};


class ServerInstance : public olc::net::Server<CustomMsgTypes> {
private:
	uint32_t nextClientID = 1; // Variable to keep track of the next available client ID

public:
	ServerInstance(uint16_t nPort) : olc::net::Server<CustomMsgTypes>(nPort)
	{

	}
	

protected:
	virtual bool onClientConnect(std::shared_ptr<olc::net::NetworkConnection<CustomMsgTypes>> client)
	{
		olc::net::Message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;

		// Assign a unique ID to the client
		client->setID(nextClientID);
		nextClientID++;

		std::cout << "Client [" << client->getID() << "] connected\n";

		if (client->getID() == 1) {
			std::cout << "Client 1 connected\n";
			msg << (uint8_t)1;
			std::cout << "sending 1\n";
		}
		else {
			msg << (uint8_t)0;
			std::cout << "sending 0\n";
		}

		client->send(msg);
		return true;
	}

	// Called when a client appears to have disconnected
	virtual void onClientDisconnect(std::shared_ptr<olc::net::NetworkConnection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->getID() << "]\n";
	}

	// Called when a message arrives
	virtual void onMessage(std::shared_ptr<olc::net::NetworkConnection<CustomMsgTypes>> client, olc::net::Message<CustomMsgTypes>& msg)
	{
		switch (msg.header.id)
		{
			case CustomMsgTypes::ServerPing:
			{
				std::cout << "[" << client->getID() << "]: Server Ping\n";

				// Simply bounce message back to client

				msg.header.id = CustomMsgTypes::ServerMessage;
				messageAllClients(msg, client);
				std::cout << "done\n";
				break;
			}
			case CustomMsgTypes::MessageAll:
			{
				std::cout << "[" << client->getID() << "]: Message All\n";

				// Construct a new message and send it to all clients
				olc::net::Message<CustomMsgTypes> tempMsg;
				tempMsg.header.id = CustomMsgTypes::ServerMessage;
				msg.header.id = CustomMsgTypes::ServerMessage;
				uint8_t a, b, c, d, e, f;
				msg >> a >> b >> c >> d >> e /*>> f*/;
				std::cout << (int)a << (int)b << (int)c << (int)d << (int)e <</* (int)f <<*/ std::endl;
				tempMsg << a << b << c << d << e/* << f*/;
				messageAllClients(tempMsg, client);
				std::cout << "doneMsgAll\n";
				break;
			}
		}
	}
};
