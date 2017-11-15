/**
 * @file memberloop.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberloop.h"
#include "registermember.h"
#include <o3d/core/char.h>
#include <o3d/core/integer.h>

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberLoop>::R memberLoop;

MemberLoop::MemberLoop(Member *parent) :
    MemberHelper(parent)
{
}

String MemberLoop::getTypeName() const
{
    return "loop";
}

String MemberLoop::getOutTypeName() const
{
    return "loop";
}

T_StringList MemberLoop::getHeaders() const
{
    T_StringList list;
    list.push_back("<o3d/core/SmartArray.h>");

    return list;
}

String MemberLoop::getReadMethod() const
{
    return "";
}

String MemberLoop::getWriteMethod() const
{
    return "";
}

void MemberLoop::writeDecl(OutStream *os)
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

    // write children as struct
    os->writeLine(identStr + "struct " + getName());
    os->writeLine(identStr + "{");

    for (Member *member : m_members)
    {
        member->writeDecl(os);
    }

    os->writeLine(identStr + "};");

    m_arrayName = getName();
    m_arrayName.lower();
    m_arrayName.insert("m_", 0);
    m_arrayName.concat("sArray");

    os->writeLine(identStr + "o3d::SmartArray<" + getName() + "> "  + m_arrayName + ";");
}

void MemberLoop::writeRead(OutStream *os)
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

    os->writeLine("");

    String resize = identStr + m_arrayName + ".allocate(" + m_var->getName() + ");";
    os->writeLine(resize);

    String loop = identStr + "for (";
    loop += m_var->getOutTypeName() + " i = 0; i < " + m_var->getName() + "; ++i)";

    os->writeLine(loop);
    os->writeLine(identStr + "{");

    // write children
    for (Member *member : m_members)
    {
        member->writeRead(os);
    }

    os->writeLine(identStr + "}");
}

void MemberLoop::writeWrite(OutStream *os)
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

    String loop = identStr + "for (";
    loop += m_var->getOutTypeName() + " i = 0; i < " + m_var->getName() + "; ++i)";

    os->writeLine("");
    os->writeLine(loop);
    os->writeLine(identStr + "{");

    // write children
    for (Member *member : m_members)
    {
        member->writeWrite(os);
    }

    os->writeLine(identStr + "}");
}

void MemberLoop::setCond(Member *var, Member *varParam)
{
    m_var = var;
    m_varParam = varParam;
}

void MemberLoop::addMember(Member *member)
{
    O3D_ASSERT(member->getParent() == this);
    m_members.push_back(member);
}

Member *MemberLoop::findMember(const String &name) const
{
    for (Member *child : m_members)
    {
        if (child->getName() == name)
            return child;
    }

    if (getParent())
        return getParent()->findMember(name);

    return nullptr;
}

UInt32 MemberLoop::getIdent() const
{
    return 1;
}

String MemberLoop::getPrefix() const
{
    return m_arrayName + "[i].";
}

void MemberLoop::writeSetterDecl(OutStream *os)
{
    // SET
    String line("    ");

    // var name
    String mname = m_var->getName();
    if (mname.startsWith("m_"))
        mname.remove(0, 2);
    if (mname.length() >= 1)
        mname[0] = WideChar::toLower(mname[0]);
    mname.prepend('_');

    // foo name
    String name = getName();
    if (name.startsWith("m_"))
        name.remove(0, 2);
    if (name.length() >= 1)
        name[0] = WideChar::toUpper(name[0]);

    line += String("void alloc") + name + "s()";
    os->writeLine(line);

    os->writeLine("    {");

    // allocate
    line = "        if (" + m_arrayName + ".isValid())";
    os->writeLine(line);
    line = String("            m_messageDataSize -= ") + m_arrayName + ".getSizeInBytes();";
    os->writeLine(line);

    line = "        " + m_arrayName + ".allocate(" + m_var->getName() + ");";
    os->writeLine(line);

    line = String("        m_messageDataSize += ") + m_arrayName + ".getSizeInBytes();";
    os->writeLine(line);

    os->writeLine("    }");
    os->writeLine("");

    // setter for children
    line = "    ";
    line += String("void set") + name + "(o3d::UInt32 n, ";

    // write in param
    size_t i = m_members.size();
    for (Member *member : m_members)
    {
        // nothing to do with it
        if (member->getType() == Member::TYPE_LOOP)
        {
            line.trimRight(", ");
            continue;
        }

        // nothing to do with it
        if (member->getType() == Member::TYPE_IF)
        {
            line.trimRight(", ");
            continue;
        }

        mname = member->getName();
        if (mname.startsWith("m_"))
            mname.remove(0, 2);

        mname.insert('_', 0);

        if (member->isRef())
            line += String("const ") + member->getOutTypeName() + " &" + mname;
        else
            line += member->getOutTypeName() + " " + mname;

        --i;

        if (i > 0)
            line += ", ";
    }

    line += ")";

    os->writeLine(line);

    String size = "";
    UInt32 intSize = 0;

    os->writeLine("    {");

    // write children
    Bool first = True;
    for (Member *member : m_members)
    {
        // nothing to do with it
        if (member->getType() == Member::TYPE_LOOP)
            continue;

        // nothing to do with it
        if (member->getType() == Member::TYPE_IF)
            continue;

        mname = member->getName();
        if (mname.startsWith("m_"))
            mname.remove(0, 2);

        mname.insert('_', 0);

        line = String("        ") + m_arrayName + "[n]." + member->getName() + " = " + mname + ";";
        os->writeLine(line);

        if (UInteger32::isInteger(member->getSizeOf()))
        {
            intSize += UInteger32::parseInteger(member->getSizeOf());
        }
        else
        {
            if (!first)
                size += " + ";
            else
                first = False;

            size += member->getSizeOf();
        }
    }
    if (intSize > 0)
    {
        if (first)
            size += UInteger32::toString(intSize);
        else
            size += " + " + UInteger32::toString(intSize);
    }

    os->writeLine("    }");
    os->writeLine("");
}

void MemberLoop::writeSetterImpl(OutStream *os)
{

}
