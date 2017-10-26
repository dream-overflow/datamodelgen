/**
 * @file membercustom.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "membercustom.h"
#include <o3d/core/debug.h>
#include <o3d/core/char.h>

using namespace o3d;
using namespace o3d::dmg;

// manual registration

MemberCustom::MemberCustom(const MemberCustom &dup, Member *parent) :
    Member(parent),
    m_typeName(dup.m_typeName),
    m_headers(dup.m_headers),
    m_outTypeName(dup.m_outTypeName),
    m_templates(dup.m_templates)
{

}

MemberCustom::MemberCustom(Member *parent) :
    Member(parent),
    m_typeName(""),
    m_headers(),
    m_outTypeName("")
{
}

void MemberCustom::setTypeName(const String &typeName)
{
    m_typeName = typeName;
}

String MemberCustom::getTypeName() const
{
    return m_typeName;
}

void MemberCustom::setOutTypeName(const String &typeName)
{
    m_outTypeName = typeName;
}

String MemberCustom::getOutTypeName() const
{
    return m_outTypeName;
}

String MemberCustom::getReadMethod() const
{
    return "readFromFile";
}

String MemberCustom::getWriteMethod() const
{
    return "writeToFile";
}

T_StringList MemberCustom::getHeaders() const
{
    return m_headers;
}

void MemberCustom::writeSetterDecl(OutStream *os)
{
    String line("    ");

    String name = getName();
    if (name.startsWith("_"))
        name.remove(0, 1);
    if (name.startsWith("m_"))
        name.remove(0, 2);
    String mname = name;
    name[0] = WideChar::toUpper(name[0]);

    line += String("void set") + name + "(const " + getOutTypeName() + " &" + mname + ")";
    os->writeLine(line);

    os->writeLine("    {");
    line = "        " + getName() + " = " + mname + ";";
    os->writeLine(line);
    os->writeLine("    }");
    os->writeLine("");
}

void MemberCustom::writeRead(OutStream *os)
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

    line += getPrefixedName() + "." + getReadMethod() + "(is);";
    os->writeLine(line);
}

void MemberCustom::writeWrite(OutStream *os)
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

    line += getPrefixedName() + "." + getWriteMethod() + "(os);";
    os->writeLine(line);
}

void MemberCustom::setHeaders(const T_StringList &headers)
{
    m_headers = headers;
}

void MemberCustom::writeSetterImpl(OutStream *os)
{

}

void MemberCustom::writeGetterDecl(OutStream *os)
{
    String line("    ");

    String name = getName();
    if (name.startsWith("m_"))
        name.remove(0, 2);
    name[0] = WideChar::toUpper(name[0]);

    line += "const " + getOutTypeName() + "& get" + name + "() const";
    os->writeLine(line);

    os->writeLine("    {");
    line = "        return " + getName() + ";";
    os->writeLine(line);
    os->writeLine("    }");
    os->writeLine("");
}

void MemberCustom::writeGetterImpl(OutStream *os)
{

}

void MemberCustom::writeFinalize(Context &ctx)
{
    ctx.os->writeLine("    " + getPrefixedName() + ".finalize();");
}

void MemberCustom::setTemplatesArgs(const T_StringList &args)
{
    for (const String &arg : args)
    {
        m_templates.push_back(TemplateParam());
        TemplateParam &tpl = m_templates.back();

        tpl.name = arg;
        tpl.resolved = False;
    }
}

void MemberCustom::setTemplate(UInt32 index, const String &value, Bool resolved)
{
    if (index >= m_templates.size())
        O3D_ERROR(E_IndexOutOfRange("Invalid template parameter index"));

    m_templates[index].value = value;
    m_templates[index].resolved = resolved;
}

String MemberCustom::getTemplateName(UInt32 index) const
{
    if (index >= m_templates.size())
        O3D_ERROR(E_IndexOutOfRange("Invalid template parameter index"));

    return m_templates[index].name;
}

std::vector<UInt32> MemberCustom::getUnresolvedTemplates() const
{
    std::vector<UInt32> result;
    UInt32 i = 0;

    for (const TemplateParam &param : m_templates)
    {
        if (!param.resolved)
            result.push_back(i);

        ++i;
    }

    return result;
}

Bool MemberCustom::isRef() const
{
    return False;
}

UInt32 MemberCustom::getMinSize() const
{
    return 2;
}

String MemberCustom::getSizeOf() const
{
    return getName() + ".length() + 2";
}
