#pragma once

#include <map>

namespace McBot
{
	template<class K, class V>
	class Registry
	{
	private:
		std::map<K, V> reg;
		std::list<V> values;

	public:
		void RegisterValue(K id, V value);
		
		void RemoveValue(K id);
		
		bool IsValueRegistered(K id);

		V& GetValue(K id);

		std::list<V> GetValues();
	};
	
	template<class K, class V>
	inline void Registry<K, V>::RegisterValue(K id, V value)
	{
		this->reg.insert(std::pair<K, V>(id, value));
	}

	template<class K, class V>
	inline void Registry<K, V>::RemoveValue(K id)
	{
		this->reg.erase(id);
	}

	template<class K, class V>
	inline bool Registry<K, V>::IsValueRegistered(K id)
	{
		return this->reg.find(id) != this->reg.end();
	}

	template<class K, class V>
	inline V& Registry<K, V>::GetValue(K id)
	{
		return this->reg.at(id);
	}

	template<class K, class V>
	inline std::list<V> Registry<K, V>::GetValues()
	{
		std::list<V> values;

		for (const auto& pair : this->reg)
		{
			values.push_back(pair.second);
		}

		return values;
	}
}

