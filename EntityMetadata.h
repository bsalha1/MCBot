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
		void AddValue(boost::any value);

		std::list<boost::any> GetValues();
	};
}

