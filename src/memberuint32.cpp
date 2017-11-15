/**
 * @file memberuint32.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberuint32.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberUInt32>::R memberUInt32;

MemberUInt32::MemberUInt32(Member *parent) :
    MemberHelper(parent)
{
}

String MemberUInt32::getTypeName() const
{
    return "uint32";
}

String MemberUInt32::getOutTypeName() const
{
    return "o3d::UInt32";
}

String MemberUInt32::getReadMethod() const
{
    return "readUInt32";
}

String MemberUInt32::getWriteMethod() const
{
    return "writeUInt32";
}

UInt32 MemberUInt32::getMinSize() const
{
    return 4;
}

String MemberUInt32::getSizeOf() const
{
    return "4";
}
