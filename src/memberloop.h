/**
 * @file memberloop.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_MEMBERLOOP_H
#define _O3D_DMG_MEMBERLOOP_H

#include "member.h"

namespace o3d {
namespace dmg {

class MemberLoop : public MemberHelper<MemberLoop, Member::TYPE_LOOP>
{
public:

    MemberLoop(Member *parent);

    virtual String getTypeName() const;
    virtual String getOutTypeName() const;

    virtual T_StringList getHeaders() const;

    virtual void writeDecl(OutStream *os);

    virtual String getReadMethod() const;
    virtual String getWriteMethod() const;

    virtual void writeRead(OutStream *os);
    virtual void writeWrite(OutStream *os);

    virtual void setCond(Member *var, Member *varParam);
    virtual void addMember(Member *member);
    virtual Member* findMember(const String &name) const;

    //! Return the number of ident
    virtual UInt32 getIdent() const;

    //! Get the read/write children prefix
    virtual String getPrefix() const;

    virtual void writeSetterDecl(OutStream *os);
    virtual void writeSetterImpl(OutStream *os);

private:

    Member *m_var;
    Member *m_varParam;

    String m_arrayName;

    std::list<Member*> m_members;
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_MEMBERLOOP_H
