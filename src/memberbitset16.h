/**
 * @file memberbitset16.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_MEMBERBITSET16_H
#define _O3D_DMG_MEMBERBITSET16_H

#include "member.h"
#include <o3d/core/idmanager.h>

namespace o3d {
namespace dmg {

class MemberBitSet16 : public MemberHelper<MemberBitSet16, Member::TYPE_BITSET16>
{
public:

    MemberBitSet16(Member *parent);

    virtual String getTypeName() const;
    virtual String getOutTypeName() const;

    virtual String getReadMethod() const;
    virtual String getWriteMethod() const;

    virtual void writeRead(OutStream *os);

    virtual T_StringList getHeaders() const;

    virtual String getIfTest(const Member *param) const;
    virtual UInt32 getNewUIntId();

    virtual void writeSetterDecl(OutStream *os);
    virtual void writeSetterImpl(OutStream *os);

    virtual String getSetTo(const Member *param, SetValue value) const;

    virtual UInt32 getMinSize() const;
    virtual String getSizeOf() const;

private:

    IDManager m_uintId;
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_MEMBERBITSET16_H
