#include "hzard_pointer.h"


HzardPointer pointers[maxNumber]{};

HPPointerOwner::HPPointerOwner()try  //try
	:hp_ptr{ nullptr }
{
	//�鿴���HzardPointer�����黹��û�пռ��ܹ������µ�HzardPointer.
	for (unsigned int index = 0; index < maxNumber; ++index) {

		//ע����������ͨ��Ĭ�Ϲ��캯��������һ���̵߳�ID�����ʱ���ID�������κ��߳�.
		std::thread::id nullId{};

		if (pointers[index].thread_id.compare_exchange_strong(nullId, std::this_thread::get_id())) { //std::memory_order_seq_cst
			this->hp_ptr = &pointers[index];

			break;  //ע������.
		}
	}

	if (!this->hp_ptr) {

		//�����ʱͬʱ���е��̳߳���������Ԥ�ڵ�100��û���������µķ���ָ������ôthrow.
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