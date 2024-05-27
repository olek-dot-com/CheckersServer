#include "NetworkConnection.h"


//template<typename T>
//olc::net::NetworkConnection<T>::
//
//template<typename T>
//olc::net::NetworkConnection<T>::~NetworkConnection()
//{
//
//}
//
//template<typename T>
//uint32_t olc::net::NetworkConnection<T>::getID() const
//{
//	return id;
//}

//template<typename T>
//void olc::net::NetworkConnection<T>::connectToClient(olc::net::Server<T>* server, uint32_t uid)
//{
//	if (m_socket.is_open()) {
//		id = uid;
//		//send data to client in order to validate it
//		writeValidation();
//
//		//wait for client validation
//		readValidation(server);
//	}
//
//}

//template<typename T>
//void olc::net::NetworkConnection<T>::connectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
//{
//	// request asio attempts to connect to an endpoint
//	asio::async_connect(m_socket, endpoints,
//		[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
//		{
//			if (!ec) readValidation;
//		});
//}

template<typename T>
void olc::net::NetworkConnection<T>::disconnect()
{
	if (isConnected()) asio::post(m_asioContext, [this]() {m_socket.close(); });
}

template<typename T>
bool olc::net::NetworkConnection<T>::isConnected() const
{
	return m_socket.is_open();
}

template<typename T>
void olc::net::NetworkConnection<T>::startListening()
{
}

template<typename T>
void olc::net::NetworkConnection<T>::send(const Message<T>& msg)
{
	asio::post(m_asioContext, [this, msg]() {
		bool bWritingMessage = !m_qMessagesOut.empty();
		m_qMessagesOut.push_back(msg);
		if (!bWritingMessage) {
			writeHeader();
		}
		});
}

//template<typename T>
//void olc::net::NetworkConnection<T>::writeHeader()
//{
//	// If this function is called, we know the outgoing message queue must have 
//// at least one message to send. So allocate a transmission buffer to hold
//// the message, and issue the work - asio, send these bytes
//	asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(MessageHeader<T>)),
//		[this](std::error_code ec, std::size_t length)
//		{
//			// asio has now sent the bytes - if there was a problem
//			// an error would be available...
//			if (!ec)
//			{
//				// ... no error, so check if the message header just sent also
//				// has a message body...
//				if (m_qMessagesOut.front().body.size() > 0)
//				{
//					// ...it does, so issue the task to write the body bytes
//					writeBody();
//				}
//				else
//				{
//					// ...it didnt, so we are done with this message. Remove it from 
//					// the outgoing message queue
//					m_qMessagesOut.pop_front();
//
//					// If the queue is not empty, there are more messages to send, so
//					// make this happen by issuing the task to send the next header.
//					if (!m_qMessagesOut.empty())
//					{
//						writeHeader();
//					}
//				}
//			}
//			else
//			{
//				std::cout << "[" << id << "] Write Header Fail.\n";
//				m_socket.close();
//			}
//		});
//}

//template<typename T>
//void olc::net::NetworkConnection<T>::writeBody()
//{
//	// If this function is called, a header has just been sent, and that header
//// indicated a body existed for this message. Fill a transmission buffer
//// with the body data, and send it!
//	asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
//		[this](std::error_code ec, std::size_t length)
//		{
//			if (!ec)
//			{
//				// Sending was successful, so we are done with the message
//				// and remove it from the queue
//				m_qMessagesOut.pop_front();
//
//				// If the queue still has messages in it, then issue the task to 
//				// send the next messages' header.
//				if (!m_qMessagesOut.empty())
//				{
//					writeHeader();
//				}
//			}
//			else
//			{
//				// Sending failed, see WriteHeader() equivalent for description :P
//				std::cout << "[" << id << "] Write Body Fail.\n";
//				m_socket.close();
//			}
//		});
//}
//
//template<typename T>
//void olc::net::NetworkConnection<T>::readHeader()
//{
//	// If this function is called, we are expecting asio to wait until it receives
//	// enough bytes to form a header of a message. We know the headers are a fixed
//	// size, so allocate a transmission buffer large enough to store it. In fact, 
//	// we will construct the message in a "temporary" message object as it's 
//	// convenient to work with.
//	asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(MessageHeader<T>)),
//		[this](std::error_code ec, std::size_t length)
//		{
//			if (!ec)
//			{
//				// A complete message header has been read, check if this message
//				// has a body to follow...
//				if (m_msgTemporaryIn.header.size > 0)
//				{
//					// ...it does, so allocate enough space in the messages' body
//					// vector, and issue asio with the task to read the body.
//					m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
//					readBody();
//				}
//				else
//				{
//					// it doesn't, so add this bodyless message to the connections
//					// incoming message queue
//					addToIncomingMessageQueue();
//				}
//			}
//			else
//			{
//				// Reading form the client went wrong, most likely a disconnect
//				// has occurred. Close the socket and let the system tidy it up later.
//				std::cout << "[" << id << "] Read Header Fail.\n";
//				m_socket.close();
//			}
//		});
//}
//
//template<typename T>
//void olc::net::NetworkConnection<T>::readBody()
//{
//	// If this function is called, a header has already been read, and that header
//	// request we read a body, The space for that body has already been allocated
//	// in the temporary message object, so just wait for the bytes to arrive...
//	asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
//		[this](std::error_code ec, std::size_t length)
//		{
//			if (!ec)
//			{
//				// ...and they have! The message is now complete, so add
//				// the whole message to incoming queue
//				addToIncomingMessageQueue();
//			}
//			else
//			{
//				// As above!
//				std::cout << "[" << id << "] Read Body Fail.\n";
//				m_socket.close();
//			}
//		});
//}

template<typename T>
void olc::net::NetworkConnection<T>::addToIncomingMessageQueue()
{
	//shove it in queue, converting it to an "owned message", by initialising with the shared pointer from this connection object
	if (m_nOwnerType == Owner::server) m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
	else m_qMessagesIn.push_back({ nullptr,m_msgTemporaryIn });
	//we must now prime the asio context to receive the net message. It will just sit and wait for bytes to arrive, and the message construction process repeats itself.
	readHeader();
}

template<typename T>
uint64_t olc::net::NetworkConnection<T>::scramble(uint64_t nInput)
{
	uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
	out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
	return out ^ 0xC0DEFACE12345678;
}

template<typename T>
void olc::net::NetworkConnection<T>::writeValidation()
{
	asio::async_write(m_socket, asio::buffer(&handshakeOut, sizeof(uint64_t)),
		[this](std::error_code ec, std::size_t length) {
			if (!ec) {
				if (m_nOwnerType == Owner::client) readHeader();
			}
			else
			{
				m_socket.close();
			}
		});
}

template<typename T>
void olc::net::NetworkConnection<T>::readValidation(olc::net::Server<T>* server)
{
	asio::async_read(m_socket, asio::buffer(&handshakeIn, sizeof(uint64_t)),
		[this, server](std::error_code ec, std::size_t length) {
			if (!ec) {
				if (m_nOwnerType == Owner::server) {
					if (handshakeIn == handshakeCheck) {
						std::cout << "Client Validated" << std::endl;
						server->onClientValidated(this->shared_from_this());

						readHeader();
					}
					else {
						std::cout << "Client Disconnected (validation failed)\n";
						m_socket.close();
					}
				}
				else
				{
					//Client is connecting
					handshakeOut = scramble(handshakeIn);
					writeValidation();
				}
			}
			else {
				std::cout << "client disconnected" << std::endl;
				m_socket.close();
			}
		});
}
