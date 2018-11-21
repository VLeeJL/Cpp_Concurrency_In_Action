//use mutex and condition variable
#include <exception>
#include <queue>
#include <mutex>
#include <memory>


template <typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut_;
    std::queue<T> dataQueue_;
    std::condition_variable dataCond_;
public:
    threadsafe_queue() {}
    
    void push(T new_val)
    {
        std::lock_guard<std::mutex> lk(mut_);
        dataQueue_.push(std::move(new_val));
        dataCond_.notify_one();
    }

    //std::unique_lock:�ӳ���������������ʱ�޳��ԡ��ݹ�����������Ȩת�ƺ�����������һͬʹ��
    void wait_and_pop(T& new_val)
    {
        std::unique_lock<std::mutex> lk(mut_);
        dataCond_.wait(lk, [this] {return !dataQueue_.empty(); });
        new_val = std::move(dataQueue_.front());
        dataQueue_.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut_);
        dataCond_.wait(lk, [this] {return !dataQueue_.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(dataQueue_.front())));
        dataQueue_.pop();
        return res;
    }

    bool try_pop(T& val)
    {
        std::lock_guard<std::mutex> lk(mut_);
        if (dataQueue_.empty())
            return false;
        val = std::move(dataQueue_.front());
        dataQueue_.pop();
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut_);
        if (dataQueue_.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(dataQueue_.front())));
        dataQueue_.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut_);
        return dataQueue_.empty();
    }
};

//-----question-----
//1��wait_and_pop()�����������ͽ�ֹ���ƶ�����/��ֵ��������Σ�
/*2��std::shared_ptr<T> res(std::make_shared<T>(std::move(dataQueue_.front())));
    ���ü���������
*/


//----to solve question-----
/*1����Ϊnotify_one()�����Ե�wait_and_pop()�׳��쳣(���繹��std::make_shared<>()ʧ��)�ᵼ��ʣ���߳��޷�������
    ---1.1  notify_all()��ķ�̫����Դ
    ---1.2  �����쳣�׳�ʱ����wait_and_pop()��������notify_one(),������һ���̳߳��������洢��ֵ
    ---1.3  ��std::shared_ptr<>��ʼ������ת�Ƶ�push������*/