#pragma once

// Concurrent Queue for Buffer
// Standard C++11 Implementation
// Nick Luo

#include <condition_variable>
#include <queue>
#include <mutex>

template<typename T>
class ConcurrentQueue
{
public:
	explicit ConcurrentQueue(size_t limit = 0);
	virtual ~ConcurrentQueue();
	ConcurrentQueue(const ConcurrentQueue&) = delete;            // disable copying
	ConcurrentQueue& operator=(const ConcurrentQueue&) = delete; // disable assignment
    bool Push(T&& item);
	bool Pop(T &item);	//Keep waiting until any item popped, if noWait is true
	T *Peek(int timeout);
	bool Pop_NoWait(T &item);
	void Discard();		//Stop popping waiting
	void Clear();
	bool Empty();
private:
	std::queue<T> queue;
	std::mutex mtx;
	std::condition_variable cond;
	size_t sizeLimit;
	bool discarded;
};

template <class T>
ConcurrentQueue<T>::ConcurrentQueue(size_t limit)
	:sizeLimit(limit), discarded(false)
{
}

template <class T>
ConcurrentQueue<T>::~ConcurrentQueue()
{
}

template <class T>
bool ConcurrentQueue<T>::Push(T&& item)
{
    std::lock_guard<std::mutex> lk(mtx);
    auto result = false;
    if (sizeLimit == 0 || queue.size() < sizeLimit)
    {
        discarded = false;
        queue.push(item);
        cond.notify_one();
        result = true;
    }
    return result;
}

template <class T>
bool ConcurrentQueue<T>::Pop_NoWait(T& item)
{
	std::lock_guard<std::mutex> lk(mtx);
	if (queue.empty())
		return false;
	item = std::move(queue.front());
	queue.pop();
	return true;
}

template <class T>
bool ConcurrentQueue<T>::Pop(T &item)
{
	std::unique_lock<std::mutex> lk(mtx);
	cond.wait(lk, [this]() { return !queue.empty() || discarded; });
	if (discarded)
		return false;
	item = std::move(queue.front());
	queue.pop();
    return true;
}

template <class T>
T *ConcurrentQueue<T>::Peek(int timeout)
{
	std::unique_lock<std::mutex> lk(mtx);
	if (!cond.wait_for(lk, std::chrono::milliseconds(timeout), 
		[this]() { return !queue.empty(); }))
		return nullptr;
    return &queue.front();
}

template <class T>
void ConcurrentQueue<T>::Clear()
{
    std::lock_guard<std::mutex> lk(mtx);
	queue.clear();
}

template <class T>
void ConcurrentQueue<T>::Discard()
{
    std::lock_guard<std::mutex> lk(mtx);
	discarded = true;
	cond.notify_one();
}

template <class T>
bool ConcurrentQueue<T>::Empty()
{
    std::lock_guard<std::mutex> lk(mtx);
    return queue.empty();
}


