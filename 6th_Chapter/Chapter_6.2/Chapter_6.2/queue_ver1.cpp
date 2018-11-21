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

    //std::unique_lock:延迟锁定、锁定的有时限尝试、递归锁定、所有权转移和与条件变量一同使用
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
//1、wait_and_pop()如果传入的类型禁止了移动构造/赋值函数会如何？
/*2、std::shared_ptr<T> res(std::make_shared<T>(std::move(dataQueue_.front())));
    引用计数的问题
*/


//----to solve question-----
/*1、因为notify_one()，所以当wait_and_pop()抛出异常(比如构造std::make_shared<>()失败)会导致剩余线程无法被唤醒
    ---1.1  notify_all()会耗费太多资源
    ---1.2  当有异常抛出时，让wait_and_pop()函数调用notify_one(),让另外一个线程尝试索引存储的值
    ---1.3  将std::shared_ptr<>初始化过程转移到push过程中*/