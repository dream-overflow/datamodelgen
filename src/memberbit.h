/**
 * @file memberbit.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_MEMBERBIT_H
#define _O3D_DMG_MEMBERBIT_H

#include "member.h"

namespace o3d {
namespace dmg {

class MemberBit : public MemberHelper<MemberBit, Member::TYPE_BIT>
{
public:

    MemberBit(Member *parent);
    virtual ~MemberBit();

    virtual String getTypeName() const;
    virtual String getOutTypeName() const;

    virtual void writeDecl(OutStream *os);

    virtual String getReadMethod() const;
    virtual String getWriteMethod() const;

    virtual void writeRead(OutStream *os);
    virtual void writeWrite(OutStream *os);

    virtual void setCond(Member *var, Member *varParam);

    virtual void writeSetterDecl(OutStream *os);

private:

    Member *m_var;
    Member *m_varParam;
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_MEMBERBIT_H
