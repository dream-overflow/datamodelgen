/**
 * @file main.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_MAIN_H
#define _O3D_DMG_MAIN_H

#include <o3d/core/evt.h>
#include <o3d/core/baseobject.h>

#include <o3d/core/memorymanager.h>
#include <o3d/core/stringlist.h>
#include <o3d/core/stringmap.h>

#include <o3d/core/idmanager.h>

#include "datafile.h"

namespace o3d {
namespace dmg {

/**
 * @brief The Main class
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-11
 */
class Main : public BaseObject
{
public:

    //! Get singleton.
    static Main* instance();

    //! Default constructor.
    Main();

    //! Destructor.
    virtual ~Main();

    void init();

    void run();

    Int32 command();

    void renameData(
            const String &filenameFrom,
            const String &filenameTo,
            const String &from,
            const String &to);

    void renameDataHeader(
            const String &filenameFrom,
            const String &filenameTo,
            const String &from,
            const String &to);

    void renameDataImpl(const String &filenameFrom,
            const String &filenameTo,
            const String &from,
            const String &to);

    //-----------------------------------------------------------------------------------
    // Accessors
    //-----------------------------------------------------------------------------------

    enum TemplateType
    {
        TPL_LICENCE = 0,
        TPL_HPP,
        TPL_CPP,
        TPL_DATA_READER_CLASS,
        TPL_DATA_WRITER_CLASS,
        TPL_DATA_READER_USER_IMPL,
        TPL_DATA_READER_IMPL,
        TPL_DATA_WRITER_IMPL,
        TPL_LAST = TPL_DATA_WRITER_IMPL
    };

    static const UInt32 NUM_TEMPLATE_TYPE = TPL_LAST + 1;

    const String& getInPath() const { return m_inPath; }

    const String& getOutHppPath(DataFile::Profile p) const { return m_outPath[0][p]; }
    const String& getOutCppPath(DataFile::Profile p) const { return m_outPath[1][p]; }
    const String& getIncludePath(DataFile::Profile p) const { return m_outPath[2][p]; }

    const String& getTypeDefExt() const { return m_typeDefExt; }
    const String& getClassExt() const { return m_classExt; }
    const String& getHppExt() const { return m_hppExt; }
    const String& getCppExt() const { return m_cppExt; }

    const T_StringList& getTemplate(TemplateType type) const { return m_templates[type]; }

    UInt32 getVersion() const { return m_version; }

    UInt32 getNextDataId();
    void registerDataId(UInt32 dataId);

    Bool isBuild(DataFile::Profile p) const { return m_build[p]; }

    const String& getNamespace(DataFile::Profile p) const { return m_namespace[p]; }
    const String& getAuthor() const { return m_author; }

    const String& getYear() const { return m_year; }
    const String& getMonth() const { return m_month; }
    const String& getDay() const { return m_day; }

private:

    static Main *ms_instance;

    Bool m_composite;

    String m_inPath;
    String m_tplPath;
    String m_outPath[3][3];

    String m_typeDefExt;
    String m_classExt;
    String m_hppExt;
    String m_cppExt;

    UInt32 m_version;

    Bool m_build[3];

    IDManager m_messageId;

    T_StringList m_templates[NUM_TEMPLATE_TYPE];
    std::list<DataFile*> m_parsed;

    String m_namespace[3];
    String m_author;

    String m_year;
    String m_month;
    String m_day;

    void readTemplate(const String &filename, T_StringList &lines);
    void readConfig(const String &filename);

    void browseSubFolder(const String &path);

public:

    static Int32 main();
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_MAIN_H
