#pragma once

#include <iostream>
#include <boost/any.hpp>

#include "Serializable.h"
#include "Enums.h"

namespace mcbot
{
	class NBTTag : Serializable
	{
	private:
		NBTType type;
		std::string name;
		boost::any value;

	public:
		NBTTag(NBTType type, std::string name, boost::any value);
		NBTTag(NBTType type, std::string name);
		NBTTag();

		void serialize(uint8_t* packet, size_t& offset) override;

		std::string get_name();
		NBTType get_type();
		boost::any get_value();
	};
}

