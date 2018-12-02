#ifndef QUEUE_VER_FIVE
#define QUERE_VER_FIVE

//use mutex and condition variable
#include <exception>
#include <queue>
#include <mutex>
#include <memory>

//可上锁和等待的线程安全队列
template <typename T>
class threadsafe_queue_thirdver
{
private:
    struct Node
    {
        std::shared_ptr<T> dataPtr_;
        std::unique_ptr<T> nextPtr_;
    };

    std::mutex headMutex_;
    std::unique_ptr<Node> headPtr_;
    std::mutex tailMutex_;
    Node* tailPtr_;
    std::condition_variable dataCond_;

    Node* get_tailPtr()
    {
        std::lock_guard<std::mutex> tailLock(tailMutex_);
        return tailPtr_;
    }

    std::unique_ptr<Node> pop_headPtr()//配合其他函数使用,不需要上锁
    {
        std::unique_ptr<Node> old_headPtr = std::move(headPtr_);
        headPtr_ = std::move(old_headPtr->nextPtr_);
        return old_headPtr;
    }

    std::unique_ptr<Node> try_pop_headPtr()
    {
        std::lock_guard<std::mutex> head_lock(headMutex_);
        if (headPtr_->get() == get_tailPtr())
            return std::unique_ptr<Node>();
        return pop_headPtr();
    }

    std::unique_ptr<Node> try_pop_headPtr(T& value)
    {
        std::lock_guard<std::mutex> head_lock(headMutex_);
        if (headPtr_->get() == get_tailPtr())
            return std::unique_ptr<Node>();
        value = std::move(*(headPtr_->dataPtr_));
        return pop_headPtr();
    }

    std::unique_lock<std::mutex> wait_for_data()
    {
        std::unique_lock<std::mutex> head_lock(headMutex_);
        dataCond_.wait(head_lock, [&] {return headPtr_.get() != get_tailPtr(); });
        return std::move(head_lock);
    }

    std::unique_ptr<Node> wait_pop_headPtr()
    {
        std::unique_lock<std::mutex> head_lock(wait_for_data());//本身是右值引用
        return pop_headPtr();
    }

    std::unique_ptr<Node> wait_pop_headPtr(T& value)
    {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        value = std::move(*(headPtr_->dataPtr_));
        return pop_headPtr();
    }
public:
    threadsafe_queue_thirdver() :
        headPtr_(new Node), tailPtr_(headPtr_.get()) {}
    threadsafe_queue_thirdver(const threadsafe_queue_thirdver&) = delete;
    threadsafe_queue_thirdver& operator=(const threadsafe_queue_thirdver&) = delete;
    std::shared_ptr<T> try_pop();
    bool try_pop(T& value);
    void wait_and_pop(T& value);
    std::shared_ptr<T> wait_and_pop();
    void push(T new_val);
    bool empty();
};
#endif // QUEUE_VER_FIVE