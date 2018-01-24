#pragma once
#include "tsDequeue.h"

/*!
* @class tsQueue<T>
* @brief Thread Safe queue
* @extends tsDequeue
* @see tsDequeue
* @see tsStack
* @author Jiyan Akgül
* @date November 2017
*/
template <typename T>
class tsQueue : tsDequeue<T> {
public:
	tsStack() {
		FirstNode = NULL;
		FirstFreeNode = NULL;
		LastNode = NULL;
	}
	/*!
	* Test whether container is empty
	*/
	using tsDequeue<T>::empty;
	/*!
	* Return size
	*/
	using tsDequeue<T>::size;
	/*!
	* Access last element
	*/
	T back() {
		if (!empty()) return First();
		else    throw new std::exception("Queue is Empty!");
	}
	/*!
	* Access next element
	*/
	T front() {
		if (!empty()) return Last();
		else    throw new std::exception("Queue is Empty!");
	}
	/*!
	* Insert element
	*/
	void push(T Value) { push_top(Value); }
	/*!
	* Remove next element
	*/
	void pop () { if (!empty()) pop_back(); }
};

