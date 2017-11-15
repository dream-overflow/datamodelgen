/**
 * @file memberint64.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberint64.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberInt64>::R memberInt64;

MemberInt64::MemberInt64(Member *parent) :
    MemberHelper(parent)
{
}

String MemberInt64::getTypeName() const
{
    return "int64";
}

String MemberInt64::getOutTypeName() const
{
    return "o3d::Int64";
}

String MemberInt64::getReadMethod() const
{
    return "readInt64";
}

String MemberInt64::getWriteMethod() const
{
    return "writeInt64";
}

UInt32 MemberInt64::getMinSize() const
{
    return 8;
}

String MemberInt64::getSizeOf() const
{
    return "8";
}
