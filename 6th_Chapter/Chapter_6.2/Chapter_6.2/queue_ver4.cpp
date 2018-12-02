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

    std::unique_ptr<Node> popHeadPtr()//�ƶ�����
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
/*������
1��tail->next == nullptr
2��tail->data == nullptr
3��head == tail
4����Ԫ���б�head->next == tail
5���б���ÿ���ڵ�x, x != tail��x->dataָ��һ��T����ʵ��
6��head��tailά�����˳��
*/

//----warning----
/*1��try_pop()��������Ҫ��tail_mutex������������tail�Ķ�ȡ��Ҳ��Ҫ��֤����ͷ�����ݣ�
try_pop��push�г�ͻ��ͬʱ����˳��δ���壬�������ݾ�����
    ---��������get_tail()�����������˶�ȡ��Ǳ�ڵĹ涨�ˣ�get_tail()Ҳ���βָ��������push��pop��˳��*/

//----advantage----
//1��try_pop()ֱ����ȡ���Ŷ����ݽ����޸ģ����Զ����Ĳ��������쳣������£�Ҳ���쳣��ȫ��
//2��push()������������������ָ�룬��֤�����쳣ʱҲ����Դ�ᱻ�ͷ�
//3��


//----question----
//ΪʲôheadMutex_��tailMutex_Ҫ�������˳��
//  ---��ΪΪ�˱���tailָ�����Ч�ԡ���Ҫ��ȡtailʱ��Ҫ��֤��ǰ����û���µ�Ԫ�ؼ������β������ά�ֲ�����

//----to solve question-----
/*1������try_pop()��wait_and_pop()����ͨ�����ý���ֵ����ʱ������������ڻ�����δ����������±�ɾ����
    ��ʣ�µ����ݷ��ظ������ߡ����T���������쳣�׳����ƶ���ֵ�����ɱ�֤�쳣��ȫ������ͨ�ã���
    ����������ʱ����������
    ---�����ͨ����value��ֵ��һ�����������ķ�Χ*/
