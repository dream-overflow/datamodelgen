/**
 * @file memberif.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberif.h"
#include "registermember.h"
#include <o3d/core/char.h>
#include <o3d/core/integer.h>

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberIf>::R memberIf;

MemberIf::MemberIf(Member *parent) :
    MemberHelper(parent),
    m_var(nullptr)
{
}

MemberIf::~MemberIf()
{
    for (Member *member : m_members)
    {
        deletePtr(member);
    }
}

String MemberIf::getTypeName() const
{
    return "if";
}

String MemberIf::getOutTypeName() const
{
    return "if";
}

String MemberIf::getReadMethod() const
{
    return "";
}

String MemberIf::getWriteMethod() const
{
    return "";
}

void MemberIf::writeDecl(OutStream *os)
{
    // write children
    for (Member *member : m_members)
    {
        member->writeDecl(os);
    }
}

void MemberIf::writeRead(OutStream *os)
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

    String cond = identStr + "if (";
    cond += m_var->getName();
    cond += m_var->getIfTest(m_varParam);
    cond += ")";

    os->writeLine("");
    os->writeLine(cond);
    os->writeLine(identStr + "{");

    // write children
    for (Member *member : m_members)
    {
        member->writeRead(os);
    }

    os->writeLine(identStr + "}");
}

void MemberIf::writeWrite(OutStream *os)
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

    String cond = identStr + "if (";
    cond += m_var->getName();
    cond += m_var->getIfTest(m_varParam);
    cond += ")";

    os->writeLine("");
    os->writeLine(cond);
    os->writeLine(identStr + "{");

    // write children
    for (Member *member : m_members)
    {
        member->writeWrite(os);
    }

    os->writeLine(identStr + "}");
}

void MemberIf::setCond(Member *var, Member *varParam)
{
    m_var = var;
    m_varParam = varParam;
}

void MemberIf::addMember(Member *member)
{
    O3D_ASSERT(member->getParent() == this);
    m_members.push_back(member);
}

Member *MemberIf::findMember(const String &name) const
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

UInt32 MemberIf::getIdent() const
{
    return 1;
}

void MemberIf::writeSetterDecl(OutStream *os)
{
    // SET
    String line("    ");

    String name = m_var->getName();
    if (name.startsWith("m_"))
        name.remove(0, 2);
    if (name.length() >= 1)
        name[0] = WideChar::toUpper(name[0]);

    String pname = m_varParam->getName();
    pname.lower();
    pname[0] = WideChar::toUpper(pname[0]);
    Int32 p;
    while ((p = pname.find('_')) != -1)
    {
        pname.remove(p, 1);
        pname[p] = WideChar::toUpper(pname[p]);
    }

    name += pname;

    line += String("void set") + name + "(";

    // write in param
    String mname;
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

        line = String("        ") + member->getName() + " = " + mname + ";";
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

        os->writeLine("");
    }

    String cond = "        if (!";
    cond += m_var->getName();
    cond += m_var->getIfTest(m_varParam);
    cond += ")";

    os->writeLine(cond);
    os->writeLine("        {");
    if (size.isValid())
    {
        line = String("            m_messageDataSize += ") << size << ";";
        os->writeLine(line);
    }

    // set bit
    cond = "            ";
    cond += m_var->getName();
    cond += m_var->getSetTo(m_varParam, Member::SET_TRUE);
    cond += ";";
    os->writeLine(cond);
    os->writeLine("        }");

    os->writeLine("    }");
    os->writeLine("");

    // UNSET
    line = "    ";
    line += String("void unset") + name + "()";

    os->writeLine(line);
    os->writeLine("    {");

    cond = "        if (";
    cond += m_var->getName();
    cond += m_var->getIfTest(m_varParam);
    cond += ")";

    os->writeLine(cond);
    os->writeLine("        {");

    if (size.isValid())
    {
        line = String("            m_messageDataSize -= ") << size << ";";
        os->writeLine(line);
    }

    // unset bit
    cond = "            ";
    cond += m_var->getName();
    cond += m_var->getSetTo(m_varParam, Member::SET_FALSE);
    cond += ";";
    os->writeLine(cond);
    os->writeLine("        }");

    os->writeLine("    }");
    os->writeLine("");

    // children
    for (Member *child : m_members)
    {
        child->writeSetterDecl(os);
    }
}

void MemberIf::writeSetterImpl(OutStream *os)
{

}
