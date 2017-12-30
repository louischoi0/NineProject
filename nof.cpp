
#include "stdafx.h"
#include "nof.h"

const std::map<int, int> A::sizeMap = A::create_map();

notifier::notifier()
{
	notification[0].set_params(dType::String, dType::String);
	
	notification[0].set_args( 0, "ID");
	notification[0].set_args( 1, "PW");


	notification[1].set_params(dType::Int, dType::Int, dType::String);
	notification[1].set_args(0, "TableKey");
	notification[1].set_args(1, "SenderKey");
	notification[1].set_args(2, "Text");


}

bool notifier::any()
{
	auto it = notification.begin();

	for (; it != notification.end(); it++)
	{
		if (it->get_new())
			return true;
	}

	return false;

}