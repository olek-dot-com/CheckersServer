#pragma once
#include "NetworkIncludes.h"

namespace olc::net {
	template <typename T>
	struct MessageHeader {
		T id{};
		uint32_t size = 0;
	};


	template <typename T>
	struct Message {
		
		// header and body of the message
		MessageHeader<T> header{};
		std::vector<uint8_t> body;


		// returns size of entire message packet in bytes
		size_t size() const {
			return body.size();
		}


		template <typename Type>
		friend Message<T>& operator << (Message<T>& msg, const Type& data) {
			// Check that the type of the data being pushed is trivially copyable
			static_assert(std::is_standard_layout<Type>::value, "Data is too complex to be pushed into vector");

			// Cache current size of vector, as this will be the point we insert the data
			size_t i = msg.body.size();

			// Resize the vector by the size of the data being pushed
			msg.body.resize(msg.body.size() + sizeof(Type));

			// Physically copy the data into the newly allocated vector space
			std::memcpy(msg.body.data() + i, &data, sizeof(Type));

			// Recalculate the message size
			msg.header.size = msg.size();

			// Return the target message so it can be "chained"
			return msg;
		}

		template <typename Type>
		friend Message<T>& operator >> (Message<T>& msg, Type& data)
		{
			// Check that the type of the data being pushed is trivially copyable
			static_assert(std::is_standard_layout<Type>::value, "Data is too complex to be pulled from vector");

			// Cache the location towards the end of the vector where the pulled data starts
			size_t i = msg.body.size() - sizeof(Type);

			// Physically copy the data from the vector into the user variable
			std::memcpy(&data, msg.body.data() + i, sizeof(Type));

			// Shrink the vector to remove read bytes, and reset end position
			msg.body.resize(i);

			// Recalculate the message size
			msg.header.size = msg.size();

			// Return the target message so it can be "chained"
			return msg;
		}

	};

  
	template <typename T>
	class NetworkConnection;

	template <typename T>
	struct OwnedMessage {
		std::shared_ptr<NetworkConnection<T>> remote = nullptr;
		Message<T> msg;

		friend std::ostream& operator << (std::ostream& os, const OwnedMessage<T>& msg) {
			os << msg.msg;
			return os;
		}
	};
}

