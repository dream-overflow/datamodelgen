/**
 * @file member.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_MEMBER_H
#define _O3D_DMG_MEMBER_H

#include <o3d/core/string.h>
#include <o3d/core/stringlist.h>
#include <o3d/core/stringmap.h>
#include "context.h"

#include <vector>

namespace o3d {
namespace dmg {

class Member
{
public:

    enum Type
    {
        TYPE_BOOL,
        TYPE_INT8,
        TYPE_UINT8,
        TYPE_INT16,
        TYPE_UINT16,
        TYPE_INT32,
        TYPE_UINT32,
        TYPE_INT64,
        TYPE_UINT64,
        TYPE_FLOAT,
        TYPE_DOUBLE,
        TYPE_STRING,
        TYPE_BITSET8,
        TYPE_BITSET16,
        TYPE_BITSET32,
        TYPE_BITSET64,
        TYPE_ARRAY8,
        TYPE_ARRAYU8,
        TYPE_ARRAY16,
        TYPE_ARRAYU16,
        TYPE_ARRAY32,
        TYPE_ARRAYU32,
        TYPE_INT8_ARRAY,
        TYPE_UINT8_ARRAY,
        TYPE_INT16_ARRAY,
        TYPE_UINT16_ARRAY,
        TYPE_INT32_ARRAY,
        TYPE_UINT32_ARRAY,
        TYPE_STRUCT,
        TYPE_LOOP,
        TYPE_IF,
        TYPE_IFNOT,
        TYPE_IFEQ,
        TYPE_IFNEQ,
        TYPE_BIT,
        TYPE_CONST_INT8,
        TYPE_CONST_UINT8,
        TYPE_CONST_INT16,
        TYPE_CONST_UINT16,
        TYPE_CONST_INT32,
        TYPE_CONST_UINT32,
        TYPE_IMMEDIATE_UINT32,
        TYPE_CTOR,
        TYPE_CUSTOM,
        TYPE_CUSTOM_REF,
        TYPE_CUSTOM_ARRAY
    };

    enum SetValue
    {
        SET_FALSE,
        SET_TRUE
    };

    Member(Member *parent);

    virtual ~Member();

    /**
     * @brief getType Member class type (@see Type).
     * @return
     */
    virtual UInt32 getType() const = 0;

    /**
     * @brief getTypeName Type name of the member in local language.
     * @return
     */
    virtual String getTypeName() const = 0;

    /**
     * @brief getOutTypeName Type name for the output language.
     * @return
     */
    virtual String getOutTypeName() const = 0;

    /**
     * @brief getReadMethod Name of the method for reading the member.
     * @return
     */
    virtual String getReadMethod() const = 0;

    /**
     * @brief getWriteMethod Name of the method for writting the member.
     * @return
     */
    virtual String getWriteMethod() const = 0;

    //! Get the related includes files (default is an empty list).
    virtual T_StringList getHeaders() const;

    //! Make an instance of the member
    virtual Member* makeInstance(Member *parent) const = 0;

    /**
     * @brief setName Member name (related to its out member name).
     * @param name
     */
    void setName(const String &name) { m_name = name; }
    /**
     * @brief getName Member name (related to its out member name).
     * @return
     */
    const String& getName() const { return m_name; }

    //! Get the name with parent prefixes
    String getPrefixedName() const
    {
        return (isParent() ? getParent()->getPrefix() : "") + getName();
    }

    /**
     * @brief getParent A parent if a structure (loop, if...)
     * @return
     */
    Member* getParent() { return m_parent; }
    /**
     * @brief getParent
     * @return
     */
    const Member* getParent() const { return m_parent; }

    /**
     * @brief isParent Simple helper.
     * @return True if a parent.
     */
    Bool isParent() const { return m_parent != nullptr; }

    /**
     * @brief getIfTest if condition expression
     * @param param
     * @return
     */
    virtual String getIfTest(const Member *param) const;

    /**
     * @brief getNewUIntId Some member generated unique integers values.
     * @return The next available integer value (0 is this implementation).
     */
    virtual UInt32 getNewUIntId();

    /**
     * @brief addMember Some member have childen members (loop, if...).
     * @param member
     */
    virtual void addMember(Member *member);

    //! Find a direct child member, or if not found up to the parent recursively.
    virtual Member* findMember(const String &name) const;

    /**
     * @brief setCond Set the condition variable and the condition parameter (loop, if...).
     * @param var
     * @param varParam
     */
    virtual void setCond(Member *var, Member *varParam);

    /**
     * @brief setPublic Set the member as public, default is private.
     */
    virtual void setPublic();
    /**
     * @brief setPublic Set the member as private, this is the default.
     */
    virtual void setPrivate();

    /**
     * @brief isPrivate Check if the member is private.
     * @return
     */
    virtual Bool isPrivate() const;
    /**
     * @brief isPrivate Check if the member is public.
     * @return
     */
    virtual Bool isPublic() const;

    /**
     * @brief setValue Some members implements a value.
     * @param value
     */
    virtual void setValue(const String &value);

    /**
     * @brief getValue Some members implements a value.
     * @param value
     */
    virtual const String& getValue() const;

    //
    // Setters
    //

    virtual void writeSetterDecl(OutStream *os);
    virtual void writeSetterImpl(OutStream *os);

    //
    // Getters
    //

    virtual void writeGetterDecl(OutStream *os);
    virtual void writeGetterImpl(OutStream *os);

    /**
     * @brief writeDecl
     * @param os
     */
    virtual void writeDecl(OutStream *os);

    /**
     * @brief writeRead
     * @param os
     */
    virtual void writeRead(OutStream *os);
    /**
     * @brief writeWrite
     * @param os
     */
    virtual void writeWrite(OutStream *os);

    /**
     * @brief writeFinalize Finalize on some members
     * @param os
     */
    virtual void writeFinalize(Context &ctx);

    //! Return the number of ident
    virtual UInt32 getIdent() const;

    //! Get the read/write children prefix
    virtual String getPrefix() const;

    //! Get the size in byte
    virtual UInt32 getMinSize() const;

    //! Reference in param (reference and not copy)
    virtual Bool isRef() const;

    //! get a "Set to TRUE, FALSE or any other value".
    virtual String getSetTo(const Member *param, SetValue value) const;

    //! Get size of the member.
    virtual String getSizeOf() const;

    //
    // Template
    //

    /**
     * @brief setTemplatesArgs Define the templates arguments names
     * @param args
     */
    virtual void setTemplatesArgs(const T_StringList &args);

    /**
     * @brief setTemplate Define the templates value for an argument.
     * @param index Zero based index of the template argument.
     * @param value Value of the template argument, or name of a template variable.
     * @param resolved True if value is resolved, False mean it is a template variable name.
     */
    virtual void setTemplate(UInt32 index, const String &value, Bool resolved);

    /**
     * @brief getTemplateName Get the template argument name.
     * @param index Zero based index.
     * @return
     */
    virtual String getTemplateName(UInt32 index) const;

    /**
     * @brief getUnresolvedTemplates Get the vector of unresolved index of templates
     *        parameters.
     * @return
     */
    virtual std::vector<UInt32> getUnresolvedTemplates() const;

protected:

    Member *m_parent;

    String m_name;
    Bool m_public;

    String m_value;
};

/**
 * @brief Abstract member class helper
 * @date 2013-11-19
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 */
template <class T, UInt32 E>
class MemberHelper : public Member
{
public:

    MemberHelper(Member *parent) :
        Member(parent)
    {

    }

    virtual UInt32 getType() const { return (UInt32)E; }
    static Member* createInstance(Member *parent) { return new T(parent); }
    virtual Member* makeInstance(Member *parent) const { return new T(parent); }
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_MEMBER_H
