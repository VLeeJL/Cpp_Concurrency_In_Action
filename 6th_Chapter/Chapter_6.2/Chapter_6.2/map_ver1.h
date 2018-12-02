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
���̰߳�ȫ���мǲ��ܷ������á�
std::map<>���ڵ����⣺
1�������������õ�Ԫ�ر�ɾ����
2��һ��ʱ����ֻ��һ���߳̿��Է������ݽṹ
*/

//----solve meanus
/*ʹ��ϸ���������ɲ��õ����ݽṹ
1�����������޷���߲���������������ÿһ�����һ����޸Ĳ�������Ҫ���ʸ��ڵ㣬�ʸ��ڵ���Ҫ����
2���������飺��Ҫ��ס��������
3�����ù�ϣ���������ʵ�������Ͱ������
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
