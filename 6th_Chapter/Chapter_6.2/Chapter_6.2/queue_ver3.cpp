//use mutex and condition variable
#include <exception>
#include <queue>
#include <mutex>
#include <memory>

//single thread
template <typename T>
class queue
{
private:
    struct Node
    {
        T data_;
        std::unique_ptr<Node> nextPtr_;
        Node(T data) : data_(std::move(data)) {}
    };

    std::unique_ptr<Node> headPtr_;
    Node* tailPtr_;
public:
    queue() :tailPtr_(nullptr) {}
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;
    std::shared_ptr<T> try_pop()
    {
        if (!headPtr_)
            return std::shared_ptr<T>();
        std::shared_ptr<T> const res(std::make_shared<T>(std::move(headPtr_->data_)));
        std::unique_ptr<Node> const old_headPtr = std::move(headPtr_);//离开作用域自动删除
        headPtr_ = std::move(old_headPtr->nextPtr_);
        return res;
    }

    void push(T new_val)
    {
        std::unique_ptr<Node> nodePtr(new Node(std::move(new_val)));
        Node* const new_tail = nodePtr->get();
        if (tailPtr_ == nullptr)
            headPtr_ = std::move(nodePtr);
        else
            tailPtr_->nextPtr_ = std::move(nodePtr);
        tailPtr_ = new_tail;
    }
};

//----to solve question-----
/* single thread如果尝试使用细粒度锁时会有问题:
    1、因为头指针和尾指针指向同一个节点，故保护时需要两个互斥量保护
    2、push()和try_pop()都能访问next指针指向的节点:
        push:操控tail->nextPtr_,pop操控headPtr->next。
        同一个对象同时try_pop和push时，锁住同一个锁
    ----分离数据实现并发：可预分配一个虚拟节点
*/

template <typename T>
class queue_secondver
{
private:
    struct Node
    {
        std::shared_ptr<T> dataPtr_;
        std::unique_ptr<Node> nextPtr_;
    };

    std::unique_ptr<Node> headPtr_;
    Node* tailPtr_;

public:
    queue_secondver() : headPtr_(new Node()), tailPtr_(headPtr_->get()) {}
    queue_secondver(const queue&) = delete;
    queue_secondver& operator=(const queue_secondver& other) = delete;
    std::shared_ptr<T> try_pop()
    {
        if (tailPtr_ == headPtr_->get())
            return std::shared_ptr<T>();
        std::shared_ptr<T> const res(headPtr_->dataPtr_);
        std::unique_ptr<Node> old_head = std::move(headPtr_);
        headPtr_ = std::move(old_head->nextPtr_);
        return res;
    }

    //尾指针始终指向虚拟节点
    void push(T new_val)
    {
        std::shared_ptr<T> new_dataPtr(std::make_shared<T>(new_val));
        std::unique_ptr<T> nodePtr(new Node());
        tailPtr_->dataPtr_ = new_dataPtr;
        Node* const new_tailPtr = nodePtr->get();
        tailPtr_->nextPtr_ = std::move(nodePtr);
        tailPtr_ = new_tailPtr;
    }
};