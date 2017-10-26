/**
 * @file memberstaticarrayuint32.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberstaticarrayuint32.h"
#include "registermember.h"
#include <o3d/core/integer.h>
#include <o3d/core/char.h>

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberStaticArrayUInt32>::R memberStaticArrayUInt32;

MemberStaticArrayUInt32::MemberStaticArrayUInt32(Member *parent) :
    MemberHelper(parent)
{
}

String MemberStaticArrayUInt32::getTypeName() const
{
    return "uint32[]";
}

String MemberStaticArrayUInt32::getOutTypeName() const
{
    return "o3d::UInt32";
}

String MemberStaticArrayUInt32::getReadMethod() const
{
    return "read";
}

String MemberStaticArrayUInt32::getWriteMethod() const
{
    return "write";
}

T_StringList MemberStaticArrayUInt32::getHeaders() const
{
    T_StringList list;
    list.push_back("<stdio.h>");

    return list;
}

String MemberStaticArrayUInt32::getIfTest(const Member *param) const
{
    O3D_ASSERT(param != nullptr);
    return "[" + param->getName() + "]";
}

UInt32 MemberStaticArrayUInt32::getNewUIntId()
{
    return m_uintId.getID();
}

void MemberStaticArrayUInt32::writeDecl(OutStream *os)
{
    Int32 ident = 1;
    Member *parent = getParent();
    while (parent != nullptr)
    {
        ident += parent->getIdent();
        parent = parent->getParent();
    }

    String line;
    for (Int32 i = 0; i < ident; ++i)
    {
        line += "    ";
    }
    line += getOutTypeName() + " " + getName() + "[" + m_value + "];";

    os->writeLine(line);
}

void MemberStaticArrayUInt32::writeSetterDecl(OutStream *os)
{
    String line("    ");

    String name = getName();
    if (name.startsWith("_"))
        name.remove(0, 1);
    if (name.startsWith("m_"))
        name.remove(0, 2);
    String mname = name;
    name[0] = WideChar::toUpper(name[0]);

    line += String("void set") + name + "(const " + getOutTypeName() + " *_" + mname + ")";
    os->writeLine(line);

    os->writeLine("    {");
    line = "        memcpy(" + getName() + ", _" + mname + ", " + m_value + ");";
    os->writeLine(line);

    os->writeLine("    }");
    os->writeLine("");
}

void MemberStaticArrayUInt32::writeSetterImpl(OutStream *os)
{

}

void MemberStaticArrayUInt32::writeGetterDecl(OutStream *os)
{
    String line("    ");

    String name = getName();
    if (name.startsWith("m_"))
        name.remove(0, 2);
    name[0] = WideChar::toUpper(name[0]);

    line += "const " + getOutTypeName() + "* get" + name + "() const";
    os->writeLine(line);

    os->writeLine("    {");
    line = "        return " + getName() + ";";
    os->writeLine(line);
    os->writeLine("    }");
    os->writeLine("");
}

void MemberStaticArrayUInt32::writeGetterImpl(OutStream *os)
{

}

void MemberStaticArrayUInt32::writeRead(OutStream *os)
{
    Int32 ident = 1;
    Member *parent = getParent();
    while (parent != nullptr)
    {
        ident += parent->getIdent();
        parent = parent->getParent();
    }

    String identStr;
    for (Int32 i = 0; i < ident; ++i)
    {
        identStr += "    ";
    }

    // content
    String line = identStr + "is." + getReadMethod() +
            "(" +
            getName() + ", " +
            m_value +
            ");";

    os->writeLine(line);
}

void MemberStaticArrayUInt32::writeWrite(OutStream *os)
{
    Int32 ident = 1;
    Member *parent = getParent();
    while (parent != nullptr)
    {
        ident += parent->getIdent();
        parent = parent->getParent();
    }

    String identStr;
    for (Int32 i = 0; i < ident; ++i)
    {
        identStr += "    ";
    }

    // content
    String line = identStr + "os." + getWriteMethod() +
            "(" +
            getName() + ", " +
            m_value +
            ");";

    os->writeLine(line);
}

Bool MemberStaticArrayUInt32::isRef() const
{
    return True;
}

UInt32 MemberStaticArrayUInt32::getMinSize() const
{
    return UInteger32::parseInteger(m_value);
}

String MemberStaticArrayUInt32::getSizeOf() const
{
    return m_value;
}
