/**
 * @file context.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_CONTEXT_H
#define _O3D_DMG_CONTEXT_H

#include <o3d/core/outstream.h>

namespace o3d {
namespace dmg {

class Data;

/**
 * @brief
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 */
class Context
{
public:

    OutStream *os;

    Data *data;

    UInt32 profile;
    UInt32 target;
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_CONTEXT_H
