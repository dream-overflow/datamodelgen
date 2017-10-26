/**
 * @file memberarray8.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberarray8.h"
#include "registermember.h"
#include <o3d/core/file.h>

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberArray8>::R memberArray8;

MemberArray8::MemberArray8(Member *parent) :
    MemberHelper(parent)
{
}

String MemberArray8::getTypeName() const
{
    return "array8";
}

String MemberArray8::getOutTypeName() const
{
    return "o3d::SmartArrayUInt8";
}

String MemberArray8::getReadMethod() const
{
    return "read";
}

String MemberArray8::getWriteMethod() const
{
    return "write";
}

T_StringList MemberArray8::getHeaders() const
{
    T_StringList list;
    list.push_back("<o3d/core/SmartArray.h>");

    return list;
}

String MemberArray8::getIfTest(const Member *param) const
{
    O3D_ASSERT(param != nullptr);
    return "[" + param->getName() + "]";
}

UInt32 MemberArray8::getNewUIntId()
{
    return m_uintId.getID();
}

void MemberArray8::writeSetterDecl(OutStream *os)
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

    // size
    line = "        if (" + (isParent() ? getParent()->getPrefix() : "") + getName() + ".isValid())";
    os->writeLine(line);
    line = String("            m_messageDataSize -= ") + (isParent() ? getParent()->getPrefix() : "") + getName() + ".getSize();";
    os->writeLine(line);

    line = "        " + getName() + " = " + mname + ";";
    os->writeLine(line);

    line = String("        m_messageDataSize += ") + (isParent() ? getParent()->getPrefix() : "") + getName() + ".getSize();";
    os->writeLine(line);

    os->writeLine("    }");
    os->writeLine("");
}

void MemberArray8::writeSetterImpl(OutStream *os)
{

}

void MemberArray8::writeRead(OutStream *os)
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

    String memberName = (isParent() ? getParent()->getPrefix() : "") + getName();
    String varSizeName = (isParent() ? getParent()->getPrefix() : "") + getName() + "Size";

    // size
    String line = identStr + "UInt16 " + varSizeName + ";";
    os->writeLine(line);
    line = identStr + varSizeName + " = buffer->readUInt16();";
    os->writeLine(line);
    // content
    line = identStr + memberName + ".allocate(" + varSizeName + ");";
    os->writeLine(line);
    line = identStr + "buffer->" + getReadMethod() +
            "(" +
            memberName + ".getData(), " +
            varSizeName +
            ");";

    os->writeLine(line);
}

void MemberArray8::writeWrite(OutStream *os)
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

    // size
    String line = identStr + String("buffer->writeUInt16((UInt16)") + (isParent() ? getParent()->getPrefix() : "") + getName() + ".getSize()" + ");";
    os->writeLine(line);
    // content
    line = identStr + String("buffer->") + getWriteMethod() +
            "(" +
            (isParent() ? getParent()->getPrefix() : "") + getName() + ".getData(), " +
            (isParent() ? getParent()->getPrefix() : "") + getName() + ".getSize()" +
            ");";

    os->writeLine(line);
}

Bool MemberArray8::isRef() const
{
    return True;
}

String MemberArray8::getSetTo(const Member *param, SetValue value) const
{
    if (value == SET_TRUE)
        return ".isValid()";
    else
        return ".isEmpty()";
}

UInt32 MemberArray8::getMinSize() const
{
    return 2;
}

String MemberArray8::getSizeOf() const
{
    return "2";
}
