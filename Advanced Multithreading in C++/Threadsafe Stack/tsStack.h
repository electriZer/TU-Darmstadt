#pragma once

#include <stack>
#include "tsDequeue.h"

/*!
* @class tsStack<T>
* @brief Thread Safe stack
* @extends tsDequeue
* @see tsDequeue
* @see tsQueue
* @author Jiyan Akgül
* @date November 2017
*/
template <typename T>
class tsStack : private tsDequeue<T>
{

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
	* Access next element
	*/
	T top() {
		if(!empty()) return First();
		else    throw new std::exception("Stack is Empty!");
	}
	/*!
	* Insert element
	*/
	void push(T Value) {
		push_top(Value);
	}
	/*!
	* Remove top element
	*/
	void pop() {
		if(!empty()) pop_top();
	}
};

