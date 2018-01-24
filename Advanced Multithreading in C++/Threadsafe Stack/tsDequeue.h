#pragma once
#include <mutex>
#include <shared_mutex>
/************************************************************************/
/* Class Definition                                                     */
/************************************************************************/
/*!
* @class tsDequeue<T>
* @brief Thread Safe double ended queue
*
* @remarks	This file consists of the typical header file class definition (only function names),
*			followed by the full function description at the end of the file.
*
* @relates tsQueue
* @relates tsStack
* @author Jiyan Akgül
* @date November 2017
*/
template <typename T>
class tsDequeue
{
public:
	/*!
	* Test whether container is empty
	*/
	bool   empty();
	/*!
	* Return size
	*/
	size_t size();
	/*!
	* Access next element
	*/
	T First();
	/*!
	* Access last element
	*/
	T Last();
	/*!
	* Add element at the end
	*/
	void push_back(T elem);
	/*!
	* Insert element at beginning
	*/
	void push_front(T elem);
	/*!
	* Delete last element
	*/
	void pop_back();
	/*!
	* Delete first element
	*/
	void pop_top();

	tsDequeue();
	~tsDequeue();

private:
	/*!
	* @class LinkedNode<typename T>
	* @brief Internal Node for linked list of LinkedNodes<T>
	*/
	template<typename T> class LinkedNode {
	public:
		LinkedNode() {
			isFree = false;
			Before = NULL;
			Next = NULL;
		}
		/*! Debug Variable */
		bool isFree;
		/*! Pointer to trailing Node */
		LinkedNode<T>* Before;
		/*! Pointer to leading Node */
		LinkedNode<T>* Next;
		/*! Stored Node Value*/
		T Value;
	};

	/*!
	* First Node and start of Linked List
	* @remark NULL if list is empty
	*/

	LinkedNode<T>* FirstNode;
	/*!
	* Last Node in Linked List
	* @remark NULL if list has only one Element or is empty
	*/
	LinkedNode<T>* LastNode;

	/*!
	* Separate list of free nodes
	* @details	Freed LinkedNodes are not really freed.
	*			They will be reused and are stored in this linked stack until then.
	*/
	LinkedNode<T>* FirstFreeNode;

	/*! Mutex for access on main Node Variables @ref FirstNode and @ref LastNode */
	std::shared_mutex NodeMutex;

	/*! Mutex for access on linked list of free nodes @ref FirstFreeNode */
	std::shared_mutex FreeNodeMutex;

	void FreeNode(LinkedNode<T>* Node);
	LinkedNode<T>* GetFreeNode();

};
/************************************************************************/
/* Template Function Descriptions                                       */
/************************************************************************/

template <typename T>
tsDequeue<T>::tsDequeue() {
	FirstNode = NULL;
	FirstFreeNode = NULL;
	LastNode = NULL;
}

template <typename T>
tsDequeue<T>::~tsDequeue() {
	std::unique_lock<std::shared_mutex> ul(NodeMutex);
	LinkedNode<T>* NextNode = FirstNode;		// Tmp Variable to store next free node
	LinkedNode<T>* TmpNextNode;					// Tmp Variable to current store free node
	while (NextNode != NULL) {					// WHILE List of free nodes is not empty
		TmpNextNode = NextNode;					// Store NextNode in tmp value
		NextNode = FirstNode->Next;				// Set new trailing node as new FirstNode
		delete TmpNextNode;						// Delete old FirstFreeNode from tmp value
	}
	ul.unlock();								// Unlock NodeMutex
	std::unique_lock<std::shared_mutex> ul_free_nodes(FreeNodeMutex);
	NextNode = FirstFreeNode;					// Tmp Variable to store next free node
	TmpNextNode = NULL;							// Tmp Variable to current store free node
	while (NextNode != NULL) {					// WHILE List of free nodes is not empty
		TmpNextNode = NextNode;					// Store NextNode in tmp value
		NextNode = FirstFreeNode->Next;			// Set new trailing node as new FirstFreeNode
		delete TmpNextNode;						// Delete old FirstFreeNode from tmp value
	}
	ul_free_nodes.unlock();						// Unlock FreeNodeMutex
}

template<class T>
bool tsDequeue<T>::empty() {
	std::shared_lock<std::shared_mutex> sl(NodeMutex);
	if (FirstNode == NULL)
		return true;
	else if (FirstNode->isFree)
		return true;
	else
		return false;
}

template<class T>
size_t tsDequeue<T>::size() {
	std::shared_lock<std::shared_mutex> sl(NodeMutex);
	size_t count = 0;
	LinkedNode<T>* NextNode = FirstNode;		// Stores current node in iteration
	while (NextNode != NULL) {					// WHILE current node is not null
		count++;								// Increment counter
		NextNode = NextNode->Next;				// set current node to next linked node
	}
	return count;
}

template<class T>
void tsDequeue<T>::push_back(T elem)
{
	LinkedNode<T>* new_node = GetFreeNode();	// Get a free Node
	new_node->Value = elem;						// Set the value
	std::unique_lock<std::shared_mutex> ul(NodeMutex);
	// Set and read the node pointers 
	//   in one step to avoid Race Conditions
	if (FirstNode == NULL) {				// IF List is Empty		
		FirstNode = new_node;				// Set First Node
	}
	else if (LastNode == NULL) {			// IF List has only 1 Entry
		LastNode = new_node;				// Set Last Node
		FirstNode->Next = new_node;			// Link fore to new node
		new_node->Before = FirstNode;		// Link back to leading entry
	}
	else {									// IF List contains more then 1 element
		LastNode->Next = new_node;			// Link fore to new node
		new_node->Before = LastNode;		// Link back from new node to old last node
		LastNode = new_node;				// Set new node as new LastNode
	}
	ul.unlock();							// Unlock exclusive access
}

