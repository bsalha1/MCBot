#include "NBTTagCompound.h"
#include "Enums.h"
#include "StringUtils.h"

#include <typeinfo>

namespace mcbot
{

	NBTTagCompound::NBTTagCompound()
	{
	}

	void NBTTagCompound::AddTag(NBTTag tag)
	{
		this->tags.push_back(tag);
	}

	void NBTTagCompound::Serialize(Packet& packet) const
	{
		for (auto tag : this->tags)
		{
			tag.Serialize(packet);
		}
	}

	std::string InsertSpaces(int num_spaces)
	{
		std::string str = "";

		for (int i = 0; i < num_spaces; i++)
		{
			str += " ";
		}

		return str;
	}

	std::string NBTTagCompound::ToString()
	{
		std::string str = "";

		//std::string spaces = insert_spaces(pretty_pointer);
		//for (auto tag : this->tags)
		//{
		//	str += spaces + StringUtils::to_string(tag.get_type()) + "\n";
		//}

		return str;
	}
}

