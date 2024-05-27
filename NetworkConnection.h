#pragma once

#include "tSQueue.h"
#include "Message.h"

namespace olc::net {

	template <typename T>
	class Server;

	template <typename T>
	class NetworkConnection : public std::enable_shared_from_this<NetworkConnection<T>>
	{
	public:
		enum class Owner
		{
			server,
			client
		};
	public:


		// probably change for client
		NetworkConnection(Owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tSQueue<OwnedMessage<T>>& qIn)
			: m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
		{
			m_nOwnerType = parent;
			if (parent == Owner::server) {
				//client has to validate itself
				handshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
				handshakeCheck = scramble(handshakeOut);
			}
			else
			{
				handshakeOut = 0;
				handshakeIn = 0;
			}

		}

		virtual ~NetworkConnection(){}

		uint32_t getID() const {
			return id;
		}
		void setID(uint32_t uid) {
			this->id = uid;
		}

		//for server only
		void connectToClient(olc::net::Server<T>* server, uint32_t uid)
		{
			if (m_socket.is_open()) {
				id = uid;
				//send data to client in order to validate it
				writeValidation();

				//wait for client validation
				readValidation(server);
			}

		}
		//for client only
		//void connectToServer(const asio::ip::tcp::resolver::results_type& endpoints);

		void disconnect()
		{
			if (isConnected()) asio::post(m_asioContext, [this]() {m_socket.close(); });
		}


		bool isConnected() const
		{
			return m_socket.is_open();
		}

		void startListening(){}

		void send(const Message<T>& msg)
		{
			asio::post(m_asioContext, [this, msg]() {
				bool bWritingMessage = !m_qMessagesOut.empty();
				m_qMessagesOut.push_back(msg);
				if (!bWritingMessage) {
					writeHeader();
				}
				});
		}

	private:
		void writeHeader()
		{
			// If this function is called, we know the outgoing message queue must have 
		// at least one message to send. So allocate a transmission buffer to hold
		// the message, and issue the work - asio, send these bytes
			asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(MessageHeader<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					// asio has now sent the bytes - if there was a problem
					// an error would be available...
					if (!ec)
					{
						// ... no error, so check if the message header just sent also
						// has a message body...
						if (m_qMessagesOut.front().body.size() > 0)
						{
							// ...it does, so issue the task to write the body bytes
							writeBody();
						}
						else
						{
							// ...it didnt, so we are done with this message. Remove it from 
							// the outgoing message queue
							m_qMessagesOut.pop_front();

							// If the queue is not empty, there are more messages to send, so
							// make this happen by issuing the task to send the next header.
							if (!m_qMessagesOut.empty())
							{
								writeHeader();
							}
						}
					}
					else
					{
						std::cout << "[" << id << "] Write Header Fail.\n";
						m_socket.close();
					}
				});
		}

		void writeBody() {
			// If this function is called, a header has just been sent, and that header
		// indicated a body existed for this message. Fill a transmission buffer
		// with the body data, and send it!
			asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						// Sending was successful, so we are done with the message
						// and remove it from the queue
						m_qMessagesOut.pop_front();

						// If the queue still has messages in it, then issue the task to 
						// send the next messages' header.
						if (!m_qMessagesOut.empty())
						{
							writeHeader();
						}
					}
					else
					{
						// Sending failed, see WriteHeader() equivalent for description :P
						std::cout << "[" << id << "] Write Body Fail.\n";
						m_socket.close();
					}
				});
		}
		
		void readHeader() {
			// If this function is called, we are expecting asio to wait until it receives
			// enough bytes to form a header of a message. We know the headers are a fixed
			// size, so allocate a transmission buffer large enough to store it. In fact, 
			// we will construct the message in a "temporary" message object as it's 
			// convenient to work with.
			asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(MessageHeader<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						// A complete message header has been read, check if this message
						// has a body to follow...
						if (m_msgTemporaryIn.header.size > 0)
						{
							// ...it does, so allocate enough space in the messages' body
							// vector, and issue asio with the task to read the body.
							m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
							readBody();
						}
						else
						{
							// it doesn't, so add this bodyless message to the connections
							// incoming message queue
							addToIncomingMessageQueue();
						}
					}
					else
					{
						// Reading form the client went wrong, most likely a disconnect
						// has occurred. Close the socket and let the system tidy it up later.
						std::cout << "[" << id << "] Read Header Fail.\n";
						m_socket.close();
					}
				});
		}
		
		void readBody() {
			// If this function is called, a header has already been read, and that header
			// request we read a body, The space for that body has already been allocated
			// in the temporary message object, so just wait for the bytes to arrive...
			asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						// ...and they have! The message is now complete, so add
						// the whole message to incoming queue
						addToIncomingMessageQueue();
					}
					else
					{
						// As above!
						std::cout << "[" << id << "] Read Body Fail.\n";
						m_socket.close();
					}
				});
		}
		
		//full message received, add it to the incoming queue
		void addToIncomingMessageQueue() {
			//shove it in queue, converting it to an "owned message", by initialising with the shared pointer from this connection object
			if (m_nOwnerType == Owner::server) m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
			else m_qMessagesIn.push_back({ nullptr,m_msgTemporaryIn });
			//we must now prime the asio context to receive the net message. It will just sit and wait for bytes to arrive, and the message construction process repeats itself.
			readHeader();
		}

		uint64_t scramble(uint64_t nInput) {
			uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
			out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
			return out ^ 0xC0DEFACE12345678;
		}

		void writeValidation() {
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

		void readValidation(olc::net::Server<T>* server = nullptr) {
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

	protected:

		asio::ip::tcp::socket m_socket;

		asio::io_context& m_asioContext;

		tSQueue<Message<T>> m_qMessagesOut;

		tSQueue<OwnedMessage<T>>& m_qMessagesIn;

		Message<T> m_msgTemporaryIn;

		Owner m_nOwnerType = Owner::server;

		uint32_t id = 0;

		uint64_t handshakeOut = 0;
		uint64_t handshakeIn = 0;
		uint64_t handshakeCheck = 0;


	};




}

