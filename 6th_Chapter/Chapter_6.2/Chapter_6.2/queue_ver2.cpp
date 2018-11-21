//use mutex and condition variable
#include <exception>
#include <queue>
#include <mutex>
#include <memory>

template <typename T>
class threadsafe_queue_secondver
{
private:
    mutable std::mutex mut_;
    std::queue<std::shared_ptr<T>> dataQueue_;
    std::condition_variable dataCond_;
public:
    threadsafe_queue_secondver() {}
    
    void wait_and_pop(T& val)
    {
        std::unique_lock<std::mutex> lk(mut_);
        dataCond_.wait(lk, [this] {return !dataQueue_.empty(); });
        val = std::move(*dataQueue_.front());
        dataQueue_.pop();
    }

    bool try_pop(T& val)
    {
        std::lock_guard<std::mutex> lk(mut_);
        if (dataQueue_.empty())
            return false;
        val = std::move(*dataQueue_.front());
        dataQueue_.pop();
        return true;
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut_);
        dataCond_.wait(lk, [this] {return !dataQueue_.empty(); });
        std::shared_ptr<T> res = dataQueue_.front();
        dataQueue_.pop();
        return res;
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut_);
        if (dataQueue_.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res = dataQueue_.front();
        dataQueue_.pop();
        return res;
    }

    void push(T new_val)
    {
        std::shared_ptr<T> data(std::make_shared<T>(std::move(new_val)));
        std::lock_guard<std::mutex> lk(mut_);
        dataQueue_.push(data);
        dataCond_.notify_one();
    }

    bool empty() const
    {
        std::lock_guard<T> lk(mut_);
        return dataQueue_.empty();
    }
};

//----advantage----
//1���µ�ʵ���������ʱ�����ᱻ����push()�С���һ�汾ֻ��pop()������ʱ���std::shared_ptr<>����
//2����std::shared_ptr<>�ķ�ʽ�����˻������ĳ���ʱ�䣬���������ܣ�std::share_ptr<>�ڴ湹����push����ɣ�

//----to solve question-----
/*��Ϊʹ��std::queue��Ϊ�洢���ݵĽṹ����mutex��������queue��
    ����ֻ����һ���̷߳�����һ��ʱ�̷��ʡ�������������
    --- ��Ҫ���ϸ����������ɸ߲���*/