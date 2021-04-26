#pragma once

#include <list>
#include <boost/any.hpp>

namespace mcbot
{
	class EntityMetaData
	{
	private:
		std::list<boost::any> values;

	public:
		void add_value(boost::any value);

		std::list<boost::any> get_values();
	};
}

