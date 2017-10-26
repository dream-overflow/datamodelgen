/**
 * @file memberint32.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberint32.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberInt32>::R memberInt32;

MemberInt32::MemberInt32(Member *parent) :
    MemberHelper(parent)
{
}

String MemberInt32::getTypeName() const
{
    return "int32";
}

String MemberInt32::getOutTypeName() const
{
    return "o3d::Int32";
}

String MemberInt32::getReadMethod() const
{
    return "readInt32";
}

String MemberInt32::getWriteMethod() const
{
    return "writeInt32";
}

UInt32 MemberInt32::getMinSize() const
{
    return 4;
}

String MemberInt32::getSizeOf() const
{
    return "4";
}
