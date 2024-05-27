#pragma once
#include "NetworkIncludes.h"

namespace olc::net {
	template <typename T>
	class tSQueue
	{
	protected:
		std::mutex muxQueue;
		std::deque<T> deqQueue;
		std::condition_variable cvBlock;
		std::mutex muxBlock;
	public:
		//constructor
		tSQueue() = default;
		
		// do not allow coping the object
		tSQueue(const tSQueue<T>&) = delete;
		
		// virtual constructor for child classes
		virtual ~tSQueue() { clear(); }

		//returns an element from the front of queue
		const T& front() {
			std::scoped_lock lock(muxQueue);
			return deqQueue.front();
		}

		//returns an element from the front of queue
		const T& back()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.back();
		}


		//removes and return an element from the front of queue
		T pop_front() {
			std::scoped_lock lock(muxQueue);
			auto t = std::move(deqQueue.front());
			deqQueue.pop_front();
			return t;
		}

		//removes and return an element from the back of queue
		T pop_back()
		{
			std::scoped_lock lock(muxQueue);
			auto t = std::move(deqQueue.back());
			deqQueue.pop_back();
			return t;
		}

		//adds an item in the front of queue
		void push_front(const T& item) {
			std::scoped_lock lock(muxQueue);
			deqQueue.emplace_front(std::move(item));

			std::unique_lock<std::mutex> ul(muxBlock);
			cvBlock.notify_one();
		}

		//adds an item in the back of queue
		void push_back(const T& item) {
			std::scoped_lock lock(muxQueue);
			deqQueue.emplace_back(std::move(item));

			std::unique_lock<std::mutex> ul(muxBlock);
			cvBlock.notify_one();
		}


		//returns true if Queue has no item
		bool empty() {
			std::scoped_lock lock(muxQueue);
			return deqQueue.empty();
		}

		//items in Queue
		size_t count() {
			std::scoped_lock lock(muxQueue);
			return deqQueue.size();
		}

		//clears queue
		void clear() {
			std::scoped_lock lock(muxQueue);
			deqQueue.clear();
		}

		//function waits if the queue is empty
		void wait() {
			while (empty()) {
				std::unique_lock<std::mutex> ul(muxBlock);
				cvBlock.wait(ul);
			}
		}
	};
	
}



