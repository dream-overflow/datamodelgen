/**
 * @file memberstaticarrayuint32.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_MEMBERSTATICARRAYUINT32_H
#define _O3D_DMG_MEMBERSTATICARRAYUINT32_H

#include "member.h"
#include <o3d/core/idmanager.h>

namespace o3d {
namespace dmg {

class MemberStaticArrayUInt32 : public MemberHelper<MemberStaticArrayUInt32, Member::TYPE_UINT32_ARRAY>
{
public:

    MemberStaticArrayUInt32(Member *parent);

    virtual String getTypeName() const;
    virtual String getOutTypeName() const;

    virtual String getReadMethod() const;
    virtual String getWriteMethod() const;

    virtual T_StringList getHeaders() const;

    virtual String getIfTest(const Member *param) const;
    virtual UInt32 getNewUIntId();

    virtual void writeDecl(OutStream *os);

    virtual void writeSetterDecl(OutStream *os);
    virtual void writeSetterImpl(OutStream *os);

    virtual void writeGetterDecl(OutStream *os);
    virtual void writeGetterImpl(OutStream *os);

    virtual void writeRead(OutStream *os);
    virtual void writeWrite(OutStream *os);

    virtual Bool isRef() const;

    virtual UInt32 getMinSize() const;
    virtual String getSizeOf() const;

private:

    IDManager m_uintId;
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_MEMBERSTATICARRAYUINT32_H
