#include "tSQueue.h"

//template<class T>
//olc::net::tSQueue<T>::tSQueue() = default;
//
//template<class T>
//olc::net::tSQueue<T>::tSQueue(const tSQueue<T>&) = default;
//
//template<class T>
//olc::net::tSQueue<T>::~tSQueue()
//{
//	clear();
//}

//template<class T>
//const T& olc::net::tSQueue<T>::front()
//{
//	std::scoped_lock lock(muxQueue);
//	return deqQueue.front();
//}
//
//template<class T>
//const T& olc::net::tSQueue<T>::back()
//{
//	std::scoped_lock lock(muxQueue);
//	return deqQueue.back();
//}

//template<class T>
//T olc::net::tSQueue<T>::pop_front()
//{
//	std::scoped_lock lock(muxQueue);
//	auto t = std::move(deqQueue.front());
//	deqQueue.pop_front();
//	return t;
//}
//
//template<class T>
//T olc::net::tSQueue<T>::pop_back()
//{
//	std::scoped_lock lock(muxQueue);
//	auto t = std::move(deqQueue.back());
//	deqQueue.pop_back();
//	return t;
//}

//template<class T>
//void olc::net::tSQueue<T>::push_front(const T& item)
//{
//	std::scoped_lock lock(muxQueue);
//	deqQueue.emplace_front(std::move(item));
//
//	std::unique_lock<std::mutex> ul(muxBlock);
//	cvBlock.notify_one();
//}
//
//template<class T>
//void olc::net::tSQueue<T>::push_back(const T& item)
//{
//	std::scoped_lock lock(muxQueue);
//	deqQueue.emplace_back(std::move(item));
//
//	std::unique_lock<std::mutex> ul(muxBlock);
//	cvBlock.notify_one();
//}

template<class T>
bool olc::net::tSQueue<T>::empty()
{
	std::scoped_lock lock(muxQueue);
	return deqQueue.empty();
}

template<class T>
size_t olc::net::tSQueue<T>::count()
{
	std::scoped_lock lock(muxQueue);
	return deqQueue.size();
}

template<class T>
void olc::net::tSQueue<T>::clear()
{
	std::scoped_lock lock(muxQueue);
	deqQueue.clear();
}

template<class T>
void olc::net::tSQueue<T>::wait()
{
	while (empty()) {
		std::unique_lock<std::mutex> ul(muxBlock);
		cvBlock.wait(ul);
	}
}
