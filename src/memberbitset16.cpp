/**
 * @file memberbitset16.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberbitset16.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberBitSet16>::R memberBitSet16;

MemberBitSet16::MemberBitSet16(Member *parent) :
    MemberHelper(parent)
{
}

String MemberBitSet16::getTypeName() const
{
    return "bitset16";
}

String MemberBitSet16::getOutTypeName() const
{
    return "o3d::BitSet16";
}

String MemberBitSet16::getReadMethod() const
{
    return "readUInt16";
}

String MemberBitSet16::getWriteMethod() const
{
    return "writeUInt16";
}

void MemberBitSet16::writeRead(OutStream *os)
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

T_StringList MemberBitSet16::getHeaders() const
{
    T_StringList list;
    list.push_back("<o3d/core/TemplateBitSet.h>");

    return list;
}

String MemberBitSet16::getIfTest(const Member *param) const
{
    O3D_ASSERT(param != nullptr);
    return ".getBit(" + param->getName() + ")";
}

UInt32 MemberBitSet16::getNewUIntId()
{
    return m_uintId.getID();
}

void MemberBitSet16::writeSetterDecl(OutStream *os)
{

}

void MemberBitSet16::writeSetterImpl(OutStream *os)
{

}

String MemberBitSet16::getSetTo(const Member *param, SetValue value) const
{
    O3D_ASSERT(param != nullptr);
    if (value == SET_TRUE)
        return ".enable(" + param->getName() + ")";
    else
        return ".disable(" + param->getName() + ")";
}

UInt32 MemberBitSet16::getMinSize() const
{
    return 2;
}

String MemberBitSet16::getSizeOf() const
{
    return "2";
}
