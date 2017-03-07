#ifndef ATOMIC_STACK_H
#define ATOMIC_STACK_H

#include <atomic>
#include <memory>

//defined hzard-pointer!
#include "hzard_pointer.h"

//stack�Ľ��.
template<typename T>
struct Node {
	std::shared_ptr<T> data{ nullptr };
	std::shared_ptr<Node<T>> next{ nullptr };

	Node() = default;

	Node(T&& data_)try
		:data{ std::make_shared<T>(std::move(data_)) },
		next{ nullptr } { } 
	 catch (const std::bad_alloc& error) 
	 {
		 std::cerr << error.what() << std::endl;
	}

	 Node(const T& data_)try
		 :data{ std::make_shared<T>(data_) },
		 next{ nullptr } {}
	 catch (const std::bad_alloc& error) {
		 std::cerr << error.what() << std::endl;
	 }

	~Node() = default;

	Node(Node<T>&& other)
		:data{ std::move(other.data) },
		next{ std::move(other.next) } {}

	Node<T>& operator=(Node<T>&& other)
	{
		this->data = std::move(other.data);
		this->next = std::move(other.next);

		(other.data).reset(nullptr);
		(other.next).reset(nullptr);
	}

};


template<typename T>
class AtomicStack {
public:

	AtomicStack() = default;
	~AtomicStack();

	AtomicStack(const AtomicStack<T>& other) = delete;
	AtomicStack<T>& operator=(const AtomicStack<T>& other) = delete;
	//AtomicStack(AtomicStack<T>&& other) = delete;
	//AtomicStack<T>& operator=(AtomicStack<T>&& other) = delete;

	std::shared_ptr<T> pop()
	{
		//��ȡ��ǰ�̵߳� HzardPointer;
		//HzardPointer�����ж��Ƿ��������߳��Ƿ���ʹ���������ɾ���Ľ��.
		std::atomic<void*>& hd_pointer = getHzardPointerForThisThread();

		Node<T>* old_head{ (this->head).load() }; //std::memory_order_seq_cst

		do {
			Node<T>* temp_node{ nullptr };

			do {
				temp_node = old_head;

				//ֱ��������ָ����Ϊ��Ҫ��ɾ����head.
				hd_pointer.store(static_cast<void*>(temp_node)); //std::memory_order_seq_cst
				old_head = (this->head).load(); //std::memory_order_seq_cst

			} while (old_head != temp_node);

		} while (old_head && !(this->head).compare_exchange_strong(old_head, old_head->next)); //std::memory_order_seq_cst

		hd_pointer.store(nullptr);
	}

	void push(T&& value)noexcept
	{
		Node<T>* new_node{ new Node<int>{std::move(value)} };
		new_node->next = (this->head).load(); //std::memory_order_seq_cst

		while (!(this->head).compare_exchange_strong(new_node->next, new_node));

	}

	void push(const T& value)noexcept
	{
		Node<T>* new_node{ new Node<int>{value} };
		new_node->next = (this->head).load();

		while (!(this->head).compare_exchange_strong(new_node->next, new_node));
	}

	void push(const T& value)noexcept
	{

	}

private:
	std::atomic<Node<T>*> head{ nullptr };

};

#endif //ATOMIC_STACK_H