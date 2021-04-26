#include "EntityMetadata.h"

void mcbot::EntityMetaData::add_value(boost::any value)
{
	this->values.push_back(value);
}

std::list<boost::any> mcbot::EntityMetaData::get_values()
{
	return this->values;
}
