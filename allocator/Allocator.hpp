#ifndef ALLOCATOR_HPP_
#define ALLOCATOR_HPP_

#include <cstddef>
#include <cstdint>
#include <assert.h>

#include <array>
#include <atomic>
#include <mutex>

namespace allocators {

template<typename T, T block_size, T num_blocks, T alignment>
class Allocator {
public:
	Allocator() {
		pool.fill(0x00);
		queue = 0;

		for(T i = 0; i < num_blocks; i++) {
			records[i].next = i + 1;
		}

		records[num_blocks].next = num_blocks;
	}
	
	~Allocator() = default;

	void* allocate_with_mutex() {
		std::lock_guard<std::mutex> guard(mutex);
		T index = queue;

		queue = records[index].next;

		if(index == num_blocks) {
			return nullptr;
		}

		return (pool.data() + data_size*index);
	}

	void deallocate_with_mutex(void* ptr) {
		uint8_t* data = reinterpret_cast<uint8_t*>(ptr);
		bool in_bounds = (data >= pool.data()) && ( data < (pool.data() + pool.size()) );

		assert(in_bounds == true);

		T index = (data - pool.data())/data_size;
		std::lock_guard<std::mutex> guard(mutex);

		records[index].next = queue;
		queue = index;
	}

	/*
		For some unknown reason atomic version randomly fails multithreaded test
	*/

	void* allocate_with_atomic() {
		T desired;
		T expected = queue.load(std::memory_order_relaxed);

		do {
			desired = records[expected].next;
		} while(queue.compare_exchange_weak(expected, desired, std::memory_order_release, std::memory_order_relaxed) == false);

		if(expected == num_blocks) {
			return nullptr;
		}

		return (pool.data() + data_size*expected);
	}

	void deallocate_with_atomic(void* ptr) {
		uint8_t* data = reinterpret_cast<uint8_t*>(ptr);
		bool in_bounds = (data >= pool.data()) && ( data < (pool.data() + pool.size()) );

		assert(in_bounds == true);

		T desired = (data - pool.data())/data_size;
		T expected = queue.load(std::memory_order_relaxed);

		do {
			records[desired].next = expected;
		} while(queue.compare_exchange_weak(expected, desired, std::memory_order_release, std::memory_order_relaxed) == false);
	}

private:
	struct Record {
		T next; // Index (not pointer !) to the next cell in pool
	};

	static constexpr T round_up(T size) {
		return ((size - 1 + alignment)/alignment)*alignment;
	}

	static constexpr T data_size = round_up(block_size);

	std::array<uint8_t, data_size*num_blocks> pool;
	std::array<Record, num_blocks + 1> records; // Last record is a dummy
	std::atomic<T> queue;
	std::mutex mutex;
};

}
#endif /* ALLOCATOR_H_ */
