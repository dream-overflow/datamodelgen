/**
 * @file memberbit.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberbit.h"
#include "registermember.h"
#include <o3d/core/char.h>
#include <o3d/core/integer.h>

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberBit>::R memberBit;

MemberBit::MemberBit(Member *parent) :
    MemberHelper(parent),
    m_var(nullptr)
{
}

MemberBit::~MemberBit()
{

}

String MemberBit::getTypeName() const
{
    return "bit";
}

String MemberBit::getOutTypeName() const
{
    return "o3d::UInt32";
}

String MemberBit::getReadMethod() const
{
    return "";
}

String MemberBit::getWriteMethod() const
{
    return "";
}

void MemberBit::writeDecl(OutStream *os)
{
    String line = "    ";
    line += String("static const ") + getOutTypeName() + " " + getName() + " = " + m_value + ";";

    os->writeLine(line);
}

void MemberBit::writeRead(OutStream *os)
{
    // nothing
}

void MemberBit::writeWrite(OutStream *os)
{
    // nothing
}

void MemberBit::setCond(Member *var, Member *varParam)
{
    m_var = var;
    m_varParam = varParam;
}

void MemberBit::writeSetterDecl(OutStream *os)
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

    line += String("void set") + name + "()";

    os->writeLine(line);
    os->writeLine("    {");

    String cond = "        if (!";
    cond += m_var->getName();
    cond += m_var->getIfTest(m_varParam);
    cond += ")";

    os->writeLine(cond);
    os->writeLine("        {");

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

    // unset bit
    cond = "            ";
    cond += m_var->getName();
    cond += m_var->getSetTo(m_varParam, Member::SET_FALSE);
    cond += ";";
    os->writeLine(cond);
    os->writeLine("        }");

    os->writeLine("    }");
    os->writeLine("");
}
