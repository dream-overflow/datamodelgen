/**
 * @file membercustomarray.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_MEMBERCUSTOMARRAY_H
#define _O3D_DMG_MEMBERCUSTOMARRAY_H

#include "membercustom.h"

namespace o3d {
namespace dmg {

/**
 * @brief The MemberCustomArray class
 * Custom type and std::vector.
 */
class MemberCustomArray : public MemberCustom
{
public:

    MemberCustomArray(const MemberCustomArray &dup, Member *parent);

    MemberCustomArray(Member *parent);

    virtual String getOutTypeName() const;

    virtual String getReadMethod() const;
    virtual String getWriteMethod() const;

    virtual void writeRead(OutStream *os);
    virtual void writeWrite(OutStream *os);

    virtual void writeSetterDecl(OutStream *os);
    virtual void writeSetterImpl(OutStream *os);

    virtual void writeFinalize(Context &ctx);

    virtual UInt32 getMinSize() const;
    virtual String getSizeOf() const;

    void setHeaders(const T_StringList &headers);

    virtual UInt32 getType() const { return (UInt32)Member::TYPE_CUSTOM_ARRAY; }

    // normaly useless
    static Member* createInstance(Member *parent) { return new MemberCustomArray(parent); }

    // we want a clone
    virtual Member* makeInstance(Member *parent) const { return new MemberCustomArray(*this, parent); }
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_MEMBERCUSTOMARRAY_H
