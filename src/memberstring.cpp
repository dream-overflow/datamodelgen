/**
 * @file memberstring.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberstring.h"
#include "registermember.h"
#include <o3d/core/file.h>

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberString>::R memberString;

MemberString::MemberString(Member *parent) :
    MemberHelper(parent)
{
}

String MemberString::getTypeName() const
{
    return "string";
}

String MemberString::getOutTypeName() const
{
    return "o3d::String";
}

String MemberString::getReadMethod() const
{
    return "readFromFile";
}

String MemberString::getWriteMethod() const
{
    return "writeToFile";
}

T_StringList MemberString::getHeaders() const
{
    T_StringList list;
    list.push_back("<o3d/core/String.h>");

    return list;
}

void MemberString::writeSetterDecl(OutStream *os)
{
    String line("    ");

    String name = getName();
    if (name.startsWith("_"))
        name.remove(0, 1);
    if (name.startsWith("m_"))
        name.remove(0, 2);
    String mname = name;
    if (name.length() >= 1 && name[0] >= 'a' && name[0] <= 'z')
        name[0] = name[0] - ('a' - 'A');

    line += String("void set") + name + "(const " + getOutTypeName() + " &" + mname + ")";
    os->writeLine(line);

    os->writeLine("    {");
    line = "        " + getName() + " = " + mname + ";";
    os->writeLine(line);
    os->writeLine("    }");
    os->writeLine("");
}

void MemberString::writeRead(OutStream *os)
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

    line += (isParent() ? getParent()->getPrefix() : "") + getName() + ".readFromFile(is);";

    os->writeLine(line);
}

void MemberString::writeWrite(OutStream *os)
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

    String prefixedName = (isParent() ? getParent()->getPrefix() : "") + getName();

    line += (isParent() ? getParent()->getPrefix() : "") + getName() + ".writeToFile(os);";

    os->writeLine(line);
}

void MemberString::writeSetterImpl(OutStream *os)
{

}

Bool MemberString::isRef() const
{
    return True;
}

UInt32 MemberString::getMinSize() const
{
    return 2;
}

String MemberString::getSizeOf() const
{
    return getName() + ".length() + 2";
}
