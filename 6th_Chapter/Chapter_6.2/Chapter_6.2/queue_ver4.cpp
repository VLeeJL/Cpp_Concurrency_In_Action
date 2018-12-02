//use mutex and condition variable
#include <exception>
#include <queue>
#include <mutex>
#include <memory>

template <typename T>
class threadsafe_queue_secondver
{
private:
    struct Node
    {
        std::shared_ptr<T> dataPtr_;
        std::unique_ptr<Node> nextPtr_;
    };
    std::mutex headMutex_;
    std::unique_ptr<Node> headPtr_;
    std::mutex tailMutex_;
    Node* tailPtr_;
    Node* getTailPtr()
    {
        std::lock_guard<std::mutex> lk(tailMutex_);
        return tailPtr_;
    }

    std::unique_ptr<Node> popHeadPtr()//移动语义
    {
        std::lock_guard<std::mutex> headLock(headMutex_);
        if (headPtr_->get() == getTailPtr())
            return nullptr;
        std::unique_ptr<Node> oldHeadPtr = std::move(headPtr_);
        headPtr_ = std::move(oldHeadPtr->nextPtr_);
        return oldHeadPtr;
    }
public:
    threadsafe_queue_secondver() : headPtr_(new Node), tailPtr_(headPtr_->get()) {}
    threadsafe_queue_secondver(const threadsafe_queue_secondver&) = delete;
    threadsafe_queue_secondver& operator=(const threadsafe_queue_secondver&) = delete;

    std::shared_ptr<T> try_pop()
    {
        std::unique_ptr<Node> old_head = popHeadPtr();
        return old_head != nullptr ? old_head->dataPtr_ : std::shared_ptr<T>();
    }

    void push(T new_val)
    {
        std::shared_ptr<T> new_data_ptr(std::make_shared<T>(std::move(new_val)));
        std::unique_ptr<Node> ptr(new Node);
        Node* const new_tail_ptr = ptr->get();
        std::lock_guard<std::mutex> lk(tailMutex_);
        tailPtr_->dataPtr_ = new_data_ptr;
        tailPtr_->nextPtr_ = std::move(ptr);
        tailPtr_ = new_tail_ptr;
    }
};

//----rule----
/*不变量
1、tail->next == nullptr
2、tail->data == nullptr
3、head == tail
4、单元素列表head->next == tail
5、列表中每个节点x, x != tail且x->data指向一个T类型实例
6、head和tail维持相对顺序
*/

//----warning----
/*1、try_pop()函数，需要对tail_mutex上锁，保护对tail的读取；也需要保证访问头部数据；
try_pop和push有冲突，同时操作顺序未定义，存在数据竞争，
    ---但是由于get_tail()中上锁保护了读取，潜在的规定了（get_tail()也会对尾指针上锁）push和pop的顺序。*/

//----advantage----
//1、try_pop()直到获取锁才对数据进行修改，所以对锁的操作发生异常的情况下，也是异常安全的
//2、push()操作将对象分配给智能指针，保证发生异常时也能资源会被释放
//3、


//----question----
//为什么headMutex_和tailMutex_要保持相对顺序
//  ---因为为了保持tail指针的有效性。当要获取tail时，要保证当前队列没有新的元素加入队列尾部才能维持不变量

//----to solve question-----
/*1、加入try_pop()和wait_and_pop()：当通过引用进行值返回时，数据项可能在互斥锁未上锁的情况下被删除，
    将剩下的数据返回给调用者。如果T类型有无异常抛出的移动赋值操作可保证异常安全（但不通用）；
    但当不存在时，既有问题
    ---解决：通过将value赋值这一操作纳入锁的范围*/
