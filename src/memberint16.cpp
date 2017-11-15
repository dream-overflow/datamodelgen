/**
 * @file memberint16.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberint16.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberInt16>::R memberInt16;

MemberInt16::MemberInt16(Member *parent) :
    MemberHelper(parent)
{
}

String MemberInt16::getTypeName() const
{
    return "int16";
}

String MemberInt16::getOutTypeName() const
{
    return "o3d::Int16";
}

String MemberInt16::getReadMethod() const
{
    return "readInt16";
}

String MemberInt16::getWriteMethod() const
{
    return "writeInt16";
}

UInt32 MemberInt16::getMinSize() const
{
    return 2;
}

String MemberInt16::getSizeOf() const
{
    return "2";
}
