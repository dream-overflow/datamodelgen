/**
 * @file membercustomref.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "membercustomref.h"
#include "datafile.h"
#include <o3d/core/char.h>

using namespace o3d;
using namespace o3d::dmg;

// manual registration

MemberCustomRef::MemberCustomRef(const MemberCustomRef &dup, Member *parent) :
    MemberCustom(dup, parent),
    m_ref(nullptr),
    m_refData(dup.m_refData)
{

}

MemberCustomRef::MemberCustomRef(Member *parent) :
    MemberCustom(parent),
    m_ref(nullptr),
    m_refData(nullptr)
{
}

MemberCustomRef::~MemberCustomRef()
{
    deletePtr(m_ref);
}

void MemberCustomRef::setHeaders(const T_StringList &headers)
{
    m_headers = headers;
}

void MemberCustomRef::setRefData(Data *data)
{
    m_refData = data;
}

void MemberCustomRef::writeDecl(OutStream *os)
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

    String line = identStr + getOutTypeName() + " " + getName() + ";";
    os->writeLine(line);

    line = identStr + m_ref->getOutTypeName() + " " + m_ref->getName() + ";";
    os->writeLine(line);
}

void MemberCustomRef::writeSetterDecl(OutStream *os)
{
    String identStr("    ");

    String name = getName();
    if (name.startsWith("_"))
        name.remove(0, 1);
    if (name.startsWith("m_"))
        name.remove(0, 2);
    String mname = name + "_";
    name[0] = WideChar::toUpper(name[0]);

    // object reference
    String line = identStr + "void set" + name + "(const " + getOutTypeName() + " " + mname + ")";
    os->writeLine(line);

    os->writeLine(identStr + "{");
    line = identStr + identStr + getPrefixedName() + " = " + mname + ";";
    os->writeLine(line);
    os->writeLine(identStr + "}");
    os->writeLine("");

    // object id
    line = identStr + "void set" + name + "Id(" + m_ref->getOutTypeName() + " " + mname + ")";
    os->writeLine(line);

    os->writeLine(identStr + "{");
    line = identStr + identStr + m_ref->getPrefixedName() + " = " + mname + ";";
    os->writeLine(line);
    os->writeLine(identStr + "}");
    os->writeLine("");
}

void MemberCustomRef::writeRead(OutStream *os)
{
    m_ref->writeRead(os);
}

void MemberCustomRef::writeWrite(OutStream *os)
{
    m_ref->writeWrite(os);
}

void MemberCustomRef::writeSetterImpl(OutStream *os)
{
    // nothing
}

void MemberCustomRef::writeGetterDecl(OutStream *os)
{
    String identStr("    ");

    String name = getName();
    if (name.startsWith("_"))
        name.remove(0, 1);
    if (name.startsWith("m_"))
        name.remove(0, 2);
    name[0] = WideChar::toUpper(name[0]);

    // object reference
    String line = identStr + getOutTypeName() + " get" + name + "() const";
    os->writeLine(line);

    os->writeLine(identStr + "{");
    line = identStr + identStr + "return " + getPrefixedName() + ";";
    os->writeLine(line);
    os->writeLine(identStr + "}");
    os->writeLine("");

    // object id
    line = identStr + m_ref->getOutTypeName() + " get" + name + "Id() const";
    os->writeLine(line);

    os->writeLine(identStr + "{");
    line = identStr + identStr + "return " + m_ref->getPrefixedName() + ";";
    os->writeLine(line);
    os->writeLine(identStr + "}");
    os->writeLine("");
}

void MemberCustomRef::writeGetterImpl(OutStream *os)
{

}

String MemberCustomRef::getOutTypeName() const
{
    return "const " + m_outTypeName + "*";
}

void MemberCustomRef::writeFinalize(Context &ctx)
{
    String method;
    String methodType = ".";

    // default we take the unspecialized class
    const IdentifierMetaData::Entry *entry = &m_refData->identifierMeta[ctx.target].defaultEntry;
    // but if we have some templates arguments, we must look for
    if (!m_templates.empty())
    {
        // make a string with template value and comma as separator
        String tplStr = "";

        UInt32 n = 0;
        for (TemplateParam &tpl : m_templates)
        {
            tplStr += tpl.value;

            ++n;
            if (n < m_templates.size())
                tplStr += ",";
        }

        // search if we have this specialization...
        auto it = m_refData->identifierMeta[ctx.target].templates.find(tplStr);
        if (it != m_refData->identifierMeta[ctx.target].templates.end())
            entry = &it->second;
    }

    method = entry->method;
    if (entry->method.startsWith("*"))
    {
        methodType = "->";
        method.trimLeft();
    }
    else
    {
        methodType = ".";
    }

    String line = "    ";
    line += getPrefixedName() + " = ";
    line += entry->manager + methodType + method + "(";

    UInt32 n = 0;
    for (const String &p : entry->params)
    {
        if (p == "$id")
            line += m_ref->getPrefixedName();
        else
        {
            Bool tf = False;
            for (const TemplateParam& tpl : m_templates)
            {
                if (tpl.name == p)
                {
                    line += m_refData->name + "Data::" + tpl.value;
                    tf = True;
                    break;
                }
            }

            if (!tf)
            {
                // is a member of the referenced data
                if (!ctx.data->getMember(p))
                    line += m_refData->name + "Data::" + p;
                else
                    line += p;
            }
        }

        ++n;
        if (n < entry->params.size())
            line += ", ";
    }

    line += ");";

    ctx.os->writeLine(line);
}

void MemberCustomRef::setRefMember(Member *ref)
{
    m_ref = ref;
}

Bool MemberCustomRef::isRef() const
{
    return True;
}

UInt32 MemberCustomRef::getMinSize() const
{
    return m_ref->getMinSize();
}

String MemberCustomRef::getSizeOf() const
{
    return m_ref->getSizeOf();
}
