#include "NBTList.h"

namespace mcbot
{

    NBTList::NBTList(NBTType type)
    {
        this->type = type;
    }

    NBTList::NBTList()
    {
        this->type = NBTType::UNKNOWN;
    }

    void NBTList::AddElement(NBTTag tag)
    {
        this->elements.push_back(tag);
    }

    NBTType NBTList::GetType()
    {
        return this->type;
    }

    std::list<NBTTag> NBTList::GetElements()
    {
        return this->elements;
    }
}
