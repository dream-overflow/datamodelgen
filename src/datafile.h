/**
 * @file datafile.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_DATAFILE_H
#define _O3D_DMG_DATAFILE_H

#include <o3d/core/stringlist.h>
#include <o3d/core/stringmap.h>
#include "member.h"

#include <vector>

namespace o3d {
namespace dmg {

struct Data;

typedef std::list<Member*> T_MemberList;
typedef T_MemberList::iterator IT_MemberList;
typedef T_MemberList::const_iterator CIT_MemberList;

typedef std::list<Data*> T_DataList;
typedef T_DataList::iterator IT_DataList;
typedef T_DataList::const_iterator CIT_DataList;

struct TemplateParam
{
    String name;
    String value;
    Bool resolved;
};

typedef std::vector<TemplateParam> T_TemplateParamVector;
typedef T_TemplateParamVector::iterator IT_TemplateParamVector;
typedef T_TemplateParamVector::const_iterator CIT_TemplateParamVector;

struct IdentifierMetaData
{
    IdentifierMetaData()
    {
    }

    struct Entry
    {
        Entry() :
            pointer(False)
        {

        }

        String manager;
        String method;
        Bool pointer;

        T_StringList params;
        T_StringList headers;

        Member *member;
    };

    Entry defaultEntry;
    StringMap<Entry> templates;
};

struct Data
{
    Data() :
        passed(-1),
        abstract(False),
        isTemplate(False),
        importLevel(0),
        directInherit(nullptr),
        id(-1),
        minSize(0),
        identifier(nullptr)
    {
    }

    ~Data();

    //! Search for a member by name
    Member* getMember(const String &name);

    //! identifier settings, per target (can be overrided)
    IdentifierMetaData identifierMeta[4];

    //! Last/current pass (-1 mean no pass done)
    Int32 passed;

    //! True mean abstract (not exported class)
    Bool abstract;
    //! True if template class
    Bool isTemplate;
    //! When importing a class, at which level we import this class (0 mean root)
    UInt32 importLevel;

    //! Single inheritance, on nullptr
    Data *directInherit;

    //! Unique class name
    String name;
    //! Unique class id
    UInt32 id;
    //! Minimal size (base size) of the class
    UInt32 minSize;

    //! Templates arguments (template line before data line)
    T_StringList templatesArgs;
    //! Templates parameters name<=>value(resolved) (template value are at inheritance)
    T_TemplateParamVector templatesParams;

    //! Member by target (common=0, ...)
    T_MemberList members[4];

    //! unique identifier and settings (can be inherited)
    Member *identifier;

    //! reference to satisfy
    T_MemberList finalizers;

    //! member that have a default value at constructor
    T_MemberList initializers;

    //! extern members are declared from an inherited class, and used with initializers
    T_MemberList externs;

    //! statics members are only declared, but not implemented (no read/write...)
    T_MemberList statics;
};

/**
 * @brief Parser and code generator
 * Improvements:
 *  - have separates class for parsing and generating, with a model.
 *  - for the generator, be more generic to avoid multiplicity of writters
 *  - for the parser, use a parametrable FSM
 *  - for the parser, optimise the second pass, limit the first as necessary
 *  - be compatible for NMG and DMG, to avoid multiplicity of parsers, members...
 *  - extern and static keywords
 *  - common export for NMG and DMG => @client/@server in/out/both... as a @profile or...
 *  - common message/data keyword (class ?) or permit aliases
 */
class DataFile
{
public:

    enum FileType
    {
        F_HPP = 0,
        F_CPP = 1
    };

    enum TargetType
    {
        T_COMMON = 0,
        T_DISPLAYER = 1,
        T_AUTHORITY = 2,
        T_EDITOR = 3
    };

    enum Profile
    {
        DISPLAYER = 0,
        AUTHORITY = 1,
        EDITOR = 2
    };

    DataFile(
            const String &path,
            const String &filename,
            const String &suffix,
            Bool composite);

    ~DataFile();

    const String& getName() const;

    void parseClassFile();
    void parseTypedefFile();

    void process();

    void addMember(TargetType type, const String &name, Member *member);

    void write(const String &outPath, const String &hppExt, const String &cppExt);

private:

    //! True mean class composition, False mean inheritance excepted for abstract classes.
    Bool m_composite;

    //! List of currently imported files during parsing, to read only once a file
    T_StringList m_importedDmg;

