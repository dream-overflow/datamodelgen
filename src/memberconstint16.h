/**
 * @file memberconstint16.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_MEMBERCONSINT16_H
#define _O3D_DMG_MEMBERCONSINT16_H

#include "member.h"

namespace o3d {
namespace dmg {

class MemberConstInt16 : public MemberHelper<MemberConstInt16, Member::TYPE_CONST_UINT32>
{
public:

    MemberConstInt16(Member *parent);

    virtual String getTypeName() const;
    virtual String getOutTypeName() const;

    virtual String getReadMethod() const;
    virtual String getWriteMethod() const;

    virtual void writeDecl(OutStream *os);

    virtual void writeRead(OutStream *os);
    virtual void writeWrite(OutStream *os);

    virtual void writeSetterDecl(OutStream *os);
    virtual void writeSetterImpl(OutStream *os);

    virtual void writeGetterDecl(OutStream *os);
    virtual void writeGetterImpl(OutStream *os);
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_MEMBERCONSINT16_H
