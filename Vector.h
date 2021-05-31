#pragma once

#include <string>

namespace mcbot
{
	template<typename T> class Vector;

	template <typename T> Vector<T> operator+(const Vector<T>& lhs, const Vector<T>& rhs);
	template <typename T> Vector<T> operator-(const Vector<T>& lhs, const Vector<T>& rhs);

	template<typename T>
	class Vector
	{
	private:
		T x;
		T y;
		T z;
	public:
		Vector(T x, T y, T z);
		Vector(T x, T y);
		Vector();

		void Scale(T factor);
		void Floor();

		T Magnitude();

		T Distance(Vector<T> destination);

		T GetX();
		T GetY();
		T GetZ();

		void SetX(T x);
		void SetY(T y);
		void SetZ(T z);

		std::string ToString();

		friend Vector operator+<T>(const Vector& vec1, const Vector& vec2);
		friend Vector operator-<T>(const Vector& vec1, const Vector& vec2);
	};

	template<typename T>
	inline Vector<T>::Vector(T x, T y, T z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	template<typename T>
	inline Vector<T>::Vector(T x, T y)
	{
		this->x = x;
		this->y = y;
		this->z = 0;
	}

	template<typename T>
	inline Vector<T>::Vector()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	template<typename T>
	inline void Vector<T>::Scale(T factor)
	{
		this->x *= factor;
		this->y *= factor;
		this->z *= factor;
	}

	template<typename T>
	inline void Vector<T>::Floor()
	{
		if (!std::is_same<T, double>() && !std::is_same<T, float>())
		{
			std::cerr << "Failed to floor vector: not float or double type" << std::endl;
			return;
		}
		this->x = std::floor(this->x);
		this->y = std::floor(this->y);
		this->z = std::floor(this->z);
	}

	template<typename T>
	inline T Vector<T>::Magnitude()
	{
		return sqrt(x * x + y * y + z * z);
	}


	template<typename T>
	inline T Vector<T>::Distance(Vector<T> destination)
	{
		T dx = this->x - destination.x;
		T dy = this->y - destination.y;
		T dz = this->z - destination.z;
		return sqrt(dx * dx + dy * dy + dz * dz);
	}

	template<typename T>
	inline T Vector<T>::GetX()
	{
		return this->x;
	}

	template<typename T>
	inline T Vector<T>::GetY()
	{
		return this->y;
	}

	template<typename T>
	inline T Vector<T>::GetZ()
	{
		return this->z;
	}

	template<typename T>
	inline void Vector<T>::SetX(T x)
	{
		this->x = x;
	}

	template<typename T>
	inline void Vector<T>::SetY(T y)
	{
		this->y = y;
	}

	template<typename T>
	inline void Vector<T>::SetZ(T z)
	{
		this->z = z;
	}

	template <typename T>
	inline Vector<T> operator+(const Vector<T>& lhs, const Vector<T>& rhs)
	{
		Vector<T> result;
		result.x = lhs.x + rhs.x;
		result.y = lhs.y + rhs.y;
		result.z = lhs.z + rhs.z;
		return result;
	}

	template<typename T>
	inline Vector<T> operator-(const Vector<T>& lhs, const Vector<T>& rhs)
	{
		Vector<T> result;
		result.x = lhs.x - rhs.x;
		result.y = lhs.y - rhs.y;
		result.z = lhs.z - rhs.z;
		return result;
	}
	
	template<typename T>
	inline std::string Vector<T>::ToString()
	{
		return "(" + std::to_string(this->x) + "," + std::to_string(this->y) + "," + std::to_string(this->z) + ")";
	}
}

