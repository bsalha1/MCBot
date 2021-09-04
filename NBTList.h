#pragma once

#include <list>
#include "NBTTag.h"
#include "Enums.h"

namespace McBot
{
	class NBTList
	{
	private:
		NBTType type;
		std::list<NBTTag> elements;

	public:
		NBTList(NBTType type);
		NBTList();

		void AddElement(NBTTag tag);

		NBTType GetType();
		std::list<NBTTag> GetElements();
	};
}

