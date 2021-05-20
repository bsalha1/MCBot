#include "NBTList.h"

mcbot::NBTList::NBTList(NBTType type)
{
    this->type = type;
}

mcbot::NBTList::NBTList()
{
    this->type = NBTType::UNKNOWN;
}

void mcbot::NBTList::add_element(NBTTag tag)
{
    this->elements.push_back(tag);
}

mcbot::NBTType mcbot::NBTList::get_type()
{
    return this->type;
}

std::list<mcbot::NBTTag> mcbot::NBTList::get_elements()
{
    return this->elements;
}
