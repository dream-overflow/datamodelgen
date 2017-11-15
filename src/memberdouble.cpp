/**
 * @file memberdouble.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberdouble.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberDouble>::R memberDouble;

MemberDouble::MemberDouble(Member *parent) :
    MemberHelper(parent)
{
}

String MemberDouble::getTypeName() const
{
    return "double";
}

String MemberDouble::getOutTypeName() const
{
    return "o3d::Double";
}

String MemberDouble::getReadMethod() const
{
    return "readDouble";
}

String MemberDouble::getWriteMethod() const
{
    return "writeDouble";
}

UInt32 MemberDouble::getMinSize() const
{
    return 8;
}

String MemberDouble::getSizeOf() const
{
    return "8";
}
