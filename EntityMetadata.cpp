#include "EntityMetadata.h"

namespace McBot
{

	void EntityMetaData::AddValue(boost::any value)
	{
		this->values.push_back(value);
	}

	std::list<boost::any> EntityMetaData::GetValues()
	{
		return this->values;
	}
}