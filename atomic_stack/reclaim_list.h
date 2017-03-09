#ifndef RECLAIM_LIST_H
#define RECLAIM_LIST_H

#include <atomic>
#include <exception>

//defined hzard-pointer
#include "hzard_pointer.h"



//当stack中的一个结点被几个线程使用的时候为了防止该结点被其中的线程给删除
//因此把该结点放入到 list中.
template<typename Ty>
struct DataToReclaim {
	Ty* data_ptr{ nullptr };
	DataToReclaim<Ty>* next{ nullptr };

	DataToReclaim() = default;

	DataToReclaim(Ty* ptr, DataToReclaim* dtr_ptr = nullptr)
		:data_ptr{ ptr },
		next{ dtr_ptr } {}

	DataToReclaim(const DataToReclaim<Ty>& other) = delete;
	DataToReclaim& operator=(const DataToReclaim<Ty>& other) = delete;
	//DataToReclaim(DataToReclaim<Ty>&& other) =delete;
	//DataToReclaim& operator=(DataToReclaim<Ty>&& other) = delete;

	~DataToReclaim()
	{
		if (this->data_ptr != nullptr) {
			delete (this->data_ptr);
		}

		this->next = nullptr;
	}

};


//当有结点将要被删除的时候首先检查其他线程是否正在使用该结点
//如果正在使用那么把结点放入到该list.
template<typename Ty>
class ManagerList {
public:
	ManagerList() = default;

	~ManagerList()
	{
		this->deleter();
	}

	ManagerList(const ManagerList<Ty>& other) = delete;
	ManagerList<Ty>& operator=(const ManagerList<Ty>& other) = delete;
	//ManagerList(ManagerList<Ty>&& other) = delete;
	//ManagerList<Ty>& operator=(ManagerList<Ty>&& other) = delete;


	void addNodeToList(void* data)noexcept
	{
		DataToReclaim<Ty>* node{ new DataToReclaim{static_cast<Ty*>(data)} };
		node->next = (this->head).load();

		//把node设为list的新head.
		while (!(this->head).compare_exchange_weak(node->next, node)); //std::memory_order_seq_cst
	}

	void deleteNodesNoHzard()noexcept
	{
		DataToReclaim<Ty>* current_node{ (this->head).exchange(nullptr) }; //std::memory_order_seq_cst

		while (current_node != nullptr) {
			DataToReclaim<Ty>* const next{ current_node->next };

			if (!outstandHzardPointer(next->data_ptr)) {
				delete current_node;  //delete!

			} else {
				Ty* data_ptr{ current_node->data_ptr };
				current_node->data_ptr = nullptr;

				this->addNodeToList(static_cast<void*>(data_ptr));

				data_ptr = nullptr; 
			}

		}
	}

private:

	//void deleter()noexcept
	//{
	//	DataToReclaim* temp_head{ (this->head).load() };//std::memory_oder_seq_cst

	//	while (temp_head != nullptr) { 
	//		delete temp_head;
	//	}

	//	temp_head = nullptr;
	//}


	static std::atomic<DataToReclaim<Ty>*> head;
};

template<typename Ty>
std::atomic<DataToReclaim<Ty>*> ManagerList<Ty>::head{ nullptr };

#endif //RECLAIM_LIST_H
