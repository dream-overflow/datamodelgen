/**
 * @file memberbool.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberbool.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberBool>::R memberBool;

MemberBool::MemberBool(Member *parent) :
    MemberHelper(parent)
{
}

String MemberBool::getTypeName() const
{
    return "bool";
}

String MemberBool::getOutTypeName() const
{
    return "o3d::Bool";
}

String MemberBool::getReadMethod() const
{
    return "readBool";
}

String MemberBool::getWriteMethod() const
{
    return "writeBool";
}

String MemberBool::getSetTo() const
{
    return " = True";
}

UInt32 MemberBool::getMinSize() const
{
    return 1;
}

String MemberBool::getSizeOf() const
{
    return "1";
}
