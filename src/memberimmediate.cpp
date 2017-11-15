/**
 * @file memberimmediate.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberimmediate.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberImmediate>::R memberImmediate;

MemberImmediate::MemberImmediate(Member *parent) :
    MemberHelper(parent)
{
}

String MemberImmediate::getTypeName() const
{
    return "immediate";
}

String MemberImmediate::getOutTypeName() const
{
    return "";
}

String MemberImmediate::getReadMethod() const
{
    return "";
}

String MemberImmediate::getWriteMethod() const
{
    return "";
}

void MemberImmediate::writeDecl(OutStream *os)
{

}

void MemberImmediate::writeRead(OutStream *os)
{

}

void MemberImmediate::writeWrite(OutStream *os)
{

}

void MemberImmediate::writeSetterDecl(OutStream *os)
{

}

void MemberImmediate::writeSetterImpl(OutStream *os)
{

}
