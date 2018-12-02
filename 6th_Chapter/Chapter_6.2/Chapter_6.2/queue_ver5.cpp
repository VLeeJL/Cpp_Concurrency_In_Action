#include "queue_ver5.h"

//struct Node
//{
//    std::shared_ptr<T> dataPtr_;
//    std::unique_ptr<T> nextPtr_;
//};
//
//std::mutex headMutex_;
//std::unique_ptr<Node> headPtr_;
//std::mutex tailMutex_;
//Node* tailPtr_;
//std::condition_variable dataCond_;

template<typename T>
std::shared_ptr<T> threadsafe_queue_thirdver<T>::try_pop()
{
    std::unique_ptr<Node> old_headPtr = try_pop_headPtr();
    return old_headPtr != nullptr ? old_headPtr->dataPtr_ : std::shared_ptr<T>();
}

template<typename T>
bool threadsafe_queue_thirdver<T>::try_pop(T& value)
{
    std::unique_ptr<Node> const old_head = try_pop_headPtr(value);
    return old_head;
}

template<typename T>
void threadsafe_queue_thirdver<T>::wait_and_pop(T& value)
{
    wait_and_pop(value);
}

template<typename T>
std::shared_ptr<T> threadsafe_queue_thirdver<T>::wait_and_pop()
{
    std::unique_ptr<Node> const old_headPtr = wait_pop_headPtr();
    return old_headPtr->dataPtr_;
}

template<typename T>
void threadsafe_queue_thirdver<T>::push(T new_val)
{
    std::shared_ptr<T> dataPtr(std::make_shared<T>(std::move(new_val)));
    std::unique_ptr<T> nodePtr(new Node);
    Node* const new_tail_ptr = nodePtr.get();
    std::lock_guard<std::mutex> lk(tailMutex_);
    tailPtr_->dataPtr_ = dataPtr;
    tailPtr_->nextPtr_ = std::move(nodePtr);
    tailPtr_ = new_tail_ptr;
}

template<typename T>
bool threadsafe_queue_thirdver<T>::empty()
{

}