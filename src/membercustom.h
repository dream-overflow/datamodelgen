/**
 * @file membercustom.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_MEMBERCUSTOM_H
#define _O3D_DMG_MEMBERCUSTOM_H

#include "member.h"
#include <vector>

namespace o3d {
namespace dmg {

class MemberCustom : public Member
{
public:

    MemberCustom(const MemberCustom &dup, Member *parent);

    MemberCustom(Member *parent);

    virtual void setTypeName(const String &typeName);
    virtual String getTypeName() const;

    virtual void setOutTypeName(const String &typeName);
    virtual String getOutTypeName() const;

    virtual String getReadMethod() const;
    virtual String getWriteMethod() const;

    virtual void writeRead(OutStream *os);
    virtual void writeWrite(OutStream *os);

    virtual void setHeaders(const T_StringList &headers);
    virtual T_StringList getHeaders() const;

    virtual void writeSetterDecl(OutStream *os);
    virtual void writeSetterImpl(OutStream *os);

    virtual void writeGetterDecl(OutStream *os);
    virtual void writeGetterImpl(OutStream *os);

    virtual void writeFinalize(Context &ctx);

    virtual void setTemplatesArgs(const T_StringList &args);
    virtual void setTemplate(UInt32 index, const String &value, Bool resolved);
    virtual String getTemplateName(UInt32 index) const;
    virtual std::vector<UInt32> getUnresolvedTemplates() const;

    virtual Bool isRef() const;

    virtual UInt32 getMinSize() const;
    virtual String getSizeOf() const;

    virtual UInt32 getType() const { return (UInt32)Member::TYPE_CUSTOM; }

    // normaly useless
    static Member* createInstance(Member *parent) { return new MemberCustom(parent); }

    // we want a clone
    virtual Member* makeInstance(Member *parent) const { return new MemberCustom(*this, parent); }

protected:

    String m_typeName;
    T_StringList m_headers;
    String m_outTypeName;

    struct TemplateParam
    {
        String name;
        String value;
        Bool resolved;
    };

    typedef std::vector<TemplateParam> T_TemplateList;
    typedef T_TemplateList::iterator IT_TemplateList;
    typedef T_TemplateList::const_iterator CIT_TemplateList;

    T_TemplateList m_templates;
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_MEMBERCUSTOM_H
