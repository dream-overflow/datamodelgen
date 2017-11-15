/**
 * @file memberconstint8.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberconstint8.h"
#include "registermember.h"
#include <o3d/core/file.h>

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberConstInt8>::R memberConstInt8;

MemberConstInt8::MemberConstInt8(Member *parent) :
    MemberHelper(parent)
{
}

String MemberConstInt8::getTypeName() const
{
    return "const int8";
}

String MemberConstInt8::getOutTypeName() const
{
    return "o3d::Int8";
}

String MemberConstInt8::getReadMethod() const
{
    return "";
}

String MemberConstInt8::getWriteMethod() const
{
    return "";
}

void MemberConstInt8::writeDecl(OutStream *os)
{
    String line = "    ";
    line += String("static const ") + getOutTypeName() + " " + getName() + " = " + m_value + ";";

    os->writeLine(line);
}

void MemberConstInt8::writeRead(OutStream *os)
{
    // nothing
}

void MemberConstInt8::writeWrite(OutStream *os)
{
    // nothing
}

void MemberConstInt8::writeSetterDecl(OutStream *os)
{
    // nothing
}

void MemberConstInt8::writeSetterImpl(OutStream *os)
{
    // nothing
}

void MemberConstInt8::writeGetterDecl(OutStream *os)
{
    // nothing
}

void MemberConstInt8::writeGetterImpl(OutStream *os)
{
    // nothing
}
