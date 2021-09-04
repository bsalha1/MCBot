#pragma once

#include <iostream>
#include <string>

namespace McBot
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

		template<unsigned int I>
		std::array<T, I> to_array();

		T operator[](int i) const
		{
			return arr[i];
		}
		
		T& operator[](int i)
		{
			return arr[i];
		}
	};

	template<class T>
	McBot::Buffer<T>::Buffer()
	{
	}

	template<typename T>
	inline McBot::Buffer<T>::Buffer(T* arr, int size)
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
	McBot::Buffer<T>::Buffer(int size)
	{
		this->arr = new T[size]{ 0 };
		this->max_size = size;
		this->current_size = 0;
	}

	template<class T>
	McBot::Buffer<T>::~Buffer()
	{
		//delete arr;
	}

	template<class T>
	void McBot::Buffer<T>::put(T element)
	{
		if (current_size >= max_size)
		{
			std::cerr << "Array overflow halted" << std::endl;
			return;
		}

		this->arr[current_size++] = element;
	}

	template<class T>
	inline T* McBot::Buffer<T>::get_array()
	{
		return this->arr;
	}

	template<class T>
	inline T& McBot::Buffer<T>::get(int i)
	{
		return this->arr[i];
	}

	template<class T>
	int McBot::Buffer<T>::get_current_size()
	{
		return this->current_size;
	}

	template<class T>
	inline int McBot::Buffer<T>::get_max_size()
	{
		return this->max_size;
	}

	template<class T>
	std::string McBot::Buffer<T>::to_string()
	{
		std::string str = "";
		for (int i = 0; i < this->current_size; i++)
		{
			str += std::to_string(this->arr[i]) + " ";
		}
		return str;
	}

	template<class T> template<unsigned int I>
	inline std::array<T, I> Buffer<T>::to_array()
	{
		if (I > this->max_size)
		{
			std::cerr << "Size mismatch" << std::endl;
		}

		std::array<T, I> arr;

		for (int i = 0; i < I; i++)
		{
			arr[i] = this->arr[i];
		}

		return arr;
	}

}

