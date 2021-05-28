#pragma once

#include <iostream>
#include <map>
#include <list>
#include <boost/any.hpp>

#include "Serializable.h"
#include "NBTTag.h"

namespace mcbot
{
	class NBTTagCompound : Serializable
	{
	private:
		int pretty_pointer = 0;
		std::list<NBTTag> tags;

	public:
		NBTTagCompound();

		void AddTag(NBTTag tag);

		void Serialize(uint8_t* packet, size_t& offset) const override;

		std::string ToString();

	};
}

