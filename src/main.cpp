/**
 * @file main.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <o3d/core/architecture.h>
#include <o3d/core/main.h>
#include <o3d/core/diskdir.h>
#include <o3d/core/filemanager.h>
#include <o3d/core/stringtokenizer.h>
#include <o3d/core/smartpointer.h>

#include "main.h"
#include "datafile.h"
#include "memberfactory.h"

using namespace o3d;
using namespace o3d::dmg;

Main *Main::ms_instance;

Main *Main::instance()
{
    return ms_instance;
}

Main::Main() :
    m_composite(False),
    m_typeDefExt("dtg"),
    m_classExt("dmg"),
    m_hppExt("h"),
    m_cppExt("cpp"),
    m_version(1),
    m_messageId(0)
{
    ms_instance = this;
}

Main::~Main()
{
    ms_instance = nullptr;
}

void Main::init()
{
    CommandLine *cmd = Application::getCommandLine();

    String configFilename = cmd->getArgs().back() + "/datamodelgen";
    if (configFilename.isEmpty())
        O3D_ERROR(E_InvalidParameter("Invalid config file"));

    readConfig(configFilename);

    Date date(True);
    m_year = date.buildString("%y");
    m_month = date.buildString("%M");
    m_day = date.buildString("%D");

    DiskDir inPath(m_inPath);
    if (!inPath.isExist())
        O3D_ERROR(E_InvalidParameter("Invalid input path"));

    for (Int32 n = 0; n < 2; ++n)
    {
        DiskDir displayerOutPath(m_outPath[n][DataFile::DISPLAYER]);
        if (!displayerOutPath.isExist())
            O3D_ERROR(E_InvalidParameter("Invalid displayer output path"));

        DiskDir authorityOutPath(m_outPath[n][DataFile::AUTHORITY]);
        if (!authorityOutPath.isExist())
            O3D_ERROR(E_InvalidParameter("Invalid authority output path"));

        DiskDir editorOutPath(m_outPath[n][DataFile::EDITOR]);
        if (!editorOutPath.isExist())
            O3D_ERROR(E_InvalidParameter("Invalid editor output path"));
    }

    DiskDir tlpPath(m_tplPath);
    if (!tlpPath.isExist())
         O3D_ERROR(E_InvalidParameter("Invalid template path"));

    System::print(String::print("%i", m_version), "Generate version");

    readTemplate(m_tplPath + "/license.template", m_templates[TPL_LICENCE]);
    readTemplate(m_tplPath + "/hpp.template", m_templates[TPL_HPP]);
    readTemplate(m_tplPath + "/cpp.template", m_templates[TPL_CPP]);
    readTemplate(m_tplPath + "/data.reader.class.template", m_templates[TPL_DATA_READER_CLASS]);
    readTemplate(m_tplPath + "/data.writer.class.template", m_templates[TPL_DATA_WRITER_CLASS]);
    readTemplate(m_tplPath + "/data.reader.user.impl.template", m_templates[TPL_DATA_READER_USER_IMPL]);
    readTemplate(m_tplPath + "/data.reader.impl.template", m_templates[TPL_DATA_READER_IMPL]);
    readTemplate(m_tplPath + "/data.writer.impl.template", m_templates[TPL_DATA_WRITER_IMPL]);
}

void Main::run()
{
    // Any found messages
    FileListing files;
    files.setPath(m_inPath);
    files.setExt("*." + m_classExt + "|*." + m_typeDefExt);
    files.searchFirstFile();

    FLItem *fl;
    while ((fl = files.searchNextFile()) != nullptr)
    {
        if (fl->FileType == FILE_FILE)
        {
            // process the data file
            DataFile *data = new DataFile("", files.getFileFullName(), "Data", m_composite);
            m_parsed.push_back(data);

            if (fl->FileName.endsWith(".dmg"))
                data->parseClassFile();
            else if (fl->FileName.endsWith(".tdg"))
                data->parseTypedefFile();
        }
        else if (fl->FileType == FILE_DIR)
        {
            // we want only a relative directory
            if (!fl->FileName.startsWith("."))
                browseSubFolder(fl->FileName);
        }
    }

    // second step process
    for (DataFile *data : m_parsed)
    {
        data->process();
        deletePtr(data);
    }
}

void Main::browseSubFolder(const String &path)
{
    // Any found messages
    FileListing files;
    files.setPath(m_inPath + "/" + path);
    files.setExt("*." + m_classExt + "|*." + m_typeDefExt);
    files.searchFirstFile();

    FLItem *fl;
    while ((fl = files.searchNextFile()) != nullptr)
    {
        if (fl->FileType == FILE_FILE)
        {
            // process the data file
            DataFile *data = new DataFile(path, files.getFileFullName(), "Data", m_composite);
            m_parsed.push_back(data);

            if (fl->FileName.endsWith(".dmg"))
                data->parseClassFile();
            else if (fl->FileName.endsWith(".tdg"))
                data->parseTypedefFile();
        }
        else if (fl->FileType == FILE_DIR)
        {
            if (!fl->FileName.startsWith("."))
                browseSubFolder(path + "/" + fl->FileName);
        }
    }
}

Int32 Main::command()
{
    CommandLine *cmd = Application::getCommandLine();

    if (cmd->getArgs().size() >= 2)
    {
        String op = cmd->getArgs()[0];
        String data = cmd->getArgs()[1];

        // mv, rename a data from source and targets
        if (op == "mv" && data.isValid() && cmd->getArgs().size() >= 3)
        {
            String dataTo = cmd->getArgs()[2];

            DiskDir source(m_inPath);
            if (source.check(data + ".dmg") == Dir::SUCCESS)
            {
                renameData(
                            source.getFullPathName() + "/" + data + ".dmg",
                            source.getFullPathName() + "/" + dataTo + ".dmg",
                            data,
                            dataTo);
                source.removeFile(data + ".dmg");
            }

            // file type
            for (Int32 n = 0; n < 2; ++n)
            {
                // profiles
                for (Int32 i = 0; i < 3; ++i)
                {
                    DiskDir out(m_outPath[n][i]);
                    if (out.check(data + "Data." + m_hppExt) == Dir::SUCCESS)
                    {
                        renameDataHeader(
                                    out.getFullPathName() + "/" + data + "Data." + m_hppExt,
                                    out.getFullPathName() + "/" + dataTo + "Data." + m_hppExt,
                                    data,
                                    dataTo);
                        out.removeFile(data + "Data." + m_hppExt);
                    }
                    if (out.check(data + "Data." + m_cppExt) == Dir::SUCCESS)
                    {
                        renameDataImpl(
                                    out.getFullPathName() + "/" + data + "Data." + m_cppExt,
                                    out.getFullPathName() + "/" + dataTo + "Data." + m_cppExt,
                                    data,
                                    dataTo);
                        out.removeFile(data + "Data." + m_cppExt);
                    }
                }
            }

            return 0;
        }

        // rm, remove a data from source and targets
        if (op == "rm" && data.isValid())
        {
            DiskDir source(m_inPath);
            if (source.check(data + ".dmg") == Dir::SUCCESS)
                source.removeFile(data + ".dmg");

            // file type
            for (Int32 n = 0; n < 2; ++n)
            {
                // profiles
                for (Int32 i = 0; i < 3; ++i)
                {
                    DiskDir out(m_outPath[n][i]);
                    if (out.check(data + "Data." + m_hppExt) == Dir::SUCCESS)
                        out.removeFile(data + "Data." + m_hppExt);
                    if (out.check(data + "Data." + m_cppExt) == Dir::SUCCESS)
                        out.removeFile(data + "Data." + m_cppExt);
                    if (out.check(data + "Data.user." + m_cppExt) == Dir::SUCCESS)
                        out.removeFile(data + "Data.user." + m_cppExt);
                }
            }

            return 0;
        }

        System::print("Invalid command", "DataModelGen", System::MSG_ERROR);
        return -1;
    }

    return 1;
}

void Main::renameData(const String &filenameFrom, const String &filenameTo, const String &from, const String &to)
{
    AutoPtr<InStream> fileFrom(FileManager::instance()->openInStream(filenameFrom));
    AutoPtr<FileOutStream> fileTo(FileManager::instance()->openOutStream(filenameTo, FileOutStream::CREATE));

    String line, l;

    while (fileFrom->readLine(line) != -1)
    {
        l = line;
        l.trimLeftChars(" \t");
        if (l.startsWith("data"))
        {
            // TODO more sensitive test
            line.replace(from, to);
        }

        fileTo->writeLine(line);
    }
}

void Main::renameDataHeader(
        const String &filenameFrom,
        const String &filenameTo,
        const String &from,
        const String &to)
{
    AutoPtr<InStream> fileFrom(FileManager::instance()->openInStream(filenameFrom));
    AutoPtr<FileOutStream> fileTo(FileManager::instance()->openOutStream(filenameTo, FileOutStream::CREATE));

    String line;

    String FROM(from);
    FROM.upper();
    String TO(to);
    TO.upper();

    while (fileFrom->readLine(line) != -1)
    {
        // TODO more sensitive test
        line.replace(from, to);
        line.replace(FROM, TO);
        fileTo->writeLine(line);
    }
}

void Main::renameDataImpl(
        const String &filenameFrom,
        const String &filenameTo,
        const String &from,
        const String &to)
{
    AutoPtr<InStream> fileFrom(FileManager::instance()->openInStream(filenameFrom));
    AutoPtr<FileOutStream> fileTo(FileManager::instance()->openOutStream(filenameTo, FileOutStream::CREATE));

    String line;

    String FROM(from);
    FROM.upper();
    String TO(to);
    TO.upper();

    while (fileFrom->readLine(line) != -1)
    {
        // TODO more sensitive test
        line.replace(from, to);
        line.replace(FROM, TO);
        fileTo->writeLine(line);
    }
}

UInt32 Main::getNextDataId()
{
    return m_messageId.getID();
}

void Main::registerDataId(UInt32 dataId)
{
    m_messageId.forceID(dataId);
}

Int32 Main::main()
{
    Debug::instance()->setDefaultLog("datamodelgen.log");
    Debug::instance()->getDefaultLog().clearLog();
    Debug::instance()->getDefaultLog().writeHeaderLog();

    CommandLine *cmd = Application::getCommandLine();
    if (cmd->getArgs().size() == 0)
    {
        System::print("Missing path to datamodelgen file", "", System::MSG_ERROR);
        return -1;
    }

    Main *apps = new Main();
    apps->init();

    Int32 res = apps->command();
    if (res != 1)
        return res;

    // process messages files
    try {
        apps->run();
    } catch (E_BaseException &e) {
    }

    // Destroy any content
    deletePtr(apps);

    Debug::instance()->getDefaultLog().writeFooterLog();

    return 0;
}

void Main::readTemplate(const String &filename, T_StringList &lines)
{
    // read templates
    InStream *is = FileManager::instance()->openInStream(filename);

    String line;
    while (is->readLine(line) != EOF)
    {
        lines.push_back(line);
    }

    deletePtr(is);
}

void Main::readConfig(const String &filename)
{
    // read templates
    InStream *is = FileManager::instance()->openInStream(filename);

    String line;
    Int32 equalPos;
    String key, value;

    while (is->readLine(line) != -1)
    {
        line.trimLeftChars(" \t");
        line.trimRightChars(" \t");

        // comment or empty line
        if (line.startsWith("#") || line.isEmpty())
            continue;

        // find the first = and split at
        equalPos = line.find('=');
        if (equalPos > 0)
        {
            key = line.sub(0, equalPos);
            key.trimRight(' ');

            value = line.sub(equalPos+1, -1);
            value.trimLeft(' ');

            if (key == "author")
                m_author = value;
            else if (key == "displayer.namespace")
                m_namespace[DataFile::DISPLAYER] = value;
            else if (key == "authority.namespace")
                m_namespace[DataFile::AUTHORITY] = value;
            else if (key == "editor.namespace")
                m_namespace[DataFile::EDITOR] = value;
            else if (key == "input")
                m_inPath = FileManager::instance()->getFullFileName(value);

            // unified outputs folders
            else if (key == "displayer.output")
                m_outPath[0][DataFile::DISPLAYER] =
                        m_outPath[1][DataFile::DISPLAYER] = FileManager::instance()->getFullFileName(value);
            else if (key == "authority.output")
                m_outPath[0][DataFile::AUTHORITY] =
                        m_outPath[1][DataFile::AUTHORITY] = FileManager::instance()->getFullFileName(value);
            else if (key == "editor.output")
                m_outPath[0][DataFile::EDITOR] =
                        m_outPath[1][DataFile::EDITOR] = FileManager::instance()->getFullFileName(value);
            // distincts headers folders
            else if (key == "displayer.output.headers")
                m_outPath[0][DataFile::DISPLAYER] = FileManager::instance()->getFullFileName(value);
            else if (key == "authority.output.headers")
                m_outPath[0][DataFile::AUTHORITY] = FileManager::instance()->getFullFileName(value);
            else if (key == "editor.output.headers")
                m_outPath[0][DataFile::EDITOR] = FileManager::instance()->getFullFileName(value);
            // distincts sources folders
            else if (key == "displayer.output.sources")
                m_outPath[1][DataFile::DISPLAYER] = FileManager::instance()->getFullFileName(value);
            else if (key == "authority.output.sources")
                m_outPath[1][DataFile::AUTHORITY] = FileManager::instance()->getFullFileName(value);
            else if (key == "editor.output.sources")
                m_outPath[1][DataFile::EDITOR] = FileManager::instance()->getFullFileName(value);
            // include folders
            else if (key == "displayer.output.includes")
                m_outPath[2][DataFile::DISPLAYER] =value;
            else if (key == "authority.output.includes")
                m_outPath[2][DataFile::AUTHORITY] = value;
            else if (key == "editor.output.includes")
                m_outPath[2][DataFile::EDITOR] = value;

            else if (key == "export")
            {
                m_build[DataFile::DISPLAYER] = m_build[DataFile::AUTHORITY] = m_build[DataFile::EDITOR] = False;

                if (value.sub("displayer", 0) != -1)
                    m_build[DataFile::DISPLAYER] = True;
                else if (value.sub("authority", 0) != -1)
                    m_build[DataFile::AUTHORITY] = True;
                else if (value.sub("editor", 0) != -1)
                    m_build[DataFile::EDITOR] = True;
                if (value.sub("displayer", 0) != -1)
                    m_build[DataFile::DISPLAYER] = m_build[DataFile::AUTHORITY] = m_build[DataFile::EDITOR] = True;
            }
            else if (key == "templates")
                m_tplPath = FileManager::instance()->getFullFileName(value);
            else if (key == "version")
                m_version = value.toUInt32();
            else if (key == "hppext")
                m_hppExt = value;
            else if (key == "cppext")
                m_cppExt = value;
        }
    }

    deletePtr(is);
}

O3D_CONSOLE_MAIN(Main, O3D_DEFAULT_CLASS_SETTINGS)
