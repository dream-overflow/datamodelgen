/**
 * @file memberconstint16.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberconstint16.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberConstInt16>::R memberConstInt16;

MemberConstInt16::MemberConstInt16(Member *parent) :
    MemberHelper(parent)
{
}

String MemberConstInt16::getTypeName() const
{
    return "const int16";
}

String MemberConstInt16::getOutTypeName() const
{
    return "o3d::Int16";
}

String MemberConstInt16::getReadMethod() const
{
    return "";
}

String MemberConstInt16::getWriteMethod() const
{
    return "";
}

void MemberConstInt16::writeDecl(OutStream *os)
{
    String line = "    ";
    line += String("static const ") + getOutTypeName() + " " + getName() + " = " + m_value + ";";

    os->writeLine(line);
}

void MemberConstInt16::writeRead(OutStream *os)
{
    // nothing
}

void MemberConstInt16::writeWrite(OutStream *os)
{
    // nothing
}

void MemberConstInt16::writeSetterDecl(OutStream *os)
{
    // nothing
}

void MemberConstInt16::writeSetterImpl(OutStream *os)
{
    // nothing
}

void MemberConstInt16::writeGetterDecl(OutStream *os)
{
    // nothing
}

void MemberConstInt16::writeGetterImpl(OutStream *os)
{
    // nothing
}
