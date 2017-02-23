#ifndef GENERICPRIORITYQUEUE_H
#define GENERICPRIORITYQUEUE_H

#include <vector>
#include <algorithm>    // std::sort
#include <iostream>
#include <queue>
#include <deque>
#include <utility>
#include <vector>
#include <inttypes.h>

template <typename T> 
struct queue_comparator
{
    inline bool operator() (const std::pair <uint32_t, std::vector< T > > e1, 
    						const std::pair <uint32_t, std::vector< T > > e2)
    {
        return (e1.first < e2.first);
    }
};

/**
 * \class PriorityQueue
 * \brief Generic Priority Queue
 *
 * A generic priority queue which keeps packets utilizing the same priority class ordered.
 */
template <class T> class PriorityQueue {

	public:
		/**
		 * \brief Returns the current size of the queue
		 */
		uint32_t
		size() const
		{
			uint32_t elemCount = 0;
			for (auto it = m_priorities.begin(); it != m_priorities.end(); ++it) {
				auto queue = it->second;
				elemCount += queue.size();
			}
			return elemCount;
		}

		/**
		 * Removes the element with the highest priority from the queue
		 */
		T
		pop()
		{
			// std::cout << "Pop element, number of queues is " << m_priorities.size() << std::endl;
			for (auto it = m_priorities.begin(); it != m_priorities.end(); ++it) {
				auto queue = it->second;
				auto elem = queue.front();
				queue.erase(queue.begin());
				it->second = queue;
				if (queue.empty()) {
					// std::cout << "Erase empty queue" << std::endl;
					m_priorities.erase(it);
				}
				return elem;
			}
			return T();
		}

		/**
		 * Returns, but does not remove, the element with the highest priority.
		 */
		T
		top() const
		{
			for (auto it = m_priorities.begin(); it != m_priorities.end(); ++it) {
				auto queue = it->second;
				auto elem = queue.front();
				return elem;
			}
			return T();
		}

		/**
		 * \brief Add a new element to the queue
		 *
		 * @param elem Element to add
		 * @param priority Priority of the new element
		 */
		void
		push(T elem, uint32_t priority)
		{
			bool priorityFound = false;
			// std::cout << "insert element in queue, priority " << priority << std::endl;
			for (auto it = m_priorities.begin(); it != m_priorities.end(); ++it) {

				uint32_t queue_prio = it->first;
				if (queue_prio == priority) {
					// std::cout << "queue with priority " << priority << " found" << std::endl;
					priorityFound = true;
					auto queue = it->second;
					queue.push_back(elem);
					it->second = queue;
					break;
				}
			}

			if (!priorityFound) {
				// std::cout << "Priority " << priority << " not found, create new queue found" << std::endl;
				std::vector< T > queue;
				queue.push_back(elem);
				std::pair <uint32_t, std::vector< T > > queuePair(priority, queue);
				m_priorities.push_back(queuePair);

				std::sort(m_priorities.begin(), m_priorities.end(), queue_comparator<T>());
			}
			// std::cout << m_priorities.size() << " queues after insert" << std::endl;
			// std::cout << "Queue Size " << size() << std::endl;
		}

	protected:
		std::vector< std::pair <uint32_t, std::vector< T > > > m_priorities;


}; 

#endif