template<typename T>
void tsDequeue<T>::push_front(T elem)
{
	LinkedNode<T>* new_node = GetFreeNode();	// Get a free Node
	new_node->Value = elem;					// Set the value
	std::unique_lock<std::shared_mutex> ul(NodeMutex);
	// Set and read the node pointers 
	//   in one step to avoid Race Conditions
	if (FirstNode == NULL) 					// IF List is Empty		
		FirstNode = new_node;				// Set FirstNode
	else {									// IF List contains elements
		new_node->Next = FirstNode;			// Link new node to old FirstNode
		FirstNode->Before = new_node;		// Link back to new node from old FirstNode
		FirstNode = new_node;				// Set new node as new FirstNode
		if (LastNode == NULL) 				// IF List had only 1 element
			LastNode = FirstNode->Next;		// Set trailing Node(old FirstNode) as LastNode
	}
	ul.unlock();							// Unlock exclusive access
}


template<typename T>
void tsDequeue<T>::pop_back()
{
	std::shared_lock<std::shared_mutex> sl(NodeMutex);
	if (LastNode == NULL && FirstNode == NULL)  // IF List is empty	
		return;									// Nothing to do here

	sl.unlock();								// Unlock shared for read
	LinkedNode<T>* OldNode = NULL;					// Temp Value to store removed node

	std::unique_lock<std::shared_mutex> ul(NodeMutex);
	if (LastNode == NULL && FirstNode != NULL)  // IF List contains 1 element
	{
		OldNode = FirstNode;					// Store pointer to old FirstNode
		FirstNode = NULL;						// Make list Empty
	}
	else if (LastNode != NULL) {
		OldNode = LastNode;						// Store pointer to old LastNode
		if (LastNode->Before != NULL)				// IF there is a leading Node
			LastNode->Before->Next = NULL;		// Remove Link to old LastNode in leading node
		LastNode = LastNode->Before;			// Set leading node as new LastNode (can be NULL)
	}
	ul.unlock();								// Unlock exclusive access

	FreeNode(OldNode);							// Free stored OldNode
}

template<typename T>
void tsDequeue<T>::pop_top()
{
	std::shared_lock<std::shared_mutex> sl(NodeMutex);
	if (FirstNode == NULL)						// IF List is empty	
		return;									// Nothing to do here

	sl.unlock();								// Unlock shared for read
	LinkedNode<T>* OldNode = NULL;					// Temp Value to store removed node

	std::unique_lock<std::shared_mutex> ul(NodeMutex);
	if (FirstNode != NULL) {					// IF List contains 1 one or more elements
		OldNode = FirstNode;					// Store pointer to old FirstNode
		if (FirstNode->Next == NULL) 			// IF there is no trailing node
			LastNode = NULL;					// Set LastNode to NULL (because the list is empty now)
		else {									// IF there is a trailing node 
			FirstNode->Next->Before = NULL;		// Remove Link to old FirstNode in trailing node
			FirstNode = FirstNode->Next;		// Set trailing node as new FirstNode (can be NULL)
			if (FirstNode->Next == NULL)		// If the new FirstNode has no trailing node
				LastNode = NULL;				// Set LastNode to NULL 
												//  because the list has only one entry now
		}
	}
	ul.unlock();								// Unlock exclusive access

	FreeNode(OldNode);							// Free stored OldNode
}

template<typename T>
T tsDequeue<T>::First()
{
	std::shared_lock<std::shared_mutex> sl(NodeMutex);
	if (FirstNode == NULL)								// IF List is empty
		throw new std::exception("Dequeue is empty!");	// Throw Error
	else												// IF List has more then 1 element
		return FirstNode->Value;						// Return Value
}

template<class T>
T tsDequeue<T>::Last()
{
	std::shared_lock<std::shared_mutex> sl(NodeMutex);
	if (LastNode == NULL && FirstNode == NULL)		// IF List is empty
		throw new std::exception("List is empty!");	// Throw Error
	if (LastNode == NULL && FirstNode != NULL)		// IF List has 1 element
		return FirstNode->Value;					// Return Value
	else											// IF List has more then 1 element
		return LastNode->Value;						// Return Value
}

template<typename T>
void tsDequeue<T>::FreeNode(LinkedNode<T>* Node)
{
	Node->isFree = true;						// Mark Node as free
	Node->Next = NULL;
	Node->Before = NULL;
	std::unique_lock<std::shared_mutex> ul_free_nodes(FreeNodeMutex);
	if (FirstFreeNode != NULL)					// IF List of free nodes is not empty
		Node->Next = FirstFreeNode;				// Link from new node to FirstFreeNode
	FirstFreeNode = Node;						// Set Node as new FirstFreeNode
	ul_free_nodes.unlock();						// Unlock exclusive access
}

template<typename T>
tsDequeue<T>::LinkedNode<T>* tsDequeue<T>::GetFreeNode()
{
	LinkedNode<T>* free_node = NULL;
	std::unique_lock<std::shared_mutex> ul_free_nodes(FreeNodeMutex);
	if (FirstFreeNode != NULL) {				// IF List of free nodes is not empty
		free_node = FirstFreeNode;				// Store FirstFreeNode as new free_node
		FirstFreeNode = free_node->Next;		// Next free node is now FirstFreeNode (can be NULL)
	}
	else {										// IF list of free nodes is empty
		free_node = new LinkedNode<T>();		// Construct new Node
	}
	ul_free_nodes.unlock();						// Unlock exclusive access
	free_node->Before = NULL;
	free_node->Next = NULL;
	free_node->isFree = false;
	return free_node;
}
