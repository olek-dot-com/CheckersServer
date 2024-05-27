#include "Server.h"

template<typename T>
void olc::net::Server<T>::onClientDisconnect(std::shared_ptr<NetworkConnection<T>> client)
{
}

template<typename T>
void olc::net::Server<T>::onMessage(std::shared_ptr<NetworkConnection<T>> client, Message<T>& msg)
{
}

//template<typename T>
//olc::net::Server<T>::Server(uint16_t port)
//	: m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
//{}
//
//template<typename T>
//olc::net::Server<T>::~Server()
//{
//	stop();
//}
//
//template<typename T>
//bool olc::net::Server<T>::start()
//{
//	try
//	{
//		// Issue a task to the asio context - This is important
//		// as it will prime the context with "work", and stop it
//		// from exiting immediately. Since this is a server, we 
//		// want it primed ready to handle clients trying to
//		// connect.
//		waitForClientConnection();
//
//		// Launch the asio context in its own thread
//		m_threadContext = std::thread([this]() { m_asioContext.run(); });
//	}
//	catch (std::exception& e)
//	{
//		// Something prohibited the server from listening
//		std::cerr << "[SERVER] Exception: " << e.what() << "\n";
//		return false;
//	}
//
//	std::cout << "[SERVER] Started!\n";
//	return true;
//}

template<typename T>
void olc::net::Server<T>::stop()
{
	// Request the context to close
	m_asioContext.stop();

	// Tidy up the context thread
	if (m_threadContext.joinable()) m_threadContext.join();

	// Inform someone, anybody, if they care...
	std::cout << "[SERVER] Stopped!\n";
}

template<typename T>
void olc::net::Server<T>::waitForClientConnection()

{
	// Prime context with an instruction to wait until a socket connects. This
	// is the purpose of an "acceptor" object. It will provide a unique socket
	// for each incoming connection attempt
	m_asioAcceptor.async_accept(
		[this](std::error_code ec, asio::ip::tcp::socket socket)
		{
			// Triggered by incoming connection request
			if (!ec)
			{
				// Display some useful(?) information
				std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";

				// Create a new connection to handle this client 
				std::shared_ptr<NetworkConnection<T>> newconn =
					std::make_shared<NetworkConnection<T>>(NetworkConnection<T>::Owner::server,
						m_asioContext, std::move(socket), m_qMessagesIn);



				// Give the user server a chance to deny connection
				if (onClientConnect(newconn))
				{
					// Connection allowed, so add to container of new connections
					m_deqConnections.push_back(std::move(newconn));

					// And very important! Issue a task to the connection's
					// asio context to sit and wait for bytes to arrive!
					m_deqConnections.back()->connectToClient(this, nIDCounter++);

					std::cout << "[" << m_deqConnections.back()->getID() << "] Connection Approved\n";
				}
				else
				{
					std::cout << "[-----] Connection Denied\n";

					// Connection will go out of scope with no pending tasks, so will
					// get destroyed automagically due to the wonder of smart pointers
				}
			}
			else
			{
				// Error has occurred during acceptance
				std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
			}

			// Prime the asio context with more work - again simply wait for
			// another connection...
			waitForClientConnection();
		});
}

template<typename T>
void olc::net::Server<T>::messageClient(std::shared_ptr<NetworkConnection<T>> client, const Message<T>& msg)
{
	// Check client is legitimate...
	if (client && client->IsConnected())
	{
		// ...and post the message via the connection
		client->Send(msg);
	}
	else
	{
		// If we cant communicate with client then we may as 
		// well remove the client - let the server know, it may
		// be tracking it somehow
		onClientDisconnect(client);

		// Off you go now, bye bye!
		client.reset();

		// Then physically remove it from the container
		m_deqConnections.erase(
			std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());
	}
}

template<typename T>
void olc::net::Server<T>::messageAllClients(const Message<T>& msg, std::shared_ptr<NetworkConnection<T>> pIgnoreClient)

{
	bool bInvalidClientExists = false;

	// Iterate through all clients in container
	for (auto& client : m_deqConnections)
	{
		// Check client is connected...
		if (client && client->isConnected())
		{
			// ..it is!
			if (client != pIgnoreClient)
				client->send(msg);
		}
		else
		{
			// The client couldnt be contacted, so assume it has
			// disconnected.
			onClientDisconnect(client);
			client.reset();

			// Set this flag to then remove dead clients from container
			bInvalidClientExists = true;
		}
	}

	// Remove dead clients, all in one go - this way, we dont invalidate the
	// container as we iterated through it.
	if (bInvalidClientExists)
		m_deqConnections.erase(
			std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
}

template<typename T>
void olc::net::Server<T>::update(size_t nMaxMessages, bool bWait)

{
	if (bWait) m_qMessagesIn.wait();

	// Process as many messages as you can up to the value
	// specified
	size_t nMessageCount = 0;
	while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
	{
		// Grab the front message
		auto msg = m_qMessagesIn.pop_front();

		// Pass to message handler
		onMessage(msg.remote, msg.msg);

		nMessageCount++;
	}
}