    //! Path name of the main parsed file
    String m_pathname;
    //! File name of the main parsed file
    String m_filename;
    //! Class name only (withouth path, neither trailing /, neither file extension)
    String m_prefix;
    //! Class suffix for generation (influence on exported classes name and generated files names)
    String m_suffix;
    //! Relative path to the root; with zero, one or more "../"
    String m_relPath;

    //! Current input stream of the main parsed file
    InStream *m_is;

    //! Current target type
    TargetType m_currentType;
    //! Content of the current line (can be truncated and optimized)
    String m_currentLine;
    //! List of template arguments for the current class (filled when template keyword is found)
    T_StringList m_templatesArgs;
    //! The current imported file name is converted to a header file with a suffix in this var
    String m_currentImport;
    //! Current import level (0 mean base file)
    UInt32 m_currentImportLevel;
    //! Current is a template specialization
    Bool m_templateSpe;
    //! For the current object, the related values to the templates arguments
    std::vector<String> m_templatesValue;

    //! Imported headers, with relative path to this data path
    T_StringList m_imports;

    //!< headers[targetType][header|cpp];
    T_StringList m_includes[4][2];

    //! Predeclaration of classes, for custom members with references (class MyClass;...)
    T_StringList m_preClass;

    //! Add a member to a data, checking, add to main or to parent...
    void addMember(TargetType target, Data *data, Member *member, Member *parent);

    //! Parse a file containing class declarations.
    void parseClassFile(InStream *is, UInt32 importLevel, Int32 pass);
    //! Parse a file containing typedef declarations.
    void parseTypedefFile(InStream *is);

    //! Import a file containing class declarations.
    void importData(const String &line, UInt32 importLevel, Int32 pass);
    //! Import a file containing typedef declarations.
    void importTypedef(const String &line, Int32 pass);

    void parseTarget(InStream *is, const String &line, Data *data);
    void parseTypeDef(InStream *is, const String &line, Int32 pass);
    void parseIdentifier(InStream *is, const String &line, Data *data);
    void parseData(InStream *is, const String &line, Int32 pass);
    void parseTemplate(InStream *is, const String &line);

    void parseDataInt(InStream *is, Bool begin, Data *data);
    void parseDataLoop(InStream *is, const String &line, Data *data, Member *parent);
    void parseDataIf(InStream *is, const String &line, Data *data, Member *parent);
    void parseDataMember(InStream *is, const String &line, Data *data, Member *parent);
    void parseDataArray(InStream *is, const String &line, Data *data, Member *parent);
    void parseDataConst(InStream *is, const String &line, Data *data, Member *parent, Bool ispublic);
    void parseDataBit(InStream *is, const String &line, Data *data, Member *parent);
    void parseDataExtern(InStream *is, const String &line, Data *data, Member *parent);
    void parseDataStatic(InStream *is, const String &line, Data *data, Member *parent);

    //! Parse @annotations
    void parseAnnotation(InStream *is, const String &line, Data *data);

    void writeDataReaderClass(const String &outPath, const String &hppExt, Profile profile);
    void writeDataReaderImpl(const String &outPath, const String &cppExt, Profile profile);

    void writeDataReaderClassContent(OutStream *os, Data *data, Profile profile);
    void writeDataReaderImplContent(OutStream *os, Data *data, Profile profile);

    void writeDataReaderUserImpl(const String &outPath, const String &cppExt, Profile profile);
    void writeDataReaderUserImplContent(OutStream *os, Data *data, Profile profile);

    void writeDataWriterClass(const String &outPath, const String &hppExt, Profile profile);
    void writeDataWriterImpl(const String &outPath, const String &cppExt, Profile profile);

    void writeDataWriterClassContent(OutStream *os, Data *data, Profile profile);
    void writeDataWriterImplContent(OutStream *os, Data *data, Profile profile);

    //! Replace variables with their related content "${VarName}"
    void parseVariable(String &outLine,
            Data *data,
            const String &header,
            const String &classSuffix,
            Profile profile);

    //! Update headers as necessary (no doubled), for a specific target, and a target file type.
    void updateHeader(const T_StringList &headers, FileType fileType);

    //! Update class predeclaration (no doubled).
    void updateClasses(const String &classname);

    //! Contains any imported classes
    StringMap<Data*> m_data;
    std::list<Data*> m_ref;
};

} // namespace o3d
} // namespace dmg

#endif // _O3D_DMG_DATAFILE_H
