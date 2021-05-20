#pragma once

#include <list>
#include "NBTTag.h"
#include "Enums.h"

namespace mcbot
{
	class NBTList
	{
	private:
		NBTType type;
		std::list<NBTTag> elements;

	public:
		NBTList(NBTType type);
		NBTList();

		void add_element(NBTTag tag);

		NBTType get_type();
		std::list<NBTTag> get_elements();
	};
}

