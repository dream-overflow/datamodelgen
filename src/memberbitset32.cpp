/**
 * @file memberbitset32.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberbitset32.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberBitSet32>::R memberBitSet32;

MemberBitSet32::MemberBitSet32(Member *parent) :
    MemberHelper(parent)
{
}

String MemberBitSet32::getTypeName() const
{
    return "bitset32";
}

String MemberBitSet32::getOutTypeName() const
{
    return "o3d::BitSet32";
}

String MemberBitSet32::getReadMethod() const
{
    return "readUInt32";
}

String MemberBitSet32::getWriteMethod() const
{
    return "writeUInt32";
}

void MemberBitSet32::writeRead(OutStream *os)
{
    Int32 ident = 1;
    Member *parent = getParent();
    while (parent != nullptr)
    {
        ident += parent->getIdent();
        parent = parent->getParent();
    }

    String line;
    for (Int32 i = 0; i < ident; ++i)
    {
        line += "    ";
    }

    String prefixedName = (isParent() ? getParent()->getPrefix() : "") + getName();

    line += prefixedName + ".set(buffer->" + getReadMethod() + "());";

    os->writeLine(line);
}

T_StringList MemberBitSet32::getHeaders() const
{
    T_StringList list;
    list.push_back("<o3d/core/TemplateBitSet.h>");

    return list;
}

String MemberBitSet32::getIfTest(const Member *param) const
{
    O3D_ASSERT(param != nullptr);
    return ".getBit(" + param->getName() + ")";
}

UInt32 MemberBitSet32::getNewUIntId()
{
    return m_uintId.getID();
}

void MemberBitSet32::writeSetterDecl(OutStream *os)
{

}

void MemberBitSet32::writeSetterImpl(OutStream *os)
{

}

String MemberBitSet32::getSetTo(const Member *param, SetValue value) const
{
    O3D_ASSERT(param != nullptr);
    if (value == SET_TRUE)
        return ".enable(" + param->getName() + ")";
    else
        return ".disable(" + param->getName() + ")";
}

UInt32 MemberBitSet32::getMinSize() const
{
    return 4;
}

String MemberBitSet32::getSizeOf() const
{
    return "4";
}
