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
//1、新的实例分配结束时，不会被锁在push()中。上一版本只在pop()持有锁时完成std::shared_ptr<>分配
//2、用std::shared_ptr<>的方式减少了互斥量的持有时间，提升了性能（std::share_ptr<>内存构造在push中完成）

//----to solve question-----
/*因为使用std::queue作为存储数据的结构且用mutex保护整个queue，
    所以只能有一个线程访问在一个时刻访问。这限制了性能
    --- 需要提高细粒度锁来完成高并发*/