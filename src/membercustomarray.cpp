/**
 * @file membercustomarray.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "membercustomarray.h"

using namespace o3d;
using namespace o3d::dmg;

// manual registration

MemberCustomArray::MemberCustomArray(const MemberCustomArray &dup, Member *parent) :
    MemberCustom(dup, parent)
{

}

MemberCustomArray::MemberCustomArray(Member *parent) :
    MemberCustom(parent)
{
    m_headers.push_back("<vector>");
}

String MemberCustomArray::getOutTypeName() const
{
    return "std::vector<" + m_outTypeName + ">";
}

String MemberCustomArray::getReadMethod() const
{
    return "";
}

String MemberCustomArray::getWriteMethod() const
{
    return "";
}

void MemberCustomArray::setHeaders(const T_StringList &headers)
{
    m_headers = headers;
    m_headers.push_back("<vector>");
}

void MemberCustomArray::writeSetterDecl(OutStream *os)
{
    String identStr("    ");

    String name = getName();
    if (name.startsWith("_"))
        name.remove(0, 1);
    if (name.startsWith("m_"))
        name.remove(0, 2);
    String mname = name;
    if (name.length() >= 1 && name[0] >= 'a' && name[0] <= 'z')
        name[0] = name[0] - ('a' - 'A');

    // full setter
    os->writeLine(identStr + "void set" + name + "(const " + getOutTypeName() + " &" + mname + ")");
    os->writeLine(identStr + "{");
    os->writeLine(identStr + identStr + getPrefixedName() + " = " + mname + ";");
    os->writeLine(identStr + "}");
    os->writeLine("");

    // and add a write getter
    os->writeLine(identStr + getOutTypeName() + "& get" + name + "()");
    os->writeLine(identStr + "{");
    os->writeLine(identStr + identStr + "return " + getPrefixedName() + ";");
    os->writeLine(identStr + "}");
    os->writeLine("");
}

void MemberCustomArray::writeRead(OutStream *os)
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

    String counter = getName() + "Size";
    if (counter.startsWith("m_"))
        counter.remove(0, 2);
    if (counter.startsWith("_"))
        counter.remove(0, 1);

    String prefixedName = getPrefixedName();

    os->writeLine(identStr + "o3d::UInt32 " + counter + " = 0;");
    os->writeLine(identStr + counter + " = is.readUInt32();");
    os->writeLine(identStr + prefixedName + ".resize(" + counter + ");");
    os->writeLine(identStr + "for (o3d::UInt32 i = 0; i < " + counter + "; ++i)");
    os->writeLine(identStr + "{");
    os->writeLine(identStr + identStr + prefixedName + "[i].readFromFile(is);");
    os->writeLine(identStr + "}");
    os->writeLine(identStr);
}

void MemberCustomArray::writeWrite(OutStream *os)
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

    String counter = getName() + "Size";
    if (counter.startsWith("m_"))
        counter.remove(0, 2);
    if (counter.startsWith("_"))
        counter.remove(0, 1);

    String prefixedName = getPrefixedName();

    os->writeLine(identStr + "o3d::UInt32 " + counter + " = 0;");
    os->writeLine(identStr + counter + " = (o3d::UInt32)" + prefixedName + ".size();");
    os->writeLine(identStr + "is.writeUInt32(" + counter + ");");
    os->writeLine(identStr + "for (o3d::UInt32 i = 0; i < " + counter + "; ++i)");
    os->writeLine(identStr + "{");
    os->writeLine(identStr + identStr + prefixedName + "[i].writeToFile(os);");
    os->writeLine(identStr + "}");
    os->writeLine(identStr);
}

void MemberCustomArray::writeSetterImpl(OutStream *os)
{

}

void MemberCustomArray::writeFinalize(Context &ctx)
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

    // for each
    ctx.os->writeLine(identStr + "for (" + m_outTypeName + " &v : " + getPrefixedName() + ")");
    ctx.os->writeLine(identStr + "{");
    ctx.os->writeLine(identStr + identStr + "v.finalize();");
    ctx.os->writeLine(identStr + "}");
    ctx.os->writeLine(identStr);
}

UInt32 MemberCustomArray::getMinSize() const
{
    return 2;
}

String MemberCustomArray::getSizeOf() const
{
    return getPrefixedName() + ".size() + 4";
}
