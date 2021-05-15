#pragma once

#include <iostream>
#include <boost/any.hpp>

#include "Enums.h"

namespace mcbot
{
	class NBTTag
	{
	private:
		NBTType type;
		std::string name;
		boost::any value;

	public:
		NBTTag(NBTType type, std::string name, boost::any value);
		NBTTag(NBTType type, std::string name);
		NBTTag();

		std::string get_name();

		boost::any get_value();
	};
}

