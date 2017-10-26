/**
 * @file registermember.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-11-19
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_REGISTERMEMBER_H
#define _O3D_DMG_REGISTERMEMBER_H

#include "member.h"
#include "memberfactory.h"

namespace o3d {
namespace dmg {

/**
 * @brief Helper to register a member.
 * Usage: in a cpp file do RegisterMember<MemberClassName>::R inst;
 * @date 2013-11-19
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 */
template <class T>
struct RegisterMember
{
    RegisterMember()
    {
        Member *member = T::createInstance(nullptr);
        MemberFactory::instance()->registerMember(member);
    }

    typedef RegisterMember R;
};

} // namespace o3d
} // namespace nmg

#endif // _O3D_DMG_REGISTERMEMBER_H
