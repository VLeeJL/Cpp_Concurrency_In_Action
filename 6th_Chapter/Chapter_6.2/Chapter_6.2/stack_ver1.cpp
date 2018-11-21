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
    //1��pop����ֱ�ӷ��ض�Ӧ��ֵ�������׼����Ǳ�ڵ�top��pop����
    //2��empty()��pop()���ھ�����ͨ����pop()����������ʽ��ѯ

    //-----warning-----
    //1���������������׳��쳣  ---std::lock_guard<>��֤����״̬
    //2��dataStack_.push()�ڿ���/�ƶ�ʱ��������Ϊ�ڴ治������쳣 ---std::stack<>��֤��ȫ
    /*3����һ��pop()������std::make_shared�����ڴ治�����������߿���/�ƶ��׳��쳣
        ---C++��׼������пⱣ֤��ȫ���´����Ķ������������١�
            ���Ե���dataStack_.pop()dataStack_.pop()dataStack_.pop()���쳣-��ȫ*/
    /*4��std::lock_guard<>���쳣����  ---����������������̰߳�ȫ�����ڹ���/���������쳣������
            ��Ϊֻ����һ�ι���/����*/


    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lk(mutex_);
        if (dataStack_.empty())
            throw empty_stack();
        std::shared_ptr<T> const res(std::make_shared<T>(std::move(dataStack_.top())));
        dataStack_.pop();// --->4   �쳣---��ȫ
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
//��ֵ���ú���ֵ����

//-----usage-----
//1����Ҫ��֤�������ǰ�����߳��޷�����
//2����֤ջ���ٺ��߳�ֹͣ����

//----to solve question-----
//�̵߳ȴ�������������