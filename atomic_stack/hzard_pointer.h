#ifndef HZARD_POINTER_H
#define HZARD_POINTER_H

#include <iostream>
#include <memory>
#include <exception>
#include <atomic>
#include <thread>
#include <utility>


class HzardPointer;

constexpr static unsigned int maxNumber{ 100 };

extern  HzardPointer pointers[maxNumber];  //declare!



struct HzardPointer {
	std::atomic<std::thread::id> thread_id{};
	std::atomic<void*> pointer{ nullptr };

	HzardPointer() = default;
	~HzardPointer() = default;

	HzardPointer(const std::thread::id& id, void* pointer_)
		:thread_id{ id },
		pointer{ pointer_ } {}

	HzardPointer(const HzardPointer& other) = delete;
	HzardPointer& operator=(const HzardPointer& other) = delete;
	//HardPointer(HardPointer&& other) = delete;
	//HardPointer(HardPointer&& other) = delete;
};



class HPPointerOwner {
public:
	HPPointerOwner();
	~HPPointerOwner();
	HPPointerOwner(const HPPointerOwner& other) = delete;
	HPPointerOwner& operator=(const HPPointerOwner& other) = delete;
	//HPPointerOwner(HPPointerOwner&& other) = delete;
	//HPPointerOwner& operator=(HPPointerOwner&& other) = delete;

	std::atomic<void*>& getPointer()& noexcept;

	//delete
	std::atomic<void*>& getPointer()&& noexcept = delete;
	const std::atomic<void*>& getPointer()const & noexcept = delete;
	const std::atomic<void*>& getPointer()const && noexcept = delete;

private:
	HzardPointer* hp_ptr;
};


std::atomic<void*>& getHzardPointerForThisThread()noexcept
{
	thread_local static HPPointerOwner owner; //虽然指定了static其实仍然是thread_local起作用.
	return owner.getPointer();
}


//查找 reclaim_list中是否含有该 HzardPointer.
template<typename Ty>
bool outstandHzardPointer(Ty* ptr)noexcept
{
	for (unsigned int index = 0; index < maxNumber; ++index) {
		if (pointers[index].pointer.load() == ptr) { //std::memory_order_seq_cst
			return true;
		}
	}

	return false;
}

#endif //HZARD_POINTER_H