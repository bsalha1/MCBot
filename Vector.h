#pragma once

#include <iostream>
#include <string>

namespace mcbot
{
	template<typename T>
	class Vector
	{
	private:
		T x;
		T y;
		T z;
	public:
		Vector(T x, T y, T z);
		Vector();

		T get_x();
		T get_y();
		T get_z();

		Vector<T> sum(Vector<T> to_add);
		Vector<T> sum(T x, T y, T z);

		std::string to_string();
	};

	template<typename T>
	inline Vector<T>::Vector(T x, T y, T z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	template<typename T>
	inline Vector<T>::Vector()
	{
	}

	template<typename T>
	inline T Vector<T>::get_x()
	{
		return this->x;
	}

	template<typename T>
	inline T Vector<T>::get_y()
	{
		return this->y;
	}

	template<typename T>
	inline T Vector<T>::get_z()
	{
		return this->z;
	}

	template<typename T>
	inline Vector<T> Vector<T>::sum(Vector<T> to_add)
	{
		Vector<T> summed;
		summed.x = this->x + to_add.x;
		summed.y = this->y + to_add.y;
		summed.z = this->z + to_add.z;
		return summed;
	}

	template<typename T>
	inline Vector<T> Vector<T>::sum(T x, T y, T z)
	{
		Vector<T> summed;
		summed.x = this->x + x;
		summed.y = this->y + y;
		summed.z = this->z + z;
		return summed;
	}
	
	template<typename T>
	inline std::string Vector<T>::to_string()
	{
		return "(" + std::to_string(this->x) + "," + std::to_string(this->y) + "," + std::to_string(this->z) + ")";
	}
}

