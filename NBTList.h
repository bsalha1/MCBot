#pragma once

#include <list>
#include "Enums.h"

namespace mcbot
{ 
	template<class T>
	class NBTList
	{
	private:
		NBTType type;
		std::list<T> elements;

	public:
		NBTList(NBTType type);
		NBTList();

		NBTType get_type();

		std::list<T> get_elements();
	};

	template<class T>
	inline NBTList<T>::NBTList(NBTType type)
	{
		this->type = type;
	}

	template<class T>
	inline NBTList<T>::NBTList()
	{
		this->type = NBTType::UNKNOWN;
	}

	template<class T>
	inline NBTType NBTList<T>::get_type()
	{
		return this->type;
	}

	template<class T>
	inline std::list<T> NBTList<T>::get_elements()
	{
		return this->elements;
	}
}

