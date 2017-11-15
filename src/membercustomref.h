/**
 * @file membercustomref.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_MEMBERCUSTOMREF_H
#define _O3D_DMG_MEMBERCUSTOMREF_H

#include "membercustom.h"
#include "context.h"

namespace o3d {
namespace dmg {

class Data;

class MemberCustomRef : public MemberCustom
{
public:

    MemberCustomRef(const MemberCustomRef &dup, Member *parent);

    MemberCustomRef(Member *parent);

    virtual ~MemberCustomRef();

    virtual void writeDecl(OutStream *os);

    virtual void writeRead(OutStream *os);
    virtual void writeWrite(OutStream *os);

    virtual void writeSetterDecl(OutStream *os);
    virtual void writeSetterImpl(OutStream *os);

    virtual void writeGetterDecl(OutStream *os);
    virtual void writeGetterImpl(OutStream *os);

    virtual String getOutTypeName() const;

    virtual void writeFinalize(Context &ctx);

    void setHeaders(const T_StringList &headers);

    void setRefData(Data *data);

    /**
     * @brief setRefMember A member reference, ie int32, string, to resolv this member,
     *                     and read/write.
     * @param ref delete by MemberCustomRef destructor
     */
    virtual void setRefMember(Member *ref);
    virtual Bool isRef() const;

    virtual UInt32 getMinSize() const;
    virtual String getSizeOf() const;

    virtual UInt32 getType() const { return (UInt32)Member::TYPE_CUSTOM_REF; }

    // normaly useless
    static Member* createInstance(Member *parent) { return new MemberCustomRef(parent); }

    // we want a clone
    virtual Member* makeInstance(Member *parent) const { return new MemberCustomRef(*this, parent); }

protected:

    Member *m_ref;
    Data *m_refData;
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_MEMBERCUSTOMREF_H
