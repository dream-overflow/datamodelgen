/**
 * @file memberfloat.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberfloat.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberFloat>::R memberFloat;

MemberFloat::MemberFloat(Member *parent) :
    MemberHelper(parent)
{
}

String MemberFloat::getTypeName() const
{
    return "float";
}

String MemberFloat::getOutTypeName() const
{
    return "o3d::Float";
}

String MemberFloat::getReadMethod() const
{
    return "readFloat";
}

String MemberFloat::getWriteMethod() const
{
    return "readFloat";
}

UInt32 MemberFloat::getMinSize() const
{
    return 4;
}

String MemberFloat::getSizeOf() const
{
    return "4";
}
