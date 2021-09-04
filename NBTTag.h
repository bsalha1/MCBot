#pragma once

#include <iostream>
#include <boost/any.hpp>

#include "Serializable.h"
#include "Enums.h"

namespace McBot
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

		void Serialize(Packet& packet) const override;

		std::string GetName();
		NBTType GetType();
		boost::any GetValue();
	};
}

