/**
 * @file memberfactory.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberfactory.h"
#include <o3d/core/debug.h>

using namespace o3d;
using namespace o3d::dmg;

MemberFactory* MemberFactory::ms_instance = nullptr;

MemberFactory* MemberFactory::instance()
{
    if (ms_instance == nullptr)
        ms_instance = new MemberFactory;

    return ms_instance;
}

void MemberFactory::destroy()
{
    deletePtr(ms_instance);
}

MemberFactory::MemberFactory()
{
}

void MemberFactory::registerMember(Member *member)
{
    String typeName = member->getTypeName();
    if (m_members.find(typeName) != m_members.end())
    {
        deletePtr(m_members[typeName]);
    }

    m_members[typeName] = member;
}

MemberFactory::~MemberFactory()
{
    for (std::pair<String, Member*> entry : m_members)
    {
        deletePtr(entry.second);
    }
}

Member* MemberFactory::buildFromTypeName(const String &typeName, Member *parent)
{
    auto it = m_members.find(typeName);

    if (it != m_members.end())
        return it->second->makeInstance(parent);
    else
        O3D_ERROR(E_InvalidParameter("Unsuported type name " + typeName));
}
