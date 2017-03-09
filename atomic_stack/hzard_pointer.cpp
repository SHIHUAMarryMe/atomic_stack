#include "hzard_pointer.h"


HzardPointer pointers[maxNumber]{};

HPPointerOwner::HPPointerOwner()try  //try
	:hp_ptr{ nullptr }
{
	//查看存放HzardPointer的数组还有没有空间能够放入新的HzardPointer.
	for (unsigned int index = 0; index < maxNumber; ++index) {

		//注意这里我们通过默认构造函数构造了一个线程的ID，这个时候该ID不代表任何线程.
		std::thread::id nullId{};

		if (pointers[index].thread_id.compare_exchange_strong(nullId, std::this_thread::get_id())) { //std::memory_order_seq_cst
			this->hp_ptr = &pointers[index];

			break;  //注意这里.
		}
	}

	if (!this->hp_ptr) {

		//如果此时同时运行的线程超过了我们预期的100，没法再设置新的风险指针了那么throw.
		throw std::runtime_error("bad hzard_pointer!"); //throw
	}

} catch (const std::runtime_error& error) {  //catch throw
	std::cerr << error.what() << std::endl;
}

HPPointerOwner::~HPPointerOwner()
{
	((this->hp_ptr)->pointer).store(nullptr);  //std::memory_order_seq_cst
	((this->hp_ptr)->thread_id).store(std::thread::id{}); //std::memory_order_seq_cst

}

std::atomic<void*>& HPPointerOwner::getPointer()& noexcept
{
	return (this->hp_ptr)->pointer;
}