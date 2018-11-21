#include <iostream>
#include <thread>
#include <algorithm>
#include <vector>
#include <numeric>

template<typename Iterator, typename T>
struct Accumlate_block
{
	void operator()(Iterator first, Iterator last, T& res)
	{
		res = std::accumulate(first, last, res);
	}
};


template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init)
{
	auto const length = std::distance(first, last);
	if (!length)//0
		return init;
	unsigned long const min_per_thread = 25;
	unsigned long const max_threads =
		(length + min_per_thread - 1) / min_per_thread;
	unsigned long const hardware_threads = std::thread::hardware_concurrency();
	unsigned long const temp_hardware_threads = hardware_threads != 0 ? hardware_threads : 2;
	unsigned long const num_threads =
		std::min((hardware_threads != 0 ? hardware_threads : 2), max_threads);
	unsigned long const block_size = length / num_threads;
	std::vector<T> results(num_threads);
	std::vector<std::thread> threadVec(num_threads - 1);
	Iterator block_start = first;
	for (unsigned long i = 0; i < (num_threads - 1); ++i)
	{
		Iterator block_end = block_start;
		std::advance(block_end, block_size);
		threadVec[i] = std::thread(Accumlate_block<Iterator, T>(),
			block_start, block_end, std::ref(results[i]));
		block_start = block_end;
	}
	Accumlate_block<Iterator, T>()(block_start, last, results[num_threads - 1]);
	std::for_each(threadVec.begin(), threadVec.end(), std::mem_fn(&std::thread::join));
	std::vector<T>::iterator resVecIterBegin = results.begin();
	std::vector<T>::iterator resVecIterEnd = results.end();
	//std::vector<int> arr;
	//auto ans = std::accumlate(arr.begin(), arr.end(), 0);
	//return ans;
	return std::accumulate(results.begin(), results.end(), init);
}

int main()
{
	std::vector<int> arr;
	for (int i = 0; i < 1000000000; ++i)
		arr.push_back(i);
	auto res = parallel_accumulate<std::vector<int>::iterator, int>(arr.begin(), arr.end(), 0);
	std::cout << "res	" << res << std::endl;
	system("pause");
	return 0;
}