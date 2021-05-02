#pragma once

#include <iostream>
#include <string>

namespace mcbot
{
	template <class T>
	class Buffer
	{
	private:
		T* arr;
		int max_size;
		int current_size;

	public:
		Buffer();
		Buffer(T* arr, int max_size);
		Buffer(int max_size);

		~Buffer();

		void put(T element);
		T& get(int i);

		T* get_array();

		int get_current_size();

		int get_max_size();

		std::string to_string();
	};

	template<class T>
	mcbot::Buffer<T>::Buffer()
	{
	}

	template<typename T>
	inline mcbot::Buffer<T>::Buffer(T* arr, int size)
	{
		this->arr = new T[size];
		this->max_size = size;
		this->current_size = size;

		for (int i = 0; i < size; i++)
		{
			this->arr[i] = arr[i];
		}
	}

	template<class T>
	mcbot::Buffer<T>::Buffer(int size)
	{
		this->arr = new T[size]{ 0 };
		this->max_size = size;
		this->current_size = 0;
	}

	template<class T>
	mcbot::Buffer<T>::~Buffer()
	{
		//delete arr;
	}

	template<class T>
	void mcbot::Buffer<T>::put(T element)
	{
		if (current_size >= max_size)
		{
			std::cerr << "Array overflow halted" << std::endl;
			return;
		}

		this->arr[current_size++] = element;
	}

	template<class T>
	inline T* mcbot::Buffer<T>::get_array()
	{
		return this->arr;
	}

	template<class T>
	inline T& mcbot::Buffer<T>::get(int i)
	{
		return this->arr[i];
	}

	template<class T>
	int mcbot::Buffer<T>::get_current_size()
	{
		return this->current_size;
	}

	template<class T>
	inline int mcbot::Buffer<T>::get_max_size()
	{
		return this->max_size;
	}

	template<class T>
	std::string mcbot::Buffer<T>::to_string()
	{
		std::string str = "";
		for (int i = 0; i < this->current_size; i++)
		{
			str += std::to_string(this->arr[i]) + " ";
		}
		return str;
	}

}

