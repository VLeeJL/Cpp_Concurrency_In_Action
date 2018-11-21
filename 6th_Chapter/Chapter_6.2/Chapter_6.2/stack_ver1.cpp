//only use mutex
#include <exception>
#include <stack>
#include <mutex>
#include <memory>

struct empty_stack : std::exception
{
    const char* what() const throw();
};

template <typename T>
class threadsafe_stack
{
private:
    std::stack<T> dataStack_;
    mutable std::mutex mutex_;
public:
    threadsafe_stack() {}
    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lk(other.mutex_);
        data = other.dataStack_;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;
    void push(T new_val)
    {
        std::lock_guard<std::mutex> lk(mutex_);
        dataStack_.push(std::move(new_val));
    }
    //-----solve-----
    //1、pop函数直接返回对应的值，避免标准库中潜在的top和pop竞争
    //2、empty()和pop()存在竞争，通过在pop()函数上锁显式查询

    //-----warning-----
    //1、互斥量上锁会抛出异常  ---std::lock_guard<>保证上锁状态
    //2、dataStack_.push()在拷贝/移动时，或者因为内存不足造成异常 ---std::stack<>保证安全
    /*3、第一个pop()函数：std::make_shared会有内存不足的情况，或者拷贝/移动抛出异常
        ---C++标准库和运行库保证安全；新创建的对象能正常销毁。
            所以调用dataStack_.pop()dataStack_.pop()dataStack_.pop()是异常-安全*/
    /*4、std::lock_guard<>的异常问题  ---构造和析构函数非线程安全。若在构造/析构出现异常无问题
            因为只调用一次构造/析构*/


    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lk(mutex_);
        if (dataStack_.empty())
            throw empty_stack();
        std::shared_ptr<T> const res(std::make_shared<T>(std::move(dataStack_.top())));
        dataStack_.pop();// --->4   异常---安全
        return res;
    }
    void pop(T& val)
    {
        std::lock_guard<std::mutex> lk(mutex_);
        if (dataStack_.empty())
            throw empty_stack();
        val = std::move(dataStack_.top());
        dataStack_.pop();
    }
};
//-----question-----
//左值引用和右值引用

//-----usage-----
//1、需要保证构造完成前其他线程无法访问
//2、保证栈销毁后线程停止访问

//----to solve question-----
//线程等待锁限制了性能