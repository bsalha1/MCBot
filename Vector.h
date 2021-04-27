#pragma once
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

		T get_x();
		T get_y();
		T get_z();

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
	inline std::string Vector<T>::to_string()
	{
		return "(" + std::to_string(this->x) + "," + std::to_string(this->y) + "," + std::to_string(this->z) + ")";
	}
}

