/**
 * @file memberint8.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberint8.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberInt8>::R memberInt8;

MemberInt8::MemberInt8(Member *parent) :
    MemberHelper(parent)
{
}

String MemberInt8::getTypeName() const
{
    return "int8";
}

String MemberInt8::getOutTypeName() const
{
    return "o3d::Int8";
}

String MemberInt8::getReadMethod() const
{
    return "readInt8";
}

String MemberInt8::getWriteMethod() const
{
    return "writeInt8";
}

UInt32 MemberInt8::getMinSize() const
{
    return 1;
}

String MemberInt8::getSizeOf() const
{
    return "1";
}
