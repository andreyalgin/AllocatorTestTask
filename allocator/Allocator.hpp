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

		for(T i = 0; i < num_blocks; i++) {
			records[i] = Record{i, static_cast<T>(i + 1)};
		}

		records[num_blocks] = Record{num_blocks, num_blocks};

		queue.store(records[0]);
	}
	
	~Allocator() = default;

	void* allocate_with_mutex() {
		std::lock_guard<std::mutex> guard(mutex);
		Record head = queue.load();

		queue.store(records[head.next]);

		if(head.index == num_blocks) {
			return nullptr;
		}

		return (pool.data() + data_size*head.index);
	}

	void deallocate_with_mutex(void* ptr) {
		uint8_t* data = reinterpret_cast<uint8_t*>(ptr);
		bool in_bounds = (data >= pool.data()) && ( data < (pool.data() + pool.size()) );

		assert(in_bounds == true);

		T index = (data - pool.data())/data_size;
		std::lock_guard<std::mutex> guard(mutex);
		Record head = queue.load();

		records[index].next = head.index;
		queue.store(records[index]);
	}

	void* allocate_with_atomic() {
		Record head = queue.load(std::memory_order_relaxed);

		do {
		} while(queue.compare_exchange_weak(head, records[head.next], std::memory_order_release, std::memory_order_relaxed) == false);

		if(head.index == num_blocks) {
			return nullptr;
		}

		return (pool.data() + data_size*head.index);
	}

	void deallocate_with_atomic(void* ptr) {
		uint8_t* data = reinterpret_cast<uint8_t*>(ptr);
		bool in_bounds = (data >= pool.data()) && ( data < (pool.data() + pool.size()) );

		assert(in_bounds == true);

		T index = (data - pool.data())/data_size;
		Record head = queue.load(std::memory_order_relaxed);

		do {
			records[index].next = head.index;
		} while(queue.compare_exchange_weak(head, records[index], std::memory_order_release, std::memory_order_relaxed) == false);
	}

private:
	struct Record {
		T index;
		T next; // Also an index (not a pointer !) to the next free cell in the pool
	};

	static constexpr T round_up(T size) {
		return ((size - 1 + alignment)/alignment)*alignment;
	}

	static constexpr T data_size = round_up(block_size);

	std::array<uint8_t, data_size*num_blocks> pool;
	std::array<Record, num_blocks + 1> records; // Last record is a dummy
	std::atomic<Record> queue;
	std::mutex mutex;
};

}
#endif /* ALLOCATOR_H_ */
