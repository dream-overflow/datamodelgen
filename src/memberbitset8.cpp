/**
 * @file memberbitset8.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "memberbitset8.h"
#include "registermember.h"

using namespace o3d;
using namespace o3d::dmg;

RegisterMember<MemberBitSet8>::R memberBitSet8;

MemberBitSet8::MemberBitSet8(Member *parent) :
    MemberHelper(parent),
    m_uintId(0)
{
}

String MemberBitSet8::getTypeName() const
{
    return "bitset8";
}

String MemberBitSet8::getOutTypeName() const
{
    return "o3d::BitSet8";
}

String MemberBitSet8::getReadMethod() const
{
    return "readUInt8";
}

String MemberBitSet8::getWriteMethod() const
{
    return "writeUInt8";
}

void MemberBitSet8::writeRead(OutStream *os)
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

T_StringList MemberBitSet8::getHeaders() const
{
    T_StringList list;
    list.push_back("<o3d/core/TemplateBitSet.h>");

    return list;
}

String MemberBitSet8::getIfTest(const Member *param) const
{
    O3D_ASSERT(param != nullptr);
    return ".getBit(" + param->getName() + ")";
}

UInt32 MemberBitSet8::getNewUIntId()
{
    return m_uintId.getID();
}

void MemberBitSet8::writeSetterDecl(OutStream *os)
{

}

void MemberBitSet8::writeSetterImpl(OutStream *os)
{

}

String MemberBitSet8::getSetTo(const Member *param, SetValue value) const
{
    O3D_ASSERT(param != nullptr);
    if (value == SET_TRUE)
        return ".enable(" + param->getName() + ")";
    else
        return ".disable(" + param->getName() + ")";
}

UInt32 MemberBitSet8::getMinSize() const
{
    return 1;
}

String MemberBitSet8::getSizeOf() const
{
    return "1";
}
