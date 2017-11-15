/**
 * @file memberuint16.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_MEMBERUINT16_H
#define _O3D_DMG_MEMBERUINT16_H

#include "member.h"

namespace o3d {
namespace dmg {

class MemberUInt16 : public MemberHelper<MemberUInt16, Member::TYPE_UINT16>
{
public:

    MemberUInt16(Member *parent);

    virtual String getTypeName() const;
    virtual String getOutTypeName() const;

    virtual String getReadMethod() const;
    virtual String getWriteMethod() const;

    virtual UInt32 getMinSize() const;
    virtual String getSizeOf() const;

private:
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_MEMBERUINT16_H
