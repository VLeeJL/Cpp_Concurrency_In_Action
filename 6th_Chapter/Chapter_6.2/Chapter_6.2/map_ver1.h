#ifndef MAP_VERSION_ONE
#define MAP_VERSION_ONE
#include <mutex>
#include <memory>
#include <functional>
#include <shared_mutex>
#include <algorithm>
#include <list>
#include <vector>

/*
----problem----
【线程安全：切记不能返回引用】
std::map<>存在的问题：
1、迭代器：引用的元素被删除；
2、一个时间内只有一个线程可以访问数据结构
*/

//----solve meanus
/*使用细粒度锁，可采用的数据结构
1、二叉树（无法提高并发访问能力）：每一个查找或者修改操作都需要访问根节点，故根节点需要上锁
2、有序数组：需要锁住整个数组
3、采用哈希表：并发访问的能力是桶的数量
*/

template <typename Key, typename Value, typename Hash = std::hash<Key>>
class threadsafe_lookup_table
{
private:
    class bucket_type
    {
    private:
        typedef std::pair<Key, Value> bucket_value;
        typedef std::list<bucket_value> bucket_dataList;
        typedef typename bucket_dataList::iterator bucket_iterator;

        bucket_dataList dataList_;
        mutable std::shared_mutex mutex_;
        bucket_iterator find_entry_for(Key const& key) const 
        {
            return std::find_if(dataList_.begin(), dataList_.end(),
                [&](bucket_value const& item)
            {return item.first == key; })
        }
    public:
        Value value_for(Key const& key, Value const& default_value) const
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            bucket_iterator const found_entry = find_entry_for(key);
            return (found_entry == dataList_.end()) ? default_value : found_entry;
        }

        void add_or_update_mapping(Key const& key, Value const& value)
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            bucket_iterator const found_entry = find_entry_for(key);
            if (found_entry == dataList_.end())
                dataList_.push_back(bucket_value(key, value));
            else
                found_entry->second = value;
        }

        void remove_mapping(Key const& key)
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            bucket_iterator const found_entry = find_entry_for(key);
            if (found_entry != dataList_.end())
                dataList_.erase(found_entry);
        }
    };

    std::vector<std::unique_ptr<bucket_type>> bucketsVec_;
    Hash hashFun;
    bucket_type& get_bucket(Key const& key) const //index invalid?
    {
        std::size_t const bucket_index = hashFun(key) % bucketsVec_.size();
        return *bucketsVec_[bucket_index];
    }
public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef Hash hash_type;

    threadsafe_lookup_table(unsigned num_buckets = 19, Hash& const hash_fun = Hash()) :
        bucketsVec_(num_buckets), hashFun(hash_fun)
    {
        for (auto &iter : bucketsVec_)
            iter.reset(new bucket_type);
    }

    threadsafe_lookup_table(threadsafe_lookup_table const&) = delete;
    threadsafe_lookup_table& operator=(threadsafe_lookup_table const&) = delete;
    mapped_type value_for(key_type const& key, mapped_type const& default_value = mapped_type())
    {
        return get_bucket(key).value_for(key, default_value);
    }

    void add_or_update_mapping(key_type const& key, mapped_type const& value)
    {
        get_bucket(key).add_or_update_mapping(key, value);
    }
};


#endif // !MAP_VERSION_ONE
