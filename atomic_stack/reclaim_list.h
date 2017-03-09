#ifndef RECLAIM_LIST_H
#define RECLAIM_LIST_H

#include <atomic>
#include <exception>

//defined hzard-pointer
#include "hzard_pointer.h"



//��stack�е�һ����㱻�����߳�ʹ�õ�ʱ��Ϊ�˷�ֹ�ý�㱻���е��̸߳�ɾ��
//��˰Ѹý����뵽 list��.
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


//���н�㽫Ҫ��ɾ����ʱ�����ȼ�������߳��Ƿ�����ʹ�øý��
//�������ʹ����ô�ѽ����뵽��list.
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

		//��node��Ϊlist����head.
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
