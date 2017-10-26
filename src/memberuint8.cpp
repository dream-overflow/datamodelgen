/**
 * @file memberuint8.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberuint8.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberUInt8>::R memberUInt8;

MemberUInt8::MemberUInt8(Member *parent) :
    MemberHelper(parent)
{
}

String MemberUInt8::getTypeName() const
{
    return "uint8";
}

String MemberUInt8::getOutTypeName() const
{
    return "o3d::UInt8";
}

String MemberUInt8::getReadMethod() const
{
    return "readUInt8";
}

String MemberUInt8::getWriteMethod() const
{
    return "writeUInt8";
}

UInt32 MemberUInt8::getMinSize() const
{
    return 1;
}

String MemberUInt8::getSizeOf() const
{
    return "1";
}
