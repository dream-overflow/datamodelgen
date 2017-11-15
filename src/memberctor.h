/**
 * @file memberctor.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_MEMBERCTOR_H
#define _O3D_DMG_MEMBERCTOR_H

#include "member.h"

namespace o3d {
namespace dmg {

class MemberCtor : public MemberHelper<MemberCtor, Member::TYPE_IF>
{
public:

    MemberCtor(Member *parent);
    virtual ~MemberCtor();

    virtual String getTypeName() const;
    virtual String getOutTypeName() const;

    virtual void writeDecl(OutStream *os);

    virtual String getReadMethod() const;
    virtual String getWriteMethod() const;

    virtual void writeRead(OutStream *os);
    virtual void writeWrite(OutStream *os);

    virtual void writeSetterDecl(OutStream *os);
    virtual void writeSetterImpl(OutStream *os);

    virtual void writeGetterDecl(OutStream *os);
    virtual void writeGetterImpl(OutStream *os);
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_MEMBERCTOR_H
