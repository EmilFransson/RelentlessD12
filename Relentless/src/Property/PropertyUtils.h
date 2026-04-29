#pragma once

#include "Utility/StringUtils.h"

namespace Relentless
{
#define NOTIFY_PROPERTY_CHANGED(member) \
    ((void)(this->member), this->BroadcastPropertyChanged(HashString(#member)))

#define GET_MEMBER_NAME_CHECKED(Type, member) \
    ((void)&Type::member, HashString(#member))
}