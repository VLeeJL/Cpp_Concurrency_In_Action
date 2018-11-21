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
        std::unique_ptr<Node> const old_headPtr = std::move(headPtr_);//�뿪�������Զ�ɾ��
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
/* single thread�������ʹ��ϸ������ʱ��������:
    1����Ϊͷָ���βָ��ָ��ͬһ���ڵ㣬�ʱ���ʱ��Ҫ��������������
    2��push()��try_pop()���ܷ���nextָ��ָ��Ľڵ�:
        push:�ٿ�tail->nextPtr_,pop�ٿ�headPtr->next��
        ͬһ������ͬʱtry_pop��pushʱ����סͬһ����
    ----��������ʵ�ֲ�������Ԥ����һ������ڵ�
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

    //βָ��ʼ��ָ������ڵ�
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