/**
 * @file datafile.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "datafile.h"
#include "main.h"
#include <o3d/core/localfile.h>
#include <o3d/core/filemanager.h>
#include <o3d/core/integer.h>
#include <o3d/core/char.h>
#include <o3d/core/localdir.h>
#include <o3d/core/stringtokenizer.h>
#include <o3d/gui/integervalidator.h>
#include "memberfactory.h"
#include "membercustomref.h"
#include "membercustomarray.h"
#include "tokenizer.h"

using namespace o3d;
using namespace o3d::dmg;

DataFile::DataFile(
        const String &path,
        const String &filename,
        const String &suffix,
        Bool composite) :
    m_composite(composite),
    m_pathname(path),
    m_filename(filename),
    m_suffix(suffix)
{
    Int32 s = m_filename.reverseFind('/');  
    m_prefix = m_filename.sub(s+1, m_filename.length() - 4);

    m_pathname.trimRight('/');
    m_pathname.replace('/', '.');

    // update the CPP reader headers
    UInt32 u = Main::instance()->getInPath().count('/');
    UInt32 v = m_filename.count('/');

    m_relPath = "";

    if (v > u+1)
    {
        for (UInt32 i = 0; i < v-u-1; ++i)
        {
            m_relPath += "../";
        }
    }
}

DataFile::~DataFile()
{
    for (std::pair<String,Data*> entry : m_data)
    {
        deletePtr(entry.second);
    }
}

Data::~Data()
{
    for (UInt32 i = 0; i < 4; ++i)
    {
        for (Member *m : members[i])
        {
            deletePtr(m);
        }
    }
}

Member *Data::getMember(const String &name)
{
    for (UInt32 i = 0; i < 4; ++i)
    {
        for (Member *m : members[i])
        {
            if (m->getName() == name)
                return m;
        }
    }

    return nullptr;
}

const String &DataFile::getName() const
{
    return m_prefix;
}

void DataFile::parseTypedefFile()
{
    System::print(m_filename, "Parse type def file");

    InStream *is = FileManager::instance()->openInStream(m_filename);

    // simple and unique pass
    try {
        parseTypedefFile(is);
    } catch (E_BaseException &e)
    {
        System::print(e.getMsg(), e.getDescr() + " in " + m_filename, System::MSG_ERROR);

        deletePtr(is);
        O3D_ERROR(E_InvalidFormat(String("Error parsing ") + m_filename));
    }

    deletePtr(is);
}

void DataFile::parseClassFile()
{
    System::print(m_filename, "Parse data file");

    InStream *is = FileManager::instance()->openInStream(m_filename);

    try {
        // first pass... resolve imports and objects names
        parseClassFile(is, 0, 0);
        is->reset(0);
        // reset list of parsed file, before doing the second pass...
        m_importedDmg.clear();
        // second pass... parse content of objects
        parseClassFile(is, 0, 1);
    } catch (E_BaseException &e)
    {
        System::print(e.getMsg(), e.getDescr() + " in " + m_filename, System::MSG_ERROR);

        deletePtr(is);
        O3D_ERROR(E_InvalidFormat(String("Error parsing ") + m_filename));
    }

    deletePtr(is);

    // compute the min size of each data
    for (std::pair<String, Data*> data : m_data)
    {
        for (int i = 0; i < 4; ++i)
        {
            for (Member *member : data.second->members[i])
            {
                data.second->minSize += member->getMinSize();
            }
        }
    }
}

void DataFile::process()
{
    System::print(m_filename, "Process data file");

    // attribute id to objects
    for (std::pair<String, Data*> entry : m_data)
    {
        // auto id
        if (entry.second->id == 0)
            entry.second->id = Main::instance()->getNextDataId();
    }

    for (Data *data : m_ref)
    {
        for (UInt32 t = 0; t < 4; ++t)
        {
            m_currentType = (TargetType)t;

            T_StringList headers;
            for (const String &h : data->identifierMeta[t].defaultEntry.headers)
            {
                headers.push_back("\"" + m_relPath + h + "\"");
            }

            updateHeader(headers, F_CPP);
        }
    }

    // resolve the resting templates values
    for (std::pair<String, Data*> entry : m_data)
    {
        Data *data = entry.second;

        // get unresolved template initializers from inherited classes
        Data *pdata = data->directInherit;
        while (pdata)
        {
            for (UInt32 i = 0; i < 4; ++i)
            {
                for (Member *m : pdata->members[i])
                {
                    // need to resolve
                    if (m->getValue().startsWith("<"))
                    {
                        data->initializers.push_back(m);
                    }
                }
            }

            pdata = pdata->directInherit;
        }

        // resolve initializers
        for (Member *member : data->initializers)
        {
            if (member->getValue().startsWith("<"))
            {
                // resolve the template value
                String tpl = member->getValue();
                tpl.trimLeft('<');
                tpl.trimRight('>');

                for (TemplateParam &tmpl : data->templatesParams)
                {
                    if (tmpl.name == tpl)
                    {
                        if (!tmpl.resolved)
                            O3D_ERROR(E_InvalidParameter("Unresolved template parameters " + tmpl.name));

                        member->setValue(tmpl.value);
                        break;
                    }
                }
            }
        }
    }

    // write for profiles
    for (Int32 p = 0; p < 3; ++p)
    {
        Profile profile = (Profile)p;

        String outHppPath = Main::instance()->getOutHppPath(profile);
        String outCppPath = Main::instance()->getOutCppPath(profile);

        // profile to target type
        m_currentType = (TargetType)(p + 1);

        if (profile == DISPLAYER || profile == AUTHORITY)
        {
            writeDataReaderClass(outHppPath, Main::instance()->getHppExt(), profile);
            writeDataReaderImpl(outCppPath, Main::instance()->getCppExt(), profile);
            writeDataReaderUserImpl(outCppPath, Main::instance()->getCppExt(), profile);
        }
        else if (profile == EDITOR)
        {
            writeDataWriterClass(outHppPath, Main::instance()->getHppExt(), profile);
            writeDataWriterImpl(outCppPath, Main::instance()->getCppExt(), profile);
        }
    }
}

void DataFile::addMember(TargetType type, const String &name, Member *member)
{
    for (std::pair<String, Data*> msg : m_data)
    {
        if (msg.first == name)
        {
            msg.second->members[type].push_back(member);
            break;
        }
    }
}

void DataFile::addMember(
        TargetType target,
        Data *data,
        Member *member,
        Member *parent)
{
    if (parent)
    {
        if (parent->findMember(member->getName()))
            O3D_ERROR(E_InvalidParameter("Member name exists: " + member->getName()));

        parent->addMember(member);
    }
    else
    {
        for (Member *m : data->members[target])
        {
            if (m->getName() == member->getName())
                O3D_ERROR(E_InvalidParameter("Member name exists: " + member->getName()));
        }

        data->members[target].push_back(member);
    }
}

void DataFile::parseClassFile(InStream *is, UInt32 importLevel, Int32 pass)
{
    String line;
    m_is = is;

    while (is->readLine(line) != EOF)
    {
        // clean
        if (importLevel == 0)
            m_currentImport.destroy();

        m_currentImportLevel = importLevel;

        // trim white spaces
        line.trimLeftChars(" \t");
        line.trimRightChars(" \t");

        m_currentLine = line;
        m_currentType = T_COMMON;

        // ignore comment lines
        if (line.startsWith("#"))
            continue;

        if (line.isEmpty())
            continue;

        // import statement
        else if (line.startsWith("import"))
        {
            importData(line, importLevel+1, pass);
        }
        // using a typedef
        else if (line.startsWith("using"))
        {
            importTypedef(line, pass);
        }
        // a typedef
        else if (line.startsWith("typedef"))
        {
            parseTypeDef(is, line, pass);
        }
        // a data
        else if (line.startsWith("data"))
        {
            parseData(is, line, pass);
            // consume
            m_templatesArgs.clear();
            m_templateSpe = False;
        }
        // an abstract data
        else if (line.startsWith("abstract"))
        {
            parseData(is, line, pass);
            // consume
            m_templatesArgs.clear();
            m_templateSpe = False;
        }
        // a template data
        else if (line.startsWith("template"))
        {
            // fill m_templatesArgs
            parseTemplate(is, line);
        }
    }
}

void DataFile::parseTypedefFile(InStream *is)
{
    String line;
    m_is = is;

    while (is->readLine(line) != EOF)
    {
        // trim white spaces
        line.trimLeftChars(" \t");
        line.trimRightChars(" \t");

        m_currentLine = line;
        m_currentType = T_COMMON;

        // ignore comment lines
        if (line.startsWith("#"))
            continue;

        if (line.isEmpty())
            continue;

        // a typedef
        if (line.startsWith("typedef"))
        {
            parseTypeDef(is, line, 0);
        }
        else
        {
            O3D_ERROR(E_InvalidFormat("Only typedef is supported in typedef file" + m_filename));
        }
    }
}

void DataFile::importTypedef(const String &_line, Int32 pass)
{
    if (pass > 0)
        return;

    StringTokenizer tk(_line, " ");
    String import;
    String name;

    if (tk.hasMoreTokens())
        import = tk.nextToken();
    else
        O3D_ERROR(E_InvalidFormat("missing import declaration in " + m_filename));

    if (tk.hasMoreTokens())
        name = tk.nextToken();
    else
        O3D_ERROR(E_InvalidFormat("missing import target name in " + m_filename));

    name.replace('.', '/');

    String filename = Main::instance()->getInPath() + "/" + name + ".tdg";

    // avoid redondant cyclic imports
    for (String &fname : m_importedDmg)
    {
        if (fname == filename)
            return;
    }

    System::print(filename, "Import type def file");

    InStream *is = FileManager::instance()->openInStream(filename);

    try {
        parseTypedefFile(is);
    } catch (E_BaseException &e)
    {
        System::print(e.getMsg(), e.getDescr() + " in " + filename, System::MSG_ERROR);

        deletePtr(is);
        O3D_ERROR(E_InvalidFormat(String("Error parsing ") + filename));
    }

    deletePtr(is);
}

void DataFile::importData(const String &_line, UInt32 importLevel, Int32 pass)
{
    StringTokenizer tk(_line, " ");
    String import;
    String name;

    if (tk.hasMoreTokens())
        import = tk.nextToken();
    else
        O3D_ERROR(E_InvalidFormat("missing import declaration in " + m_filename));

    if (tk.hasMoreTokens())
        name = tk.nextToken();
    else
        O3D_ERROR(E_InvalidFormat("missing import target name in " + m_filename));

    name.replace('.', '/');

    String filename = Main::instance()->getInPath() + "/" + name + ".dmg";

    // cannot import itself
    if (filename == m_filename)
        return;

    // avoid redondant cyclic, and multiples imports
    for (String &fname : m_importedDmg)
    {
        if (fname == filename)
            return;
    }

    System::print(filename, "Import data file");

    InStream *is = FileManager::instance()->openInStream(filename);

    try {
        if (m_pathname.isValid())
        {
            String fin = FileManager::instance()->getFullFileName(filename);
            String fcn = FileManager::instance()->getFullFileName(m_filename);

            String ap, bp, af, bf;
            FileManager::getFileNameAndPath(fin, af, ap);
            FileManager::getFileNameAndPath(fcn, bf, bp);

            String a = ap;
            a.remove(bp);

            String b = bp;
            b.remove(ap);

            if (ap == bp)
            {
                // same path
                m_currentImport = "";
            }
            else if (b < bp)
            {
                // imported data is before
                UInt32 n = b.count('/');

                m_currentImport = "";
                for (UInt32 i = 0; i < n; ++i)
                {
                    m_currentImport += "../";
                }
            }
            else
            {
                // imported data is in another branch
                StringTokenizer ta(ap, "/");
                StringTokenizer tb(bp, "/");

                String prefix;
                String suffix;

                while (ta.hasMoreTokens() && tb.hasMoreTokens())
                {
                    a = ta.nextToken();
                    b = tb.nextToken();

                    if (a != b)
                    {
                        prefix += "../";
                        suffix += a + "/";
                    }
                }

                while (tb.hasMoreTokens())
                {
                    tb.nextToken();
                    prefix += "../";
                }

                m_currentImport = prefix + suffix;
            }

            m_currentImport += af;
            m_currentImport.trimRight(".dmg");

            // imported file
            if (pass == 0)
                m_imports.push_back(m_currentImport);

            m_currentImport += "Data." + Main::instance()->getHppExt();
        }
        else
        {
            String ap, af;
            FileManager::getFileNameAndPath(filename, af, ap);

            af.trimRight(".dmg");

            // imported file
            if (pass == 0)
                m_imports.push_back(af);

            m_currentImport = af + "Data." + Main::instance()->getHppExt();
        }

        // imported file
        if (pass == 0)
            m_imports.push_back(m_currentImport);

        m_importedDmg.push_back(filename);

        parseClassFile(is, importLevel, pass);
    } catch (E_BaseException &e)
    {
        System::print(e.getMsg(), e.getDescr() + " in " + filename, System::MSG_ERROR);

        deletePtr(is);
        O3D_ERROR(E_InvalidFormat(String("Error parsing ") + filename));
    }

    deletePtr(is);
}

void DataFile::parseTypeDef(InStream *is, const String &_line, Int32 pass)
{
    Bool begin = False;

    Tokenizer tk(_line, "{}");
    String token;
    String name;
    Int32 state = -1, nextState = 0;

    while ((token = tk.nextToken()).isValid())
    {
        if (state != nextState)
        {
            state = nextState;
        }

        if (state == 0)
        {
            if (token != "typedef")
                O3D_ERROR(E_InvalidParameter("missing typedef keyword"));

            nextState = 1;
        }
        else if (state == 1)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidParameter("type name must be a litteral"));

            name = token;
            nextState = 2;
        }
        else if (state == 2)
        {
            if (token == "{")
            {
                begin = True;
                nextState = 3;
            }
        }
        else if (state == 3)
        {
            O3D_ERROR(E_InvalidParameter("End of line excpeted after opening bracket {"));
        }
    }

    String outTypeName;
    T_StringList headers;
    String line;

    while (is->readLine(line) != EOF)
    {
        // trim white spaces
        line.trimLeftChars(" \t");
        line.trimRightChars(" \t");
        line.replace('\t', ' ');

        // ignore comment lines
        if (line.startsWith("#"))
            continue;

        if (line.isEmpty())
            continue;

        if (line.startsWith("{"))
        {
            // begin
            if (begin)
                O3D_ERROR(E_InvalidFormat("opening bracket { must not succed another {"));

            begin = True;
        }
        else
        {
            if (!begin)
                O3D_ERROR(E_InvalidFormat("missing prior opening bracket {"));

            if (line.startsWith("}"))
            {
                // end
                if (line.length() > 1)
                    O3D_ERROR(E_InvalidFormat("ending bracket } line must only contain ending bracket"));

                break;
            }

            // header
            if (line.startsWith("header "))
            {
                String header = line;
                header.remove("header ");
                header.trimLeft(' ');

                headers.push_back(header);
            }
            // class name
            else if (line.startsWith("class "))
            {
                outTypeName = line;
                outTypeName.remove("class ");
                outTypeName.trimLeft(' ');
            }
            // a const
            else
                O3D_ERROR(E_InvalidFormat("unsuported keyword"));
        }
    }

    if (pass == 0)
    {
        MemberCustom *member = new MemberCustom(nullptr);
        member->setTypeName(name);
        member->setOutTypeName(outTypeName);
        member->setHeaders(headers);

        MemberFactory::instance()->registerMember(member);

        MemberCustomArray *memberArray = new MemberCustomArray(nullptr);
        memberArray->setTypeName(name + "[]");
        memberArray->setOutTypeName(outTypeName);
        memberArray->setHeaders(headers);

        MemberFactory::instance()->registerMember(memberArray);

        MemberCustomRef *memberRef = new MemberCustomRef(nullptr);
        memberRef->setTypeName(name + "&");
        memberRef->setOutTypeName(outTypeName);
        memberRef->setHeaders(headers);

        MemberFactory::instance()->registerMember(memberRef);
    }
}

void DataFile::parseIdentifier(InStream *is, const String &_line, Data *data)
{
    Tokenizer tk(_line, "");
    String token;
    String type;
    String name;

    Int32 state = -1, nextState = 0;

    while ((token = tk.nextToken()).isValid())
    {
        if (nextState != state)
        {
            state = nextState;
        }

        if (state == 0)
        {
            if (token != "identifier")
               O3D_ERROR(E_InvalidFormat("missing identifier keyword"));

            nextState = 1;
        }
        else if (state == 1)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("invalid type format"));

            type = token;
            nextState = 2;
        }
        else if (state == 2)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("invalid type name format"));

            name = token;
            nextState = 3;
        }
        else if (state == 5)
        {
            O3D_ERROR(E_InvalidFormat("end of line excepted"));
        }
    }

    Member *member = MemberFactory::instance()->buildFromTypeName(type, nullptr);
    member->setName(name);

    // as identifier (must be unique)
    if (data->identifier)
        O3D_ERROR(E_InvalidOperation("identifier must be unique"));

    data->identifier = member;

    // and as member for read/write and variable member
    addMember(T_COMMON, data, member, nullptr);
}

void DataFile::parseTarget(InStream *is, const String &_line, Data *data)
{
    Bool begin = False;

    // parse the condition
    Tokenizer tk(_line, "{}");
    String token;
    Int32 state = -1, nextState = 0;

    while ((token = tk.nextToken()).isValid())
    {
        if (state != nextState)
        {
            state = nextState;
        }

        if (state == 0)
        {
            if (token != "target")
                O3D_ERROR(E_InvalidParameter("missing target keyword"));

            nextState = 1;
        }
        else if (state == 1)
        {
            if (token == "displayer")
            {
                m_currentType = T_DISPLAYER;
            }
            else if (token == "authority")
            {
                m_currentType = T_AUTHORITY;
            }
            else if (token == "editor")
            {
                m_currentType = T_EDITOR;
            }
            else
                O3D_ERROR(E_InvalidParameter("invalid target name"));

            nextState = 2;
        }
        else if (state == 2)
        {
            if (token == "{")
            {
                begin = True;
                nextState = 3;
            }
        }
        else if (state == 3)
        {
            O3D_ERROR(E_InvalidParameter("End of line excpeted after opening bracket {"));
        }
    }

    String line;
    Bool ispublic;

    while (is->readLine(line) != EOF)
    {
        // trim white spaces
        line.trimLeftChars(" \t");
        line.trimRightChars(" \t");
        line.replace('\t', ' ');

        // ignore comment lines
        if (line.startsWith("#"))
            continue;

        if (line.isEmpty())
            continue;

        if (line.startsWith("{"))
        {
            // begin
            if (begin)
                O3D_ERROR(E_InvalidFormat("opening bracket { must not succed another {"));

            begin = True;
        }
        else
        {
            ispublic = False;

            if (!begin)
                O3D_ERROR(E_InvalidFormat("missing prior opening bracket {"));

            if (line.startsWith("}"))
            {
                // to common scope
                m_currentType = T_COMMON;

                // end
                if (line.length() > 1)
                    O3D_ERROR(E_InvalidFormat("ending bracket } line must only contain ending bracket"));

                break;
            }

            // public const declaration
            if (line.startsWith("public "))
            {
                ispublic = True;
                line.remove(0, 6);
                line.trimLeftChars(" ");
            }

            // an annotation
            if (line.startsWith("@"))
            {
                parseAnnotation(is, line, data);
            }
            // a loop
            else if (line.startsWith("loop "))
            {
                parseDataLoop(is, line, data, nullptr);
            }
            // a condition
            else if (line.startsWith("if "))
            {
                O3D_ERROR(E_InvalidFormat("if in if is forbidden"));
                parseDataIf(is, line, data, nullptr);
            }
            // a const
            else if (line.startsWith("const "))
            {
                // should be a const member
                parseDataConst(is, line, data, nullptr, ispublic);
            }
            // a bit const
            else if (line.startsWith("bit "))
            {
                // should be a const member
                parseDataBit(is, line, data, nullptr);
            }
            // a static sized array
            else if (line.find('[') != -1)
            {
                // should be a const member
                parseDataArray(is, line, data, nullptr);
            }
            else
            {
                // should be a membre
                parseDataMember(is, line, data, nullptr);
            }
        }
    }
}

void DataFile::writeDataReaderClass(const String &outPath, const String &hppExt, Profile profile)
{
    // only if concrete message to export
    Bool something = False;
    for (std::pair<String, Data*> entry : m_data)
    {
        if (!entry.second->abstract && entry.second->importLevel == 0)
        {
            something = True;
            break;
        }
    }

    if (!something)
        return;

    LocalDir dir(outPath + "/" + m_pathname);
    if (!dir.exists())
    {
        dir.cdUp();
        dir.makeDir(m_pathname);
    }

    String filename = FileManager::instance()->getFullFileName(outPath + "/" + m_pathname + "/" + m_prefix + "Data." + hppExt);
    FileOutStream *os = FileManager::instance()->openOutStream(filename, FileOutStream::CREATE);



    try {
        for (const String &hppLine : Main::instance()->getTemplate(Main::TPL_HPP))
        {
            String outLine = hppLine;
            Int32 p1;
            Int32 p2;

            if (outLine.isEmpty())
                os->writeLine("");

            // block
            else if ((p1 = outLine.sub("@{", 0)) != -1)
            {
                p1 = 0;
                p2 = outLine.find('}', p1+2);

                if (p2 == -1)
                {
                    deletePtr(os);
                    O3D_ERROR(E_InvalidFormat("Missing ending bracket } after @{ in hpp.template"));
                }

                String blockName = outLine.sub(p1+2, p2);
                if (blockName == "license")
                {
                    // license
                    for (const String &line : Main::instance()->getTemplate(Main::TPL_LICENCE))
                    {
                        os->writeLine(line);
                    }
                }
                else if (blockName == "content")
                {
                    // classes predeclarations
                    for (String &clazz : m_preClass)
                    {
                        os->writeLine("class " + clazz + ";");
                    }

                    if (m_preClass.size())
                        os->writeLine("");

                    // classes
                    for (std::pair<String,Data*> entry : m_data)
                    {
                        if (!entry.second->abstract && entry.second->importLevel == 0)
                        {
                            writeDataReaderClassContent(os, entry.second, profile);
                        }
                    }
                }
                else if (blockName == "includes")
                {
                    // include
                    for (const String &header : m_includes[T_COMMON][F_HPP])
                    {
                        os->writeLine(String("#include ") + header);
                    }

                    for (const String &header : m_includes[m_currentType][F_HPP])
                    {
                        os->writeLine(String("#include ") + header);
                    }
                }
            }
            else
            {
                // line with 0 or many variables
                parseVariable(outLine, nullptr, m_prefix + m_suffix, m_suffix, profile);
                os->writeLine(outLine);
            }
        }
    } catch (E_BaseException &e)
    {
        deletePtr(os);
        throw;
    }

    deletePtr(os);
}

void DataFile::writeDataReaderImpl(const String &outPath, const String &cppExt, Profile profile)
{
    // only if concrete message to export
    Bool something = False;
    for (std::pair<String, Data*> entry : m_data)
    {
        if (!entry.second->abstract && entry.second->importLevel == 0)
        {
            something = True;
            break;
        }
    }

    if (!something)
        return;

    LocalDir dir(outPath + "/" + m_pathname);
    if (!dir.exists())
    {
        dir.cdUp();
        dir.makeDir(m_pathname);
    }

    String filename = FileManager::instance()->getFullFileName(outPath + "/" + m_pathname + "/" + m_prefix + "Data." + cppExt);
    FileOutStream *os = FileManager::instance()->openOutStream(filename, FileOutStream::CREATE);

    try {
        for (const String &cppLine : Main::instance()->getTemplate(Main::TPL_CPP))
        {
            String outLine = cppLine;
            Int32 p1;
            Int32 p2;

            if (outLine.isEmpty())
                os->writeLine("");

            // block
            else if ((p1 = outLine.sub("@{", 0)) != -1)
            {
                p1 = 0;
                p2 = outLine.find('}', p1+2);

                if (p2 == -1)
                {
                    deletePtr(os);
                    O3D_ERROR(E_InvalidFormat("Missing ending bracket } after @{ in cpp.template"));
                }

                String blockName = outLine.sub(p1+2, p2);
                if (blockName == "license")
                {
                    // license
                    for (const String &line : Main::instance()->getTemplate(Main::TPL_LICENCE))
                    {
                        os->writeLine(line);
                    }
                }
                else if (blockName == "content")
                {
                    for (std::pair<String,Data*> entry : m_data)
                    {
                        if (!entry.second->abstract && entry.second->importLevel == 0)
                        {
                            writeDataReaderImplContent(os, entry.second, profile);
                        }
                    }
                }
                else if (blockName == "includes")
                {
                    // include
                    String includes = Main::instance()->getIncludePath(profile);

                    if (includes.isValid())
                    {
                        for (const String &header : m_includes[T_COMMON][F_CPP])
                        {
                            if (header.startsWith("\""))
                            {
                                String h = header;
                                h.trimLeft('"');

                                os->writeLine(String("#include ") + "\"" + includes + "/" + h);
                            }
                            else
                                os->writeLine(String("#include ") + includes + "/" + header);
                        }

                        for (const String &header : m_includes[m_currentType][F_CPP])
                        {
                            if (header.startsWith("\""))
                            {
                                String h = header;
                                h.trimLeft('"');
                                h.remove("../");

                                os->writeLine(String("#include ") + "\"" + includes + "/" + h);
                            }
                            else
                                os->writeLine(String("#include ") + includes + "/" + header);
                        }
                    }
                    else
                    {
                        for (const String &header : m_includes[T_COMMON][F_CPP])
                        {
                            os->writeLine(String("#include ") + header);
                        }

                        for (const String &header : m_includes[m_currentType][F_CPP])
                        {
                            os->writeLine(String("#include ") + header);
                        }
                    }
                }
            }
            else
            {
                // line with 0 or many variables
                parseVariable(outLine, nullptr, m_prefix + m_suffix, m_suffix, profile);
                os->writeLine(outLine);
            }
        }
    } catch (E_BaseException &e)
    {
        deletePtr(os);
        throw;
    }

    deletePtr(os);
}

void DataFile::writeDataReaderUserImpl(const String &outPath, const String &cppExt, Profile profile)
{
    // only if concrete message to export
    Bool something = False;
    for (std::pair<String, Data*> entry : m_data)
    {
        if (!entry.second->abstract && entry.second->importLevel == 0)
        {
            something = True;
            break;
        }
    }

    if (!something)
        return;

    LocalDir dir(outPath + "/" + m_pathname);
    if (!dir.exists())
    {
        dir.cdUp();
        dir.makeDir(m_pathname);
    }

    String filename = FileManager::instance()->getFullFileName(outPath + "/" + m_pathname + "/" + m_prefix + "Data.user." + cppExt);

    LocalFile fileInfo(filename);
    if (fileInfo.exists())
        return;

    FileOutStream *os = FileManager::instance()->openOutStream(filename, FileOutStream::CREATE);

    try {
        for (const String &cppLine : Main::instance()->getTemplate(Main::TPL_CPP))
        {
            String outLine = cppLine;
            Int32 p1;
            Int32 p2;

            if (outLine.isEmpty())
                os->writeLine("");

            // block
            else if ((p1 = outLine.sub("@{", 0)) != -1)
            {
                p1 = 0;
                p2 = outLine.find('}', p1+2);

                if (p2 == -1)
                {
                    deletePtr(os);
                    O3D_ERROR(E_InvalidFormat("Missing ending bracket } after @{ in cpp.template"));
                }

                String blockName = outLine.sub(p1+2, p2);
                if (blockName == "license")
                {
                    // license
                    for (const String &line : Main::instance()->getTemplate(Main::TPL_LICENCE))
                    {
                        os->writeLine(line);
                    }
                }
                else if (blockName == "content")
                {
                    for (std::pair<String,Data*> entry : m_data)
                    {
                        if (!entry.second->abstract && entry.second->importLevel == 0)
                        {
                            writeDataReaderUserImplContent(os, entry.second, profile);
                        }
                    }
                }
                else if (blockName == "includes")
                {
                    // include
                    String includes = Main::instance()->getIncludePath(profile);

                    if (includes.isValid())
                    {
                        for (const String &header : m_includes[T_COMMON][F_CPP])
                        {
                            if (header.startsWith("\""))
                            {
                                String h = header;
                                h.trimLeft('"');

                                os->writeLine(String("#include ") + "\"" + includes + "/" + h);
                            }
                            else
                                os->writeLine(String("#include ") + includes + "/" + header);
                        }

                        for (const String &header : m_includes[m_currentType][F_CPP])
                        {
                            if (header.startsWith("\""))
                            {
                                String h = header;
                                h.trimLeft('"');
                                h.remove("../");

                                os->writeLine(String("#include ") + "\"" + includes + "/" + h);
                            }
                            else
                                os->writeLine(String("#include ") + includes + "/" + header);
                        }
                    }
                    else
                    {
                        for (const String &header : m_includes[T_COMMON][F_CPP])
                        {
                            os->writeLine(String("#include ") + header);
                        }

                        for (const String &header : m_includes[m_currentType][F_CPP])
                        {
                            os->writeLine(String("#include ") + header);
                        }
                    }
                }
            }
            else
            {
                // line with 0 or many variables
                parseVariable(outLine, nullptr, m_prefix + m_suffix, m_suffix, profile);
                os->writeLine(outLine);
            }
        }
    } catch (E_BaseException &e)
    {
        deletePtr(os);
        throw;
    }

    deletePtr(os);
}

void DataFile::writeDataReaderClassContent(OutStream *os, Data *data, Profile profile)
{
    TargetType targetType = TargetType(profile + 1);

    for (const String &inClassLine : Main::instance()->getTemplate(Main::TPL_DATA_READER_CLASS))
    {
        String outLine = inClassLine;
        Int32 p1;
        Int32 p2;

        if (outLine.isEmpty())
            os->writeLine("");

        // block
        else if ((p1 = outLine.sub("@{", 0)) != -1)
        {
            p2 = outLine.find('}', p1+2);

            if (p2 == -1)
            {
                deletePtr(os);
                O3D_ERROR(E_InvalidFormat("Missing ending bracket } after @{ in msg.in.class.template"));
            }

            String blockName = outLine.sub(p1+2, p2);
            if (blockName == "initializers")
            {
                for (Member *member : data->initializers)
                {
                    // write only if resolved
                    if (!member->getValue().startsWith("<"))
                        os->writeLine("        " + member->getPrefixedName() + " = " + member->getValue() + ";");
                }
            }
            else if (blockName == "private_members")
            {
                for (Member *member : data->members[T_COMMON])
                {
                    if (member->isPrivate())
                        member->writeDecl(os);
                }
                for (Member *member : data->members[targetType])
                {
                    if (member->isPrivate())
                        member->writeDecl(os);
                }
            }
            else if (blockName == "public_members")
            {
                for (Member *member : data->members[T_COMMON])
                {
                    if (member->isPublic())
                        member->writeDecl(os);
                }
                for (Member *member : data->members[targetType])
                {
                    if (member->isPublic())
                        member->writeDecl(os);
                }
            }
            else if (blockName == "getters")
            {
                for (Member *member : data->members[T_COMMON])
                {
                    member->writeGetterDecl(os);
                }
                for (Member *member : data->members[targetType])
                {
                    member->writeGetterDecl(os);
                }
            }
        }
        else
        {
            // line with 0 or many variables
            parseVariable(outLine, data, m_prefix + m_suffix, m_suffix, profile);
            os->writeLine(outLine);
        }
    }

    os->writeLine("");
}

void DataFile::writeDataReaderImplContent(OutStream *os, Data *data, Profile profile)
{
    Context ctx;
    ctx.data = data;
    ctx.os = os;
    ctx.profile = profile;
    ctx.target = m_currentType;

    for (const String &inImplLine : Main::instance()->getTemplate(Main::TPL_DATA_READER_IMPL))
    {
        String outLine = inImplLine;
        Int32 p1;
        Int32 p2;

        if (outLine.isEmpty())
            os->writeLine("");

        // block
        else if ((p1 = outLine.sub("@{", 0)) != -1)
        {
            p2 = outLine.find('}', p1+2);

            if (p2 == -1)
            {
                deletePtr(os);
                O3D_ERROR(E_InvalidFormat("Missing ending bracket } after @{ in msg.in.class.template"));
            }

            String blockName = outLine.sub(p1+2, p2);
            if (blockName == "readFromFile")
            {
                // inherited class
                if (data->directInherit)
                {
                    String line = "    " + data->directInherit->name + m_suffix + "::readFromFile(is);";
                    os->writeLine(line);
                    os->writeLine("");
                }

                for (Member *member : data->members[T_COMMON])
                {
                    member->writeRead(os);
                }

                for (Member *member : data->members[m_currentType])
                {
                    member->writeRead(os);
                }
            }
            else if (blockName == "finalize")
            {
                for (Member *member : data->finalizers)
                {
                    member->writeFinalize(ctx);
                }
            }
        }
        else
        {
            // line with 0 or many variables
            parseVariable(outLine, data, m_prefix + m_suffix, m_suffix, profile);
            os->writeLine(outLine);
        }
    }

    os->writeLine("");
}

void DataFile::writeDataReaderUserImplContent(OutStream *os, Data *data, Profile profile)
{
    for (const String &inImplLine : Main::instance()->getTemplate(Main::TPL_DATA_READER_USER_IMPL))
    {
        String outLine = inImplLine;
        Int32 p1;
        Int32 p2;

        if (outLine.isEmpty())
            os->writeLine("");

        // block
        else if ((p1 = outLine.sub("@{", 0)) != -1)
        {
            p2 = outLine.find('}', p1+2);

            if (p2 == -1)
            {
                deletePtr(os);
                O3D_ERROR(E_InvalidFormat("Missing ending bracket } after @{ in msg.in.class.template"));
            }

            //String blockName = outLine.sub(p1+2, p2);
        }
        else
        {
            // line with 0 or many variables
            parseVariable(outLine, data, m_prefix + m_suffix, m_suffix, profile);
            os->writeLine(outLine);
        }
    }

    os->writeLine("");
}

void DataFile::writeDataWriterClass(const String &outPath, const String &hppExt, DataFile::Profile profile)
{
    // only if concrete message to export
    Bool something = False;
    for (std::pair<String, Data*> entry : m_data)
    {
        if (!entry.second->abstract && entry.second->importLevel == 0)
        {
            something = True;
            break;
        }
    }

    if (!something)
        return;

    LocalDir dir(outPath + "/" + m_pathname);
    if (!dir.exists())
    {
        dir.cdUp();
        dir.makeDir(m_pathname);
    }

    String filename = FileManager::instance()->getFullFileName(outPath + "/" + m_pathname + "/" + m_prefix + "Data." + hppExt);
    FileOutStream *os = FileManager::instance()->openOutStream(filename, FileOutStream::CREATE);

    try {
        for (const String &hppLine : Main::instance()->getTemplate(Main::TPL_HPP))
        {
            String outLine = hppLine;
            Int32 p1;
            Int32 p2;

            if (outLine.isEmpty())
                os->writeLine("");

            // block
            else if ((p1 = outLine.sub("@{", 0)) != -1)
            {
                p1 = 0;
                p2 = outLine.find('}', p1+2);

                if (p2 == -1)
                {
                    deletePtr(os);
                    O3D_ERROR(E_InvalidFormat("Missing ending bracket } after @{ in hpp.template"));
                }

                String blockName = outLine.sub(p1+2, p2);
                if (blockName == "license")
                {
                    // license
                    for (const String &line : Main::instance()->getTemplate(Main::TPL_LICENCE))
                    {
                        os->writeLine(line);
                    }
                }
                else if (blockName == "content")
                {
                    // classes predeclarations
                    for (String &clazz : m_preClass)
                    {
                        os->writeLine("class " + clazz + ";");
                    }

                    if (m_preClass.size())
                        os->writeLine("");

                    // classes declarations
                    for (std::pair<String,Data*> entry : m_data)
                    {
                        if (!entry.second->abstract && entry.second->importLevel == 0)
                        {
                            writeDataWriterClassContent(os, entry.second, profile);
                        }
                    }
                }
                else if (blockName == "includes")
                {
                    // include
                    for (const String &header : m_includes[T_COMMON][F_CPP])
                    {
                        os->writeLine(String("#include ") + header);
                    }

                    for (const String &header : m_includes[m_currentType][F_CPP])
                    {
                        os->writeLine(String("#include ") + header);
                    }
                }
            }
            else
            {
                // line with 0 or many variables
                parseVariable(outLine, nullptr, m_prefix + m_suffix, m_suffix, profile);
                os->writeLine(outLine);
            }
        }
    } catch (E_BaseException &e)
    {
        deletePtr(os);
        throw;
    }

    deletePtr(os);
}

void DataFile::writeDataWriterImpl(const String &outPath, const String &cppExt, DataFile::Profile profile)
{
    // only if concrete message to export
    Bool something = False;
    for (std::pair<String, Data*> entry : m_data)
    {
        if (!entry.second->abstract && entry.second->importLevel == 0)
        {
            something = True;
            break;
        }
    }

    if (!something)
        return;

    LocalDir dir(outPath + "/" + m_pathname);
    if (!dir.exists())
    {
        dir.cdUp();
        dir.makeDir(m_pathname);
    }

    String filename = FileManager::instance()->getFullFileName(outPath + "/" + m_pathname + "/" + m_prefix + "Data." + cppExt);
    FileOutStream *os = FileManager::instance()->openOutStream(filename, FileOutStream::CREATE);

    try {
        for (const String &cppLine : Main::instance()->getTemplate(Main::TPL_CPP))
        {
            String outLine = cppLine;
            Int32 p1;
            Int32 p2;

            if (outLine.isEmpty())
                os->writeLine("");

            // block
            else if ((p1 = outLine.sub("@{", 0)) != -1)
            {
                p1 = 0;
                p2 = outLine.find('}', p1+2);

                if (p2 == -1)
                {
                    deletePtr(os);
                    O3D_ERROR(E_InvalidFormat("Missing ending bracket } after @{ in cpp.template"));
                }

                String blockName = outLine.sub(p1+2, p2);
                if (blockName == "license")
                {
                    // license
                    for (const String &line : Main::instance()->getTemplate(Main::TPL_LICENCE))
                    {
                        os->writeLine(line);
                    }
                }
                else if (blockName == "content")
                {
                    for (std::pair<String,Data*> entry : m_data)
                    {
                        if (!entry.second->abstract && entry.second->importLevel == 0)
                        {
                            writeDataWriterImplContent(os, entry.second, profile);
                        }
                    }
                }
                else if (blockName == "includes")
                {
                    // include
                    String includes = Main::instance()->getIncludePath(profile);

                    if (includes.isValid())
                    {
                        for (const String &header : m_includes[T_COMMON][F_CPP])
                        {
                            if (header.startsWith("\""))
                            {
                                String h = header;
                                h.trimLeft('"');

                                os->writeLine(String("#include ") + "\"" + includes + "/" + h);
                            }
                            else
                                os->writeLine(String("#include ") + includes + "/" + header);
                        }

                        for (const String &header : m_includes[m_currentType][F_CPP])
                        {
                            if (header.startsWith("\""))
                            {
                                String h = header;
                                h.trimLeft('"');
                                h.remove("../");

                                os->writeLine(String("#include ") + "\"" + includes + "/" + h);
                            }
                            else
                                os->writeLine(String("#include ") + includes + "/" + header);
                        }
                    }
                    else
                    {
                        for (const String &header : m_includes[T_COMMON][F_CPP])
                        {
                            os->writeLine(String("#include ") + header);
                        }

                        for (const String &header : m_includes[m_currentType][F_CPP])
                        {
                            os->writeLine(String("#include ") + header);
                        }
                    }
                }
            }
            else
            {
                // line with 0 or many variables
                parseVariable(outLine, nullptr, m_prefix + m_suffix, m_suffix, profile);
                os->writeLine(outLine);
            }
        }
    } catch (E_BaseException &e)
    {
        deletePtr(os);
        throw;
    }

    deletePtr(os);
}

void DataFile::writeDataWriterClassContent(OutStream *os, Data *data, DataFile::Profile profile)
{
    TargetType targetType = TargetType(profile + 1);

    for (const String &outClassLine : Main::instance()->getTemplate(Main::TPL_DATA_WRITER_CLASS))
    {
        String outLine = outClassLine;
        Int32 p1;
        Int32 p2;

        if (outLine.isEmpty())
            os->writeLine("");

        // block
        else if ((p1 = outLine.sub("@{", 0)) != -1)
        {
            p2 = outLine.find('}', p1+2);

            if (p2 == -1)
            {
                deletePtr(os);
                O3D_ERROR(E_InvalidFormat("Missing ending bracket } after @{ in msg.in.class.template"));
            }

            String blockName = outLine.sub(p1+2, p2);
            if (blockName == "initializers")
            {
                // resolve initializers
                for (Member *member : data->initializers)
                {
                    // write only if resolved
                    if (!member->getValue().startsWith("<"))
                        os->writeLine("        " + member->getPrefixedName() + " = " + member->getValue() + ";");
                }
            }
            else if (blockName == "private_members")
            {
                for (Member *member : data->members[T_COMMON])
                {
                    if (member->isPrivate())
                        member->writeDecl(os);
                }
                for (Member *member : data->members[targetType])
                {
                    if (member->isPrivate())
                        member->writeDecl(os);
                }
            }
            else if (blockName == "public_members")
            {
                for (Member *member : data->members[T_COMMON])
                {
                    if (member->isPublic())
                        member->writeDecl(os);
                }
                for (Member *member : data->members[targetType])
                {
                    if (member->isPublic())
                        member->writeDecl(os);
                }
            }
            else if (blockName == "setters")
            {
                for (Member *member : data->members[T_COMMON])
                {
                    member->writeSetterDecl(os);
                }
                for (Member *member : data->members[targetType])
                {
                    member->writeSetterDecl(os);
                }
            }
            else if (blockName == "getters")
            {
                for (Member *member : data->members[T_COMMON])
                {
                    member->writeGetterDecl(os);
                }
                for (Member *member : data->members[targetType])
                {
                    member->writeGetterDecl(os);
                }
            }
        }
        else
        {
            // line with 0 or many variables
            parseVariable(outLine, data, m_prefix + m_suffix, m_suffix, profile);
            os->writeLine(outLine);
        }
    }

    os->writeLine("");
}

void DataFile::writeDataWriterImplContent(OutStream *os, Data *data, DataFile::Profile profile)
{
    for (const String &outImplLine : Main::instance()->getTemplate(Main::TPL_DATA_WRITER_IMPL))
    {
        String outLine = outImplLine;
        Int32 p1;
        Int32 p2;

        if (outLine.isEmpty())
            os->writeLine("");

        // block
        else if ((p1 = outLine.sub("@{", 0)) != -1)
        {
            p2 = outLine.find('}', p1+2);

            if (p2 == -1)
            {
                deletePtr(os);
                O3D_ERROR(E_InvalidFormat("Missing ending bracket } after @{ in msg.out.impl.template"));
            }

            String blockName = outLine.sub(p1+2, p2);
            if (blockName == "writeToFile")
            {
                // inherited class
                if (data->directInherit)
                {
                    String line = "    " + data->directInherit->name + m_suffix + "::writeToFile(os);";
                    os->writeLine(line);
                    os->writeLine("");
                }

                for (Member *member : data->members[T_COMMON])
                {
                    member->writeWrite(os);
                }

                for (Member *member : data->members[m_currentType])
                {
                    member->writeWrite(os);
                }
            }
        }
        else
        {
            // line with 0 or many variables
            parseVariable(outLine, data, m_prefix + m_suffix, m_suffix, profile);
            os->writeLine(outLine);
        }
    }

    os->writeLine("");
}

void DataFile::parseVariable(
        String &outLine,
        Data *data,
        const String &header,
        const String &classSuffix,
        Profile profile)
{
    Int32 p1, p2;

    // line with 0 or many variables
    while ((p1 = outLine.sub("${", 0)) != -1)
    {
        p2 = outLine.find('}', p1+2);

        if (p2 == -1)
            O3D_ERROR(E_InvalidFormat("Missing ending bracket } after @{ in template"));

        String varName = outLine.sub(p1+2, p2);
        outLine.remove(p1, p2-p1+1);

        if (varName == "data")
        {
            outLine.insert(data->name, p1);
        }
        else if (varName == "dataId")
        {
            outLine.insert(String::print("%i", data->id), p1);
        }
        else if (varName == "author")
        {
            outLine.insert(Main::instance()->getAuthor(), p1);
        }
        else if (varName == "yyyy")
        {
            outLine.insert(Main::instance()->getYear(), p1);
        }
        else if (varName == "mm")
        {
            outLine.insert(Main::instance()->getMonth(), p1);
        }
        else if (varName == "dd")
        {
            outLine.insert(Main::instance()->getDay(), p1);
        }
        else if (varName == "ns")
        {
            outLine.insert(Main::instance()->getNamespace(profile), p1);
        }
        else if (varName == "NS")
        {
            String NS = Main::instance()->getNamespace(profile);
            NS.upper();

            outLine.insert(NS, p1);
        }
        else if (varName == "header")
        {
            if (Main::instance()->getIncludePath(profile).isValid())
                outLine.insert(Main::instance()->getIncludePath(profile) + "/" + m_pathname + "/" + header, p1);
            else
                outLine.insert(header, p1);
        }
        else if (varName == "hpp")
        {
            outLine.insert(Main::instance()->getHppExt(), p1);
        }
        else if (varName == "FILENAME")
        {
            String FILENAME = header;
            FILENAME.upper();

            outLine.insert(FILENAME, p1);
        }
        else if (varName == "HPP")
        {
            String HPP = Main::instance()->getHppExt();
            HPP.upper();

            outLine.insert(HPP, p1);
        }
        else if (varName ==  "baseclasses")
        {
            if (data->directInherit)
            {
                String baseclasses = ": public " + data->directInherit->name + classSuffix;
                outLine.insert(baseclasses, p1);
            }
        }
    }
}

void DataFile::updateHeader(const T_StringList &headers, DataFile::FileType fileType)
{
    if (headers.empty())
        return;

    // add the header if necessary
    Bool hd = True;
    for (const String &memberHeader : headers)
    {
        hd = True;
        for (const String &currHeader : m_includes[m_currentType][fileType])
        {
            if (memberHeader == currHeader)
            {
                hd = False;
                break;
            }
        }

        if (hd)
            m_includes[m_currentType][fileType].push_back(memberHeader);
    }
}

void DataFile::updateClasses(const String &classname)
{
    for (const String &c : m_preClass)
    {
        if (c == classname)
            return;
    }

    m_preClass.push_back(classname);
}

void DataFile::parseData(InStream *is, const String &_line, Int32 pass)
{
    Tokenizer tk(_line, ":{}<>,");
    String type;
    String data;
    String inheritFrom;
    String token;
    std::vector<String> templateValues;
    Bool begin = False;

    UInt32 id = 0;
    Data *pdata = nullptr;

    Int32 state = -1, nextState = 0;
    while ((token = tk.nextToken()).isValid())
    {
        // ignore spaces
        if (token == " ")
            continue;

        if (nextState != state)
        {
            state = nextState;
        }

        if (state == 0)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("must be a litteral"));

            if (token != "data" && token != "abstract")
                O3D_ERROR(E_InvalidFormat("Expected data or absract keyword"));

            type = token;
            nextState = 1;
        }
        if (state == 1)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("data name must be a litteral"));

            data = token;
            nextState = 2;

            if (pass == 0)
            {
                // register the data
                auto it = m_data.find(data);
                if (it != m_data.end())
                {
                    pdata = it->second;
                }
                else
                {
                    // we instanciate here because we can have some references into
                    pdata = new Data;
                    pdata->name = data;
                    pdata->importLevel = m_currentImportLevel;
                    pdata->templatesArgs = m_templatesArgs;

                    if (type == "abstract")
                        pdata->abstract = True;

                    m_data.insert(std::make_pair(data, pdata));
                }

                // create
                if (it == m_data.end())
                {
                    // and register it as a custom member

                    // simple
                    MemberCustom *member = new MemberCustom(nullptr);
                    member->setTypeName(data);
                    member->setOutTypeName(data + m_suffix);
                    member->setTemplatesArgs(pdata->templatesArgs);
                    MemberFactory::instance()->registerMember(member);

                    // array
                    MemberCustomArray *memberArray = new MemberCustomArray(nullptr);
                    memberArray->setTypeName(data + "[]");
                    memberArray->setOutTypeName(data + m_suffix);
                    memberArray->setTemplatesArgs(pdata->templatesArgs);
                    MemberFactory::instance()->registerMember(memberArray);

                    // reference
                    MemberCustomRef *memberRef = new MemberCustomRef(nullptr);
                    memberRef->setTypeName(data + "&");
                    memberRef->setOutTypeName(data + m_suffix);
                    memberRef->setTemplatesArgs(pdata->templatesArgs);
                    MemberFactory::instance()->registerMember(memberRef);

                    // find the corresponding header into the import list
                    T_StringList headers;
                    for (const String &header : m_imports)
                    {
                        if (!header.endsWith(data))
                            continue;

                        // add corresponding header
                        Int32 p = header.sub(data, 0);
                        if (p == -1)
                        {
                            break;
                        }
                        else if (p == 0)
                        {
                            headers.push_back("\"" + header + "Data." + Main::instance()->getHppExt() + "\"");
                            break;
                        }
                        else if (p > 0)
                        {
                            if (header[p-1] == '/')
                            {
                                headers.push_back("\"" + header + "Data." + Main::instance()->getHppExt() + "\"");
                                break;
                            }
                        }
                    }

                    member->setHeaders(headers);
                    memberArray->setHeaders(headers);
                }
                // update
                else
                {
                    if (!m_templateSpe)
                        pdata->importLevel = m_currentImportLevel;
                }
            }
            else if (pass == 1)
            {
                auto it = m_data.find(data);
                if (it == m_data.end())
                    O3D_ERROR(E_InvalidFormat("data not found at second pass!"));

                pdata = it->second;
            }
        }
        else if (state == 2)
        {
            if (token == ":")
                nextState = 3;
            else if (token == "<")
            {
                if (!m_templateSpe)
                    O3D_ERROR(E_InvalidFormat("< but not template specialization"));

                nextState = 5;
            }
            else if (token == "{")
                nextState = 10;
            else
                O3D_ERROR(E_InvalidFormat("Excpected : or {"));
        }
        else if (state == 3)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("inherited data or abstract name must be a litteral"));

            inheritFrom = token;
            nextState = 4;
        }
        else if (state == 4)
        {
            if (token == "{")
                nextState = 10;
            else if (token == "<")
                nextState = 5;
            else
                O3D_ERROR(E_InvalidFormat("Excpected < or {"));
        }
        else if (state == 5)
        {
            if (token == ",")
                continue;
            if (token == ">")
                nextState = 6;
            else
                templateValues.push_back(token);
        }
        else if (state == 6)
        {
            if (token == "{")
                nextState = 10;
        }
        else if (state == 10)
        {
            O3D_ERROR(E_InvalidFormat("opening bracket { is only permited as last caracter of a line"));
        }
    }

    if (nextState == 10)
        begin = True;

    if (pass == 0 && pdata->passed == -1 && !m_templateSpe)
    {
        pdata->passed = 0;
        pdata->id = id;
        pdata->importLevel = m_currentImportLevel;

        if (data.isEmpty())
            O3D_ERROR(E_InvalidFormat("data name must be defined"));

        // auto id
        if (id != 0)
            Main::instance()->registerDataId(id);

        // inheritance
        if (inheritFrom.isValid())
        {
            auto it = m_data.find(inheritFrom);
            if (it == m_data.end())
                O3D_ERROR(E_InvalidFormat("Undefined data " + inheritFrom));

            pdata->directInherit = it->second;

            if (pdata->directInherit->templatesArgs.size() != templateValues.size())
                O3D_ERROR(E_InvalidFormat("missing template value at data inheritance" + inheritFrom));

            // templates values if defined (only if inheritance)
            UInt32 i = 0;
            for (String &arg : pdata->directInherit->templatesArgs)
            {
                pdata->templatesParams.push_back(TemplateParam());
                TemplateParam &tpl = pdata->templatesParams.back();

                tpl.name = arg;
                tpl.value = templateValues[i];

                // assume resolved
                tpl.resolved = True;

                // search for a template argument in data, if found, template param
                // will be resolve later
                for (String &dataArg : pdata->templatesArgs)
                {
                    if (dataArg == tpl.value)
                    {
                        tpl.resolved = False;
                        break;
                    }
                }

                ++i;
            }
        }

        System::print(data, "Add data");
    }
    else if (pass == 1 && pdata->passed == 0 && !m_templateSpe)
    {
        pdata->passed = 1;

        // header of the inherited class
        if (pdata->directInherit)
        {
            T_StringList headers;
            for (const String &header : m_imports)
            {
                if (!header.endsWith(pdata->directInherit->name))
                    continue;

                // add corresponding header
                Int32 p = header.sub(pdata->directInherit->name, 0);
                if (p == -1)
                {
                    break;
                }
                else if (p == 0)
                {
                    headers.push_back("\"" + header + "Data." + Main::instance()->getHppExt() + "\"");
                    break;
                }
                else if (p > 0)
                {
                    if (header[p-1] == '/')
                    {
                        headers.push_back("\"" + header + "Data." + Main::instance()->getHppExt() + "\"");
                        break;
                    }
                }
            }

            if (m_currentImportLevel == 0)
                updateHeader(headers, F_HPP);
        }

        parseDataInt(is, begin, pdata);
    }
    else if (pass == 1 && pdata->passed == 1 && m_templateSpe)
    {
        m_templatesValue = templateValues;
        parseDataInt(is, begin, pdata);
    }
}

void DataFile::parseTemplate(InStream *is, const String &_line)
{
    Tokenizer tk(_line, "<>,");
    String token;
    String type;
    String name;
    String value;

    Int32 state = -1, nextState = 0;

    while ((token = tk.nextToken()).isValid())
    {
        if (nextState != state)
        {
            state = nextState;
        }

        if (state == 0)
        {
            if (token != "template")
               O3D_ERROR(E_InvalidFormat("missing template keyword"));

            nextState = 1;
        }
        else if (state == 1)
        {
            if (token != "<")
                O3D_ERROR(E_InvalidFormat("excpeced <"));

            nextState = 2;
        }
        else if (state == 2)
        {
            if (token == ">")
                nextState = 4;
            else if (token == ",")
                continue;
            else
            {
                if (!tk.isName(token))
                    O3D_ERROR(E_InvalidFormat("invalid template arg name format"));

                for (String &tpl : m_templatesArgs)
                {
                    if (tpl == token)
                        O3D_ERROR(E_InvalidFormat("already used template arg name"));
                }

                m_templatesArgs.push_back(token);
            }
        }
        else if (state == 3)
        {
            if (token != "=")
                O3D_ERROR(E_InvalidFormat("excepted ="));

            nextState = 4;
        }
        else if (state == 4)
        {
            O3D_ERROR(E_InvalidFormat("end of line excepted"));
        }
    }

    if (m_templatesArgs.size() == 0)
        m_templateSpe = True;
}

void DataFile::parseDataInt(InStream *is, Bool begin, Data *data)
{
    // inject members of inherited data if abstract or m_composite is enable
    // we do it here, because here we are in the second pass
    if (data->directInherit && (data->directInherit->abstract || m_composite))
    {
        for (UInt32 i = 0; i < 4; ++i)
        {
            data->members[i] = data->directInherit->members[i];
        }

        data->finalizers = data->directInherit->finalizers;
        data->identifier = data->directInherit->identifier;
        data->statics = data->directInherit->statics;
        data->externs = data->directInherit->externs;

        data->isTemplate = data->directInherit->isTemplate;
        data->templatesArgs = data->directInherit->templatesArgs;
        data->templatesParams = data->directInherit->templatesParams;

        data->identifier = data->directInherit->identifier;
    }

    // now parse the data scope
    m_currentType = T_COMMON;

    String line;
    Bool ispublic;

    // copy template args
    data->templatesArgs = m_templatesArgs;

    while (is->readLine(line) != EOF)
    {
        // trim white spaces
        line.trimLeftChars(" \t");
        line.trimRightChars(" \t");
        line.replace('\t', ' ');

        // ignore comment lines
        if (line.startsWith("#"))
            continue;

        if (line.isEmpty())
            continue;

        if (line.startsWith("{"))
        {
            // begin
            if (begin)
                O3D_ERROR(E_InvalidFormat("opening bracket { must not succed another {"));

            begin = True;
        }
        else
        {
            ispublic = False;

            if (!begin)
                O3D_ERROR(E_InvalidFormat("missing prior opening bracket {"));

            if (line.startsWith("}"))
            {
                // end
                if (line.length() > 1)
                    O3D_ERROR(E_InvalidFormat("ending bracket } line must only contains ending bracket"));

                break;
            }

            // public declaration
            if (line.startsWith("public "))
            {
                ispublic = True;
                line.remove(0, 6);
                line.trimLeftChars(" ");
            }

            // a loop
            if (line.startsWith("loop "))
            {
                parseDataLoop(is, line, data, nullptr);
            }
            // a condition
            else if (line.startsWith("if "))
            {
                parseDataIf(is, line, data, nullptr);
            }
            else if (line.startsWith("}"))
            {
                // end
                break;
            }
            else if (line.startsWith("@"))
            {
                // annotation
                parseAnnotation(is, line, data);
            }
            else if (line.startsWith("target "))
            {
                // should be a target block
                parseTarget(is, line, data);
            }
            else if (line.startsWith("identifier "))
            {
                // should be a type member
                parseIdentifier(is, line, data);
            }
            else if (line.startsWith("const "))
            {
                // should be a const member
                parseDataConst(is, line, data, nullptr, ispublic);
            }
            // a bit const
            else if (line.startsWith("bit "))
            {
                // should be a const member
                parseDataBit(is, line, data, nullptr);
            }
            // a static sized array
            else if (line.find('[') != -1)
            {
                // should be a const member
                parseDataArray(is, line, data, nullptr);
            }
            else
            {
                // should be a membre
                parseDataMember(is, line, data, nullptr);
            }
        }
    }
}

void DataFile::parseDataLoop(
        InStream *is,
        const String &_line,
        Data *data,
        Member *parent)
{
    Bool begin = False;

    // parse the condition
    Tokenizer tk(_line, "{}:");
    String token;
    String loopName;
    String counterVarName;
    String counterVarParam;

    Int32 state = -1, nextState = 0;

    while ((token = tk.nextToken()).isValid())
    {
        if (state != nextState)
        {
            state = nextState;
        }

        if (state == 0)
        {
            if (token != "loop")
                O3D_ERROR(E_InvalidParameter("missing loop keyword"));

            nextState = 1;
        }
        else if (state == 1)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidParameter("loop name must be a litteral"));

            loopName = token;
            nextState = 2;

        }
        else if (state == 2)
        {
            if (token != ":")
                O3D_ERROR(E_InvalidParameter(": excpected"));

            nextState = 3;
        }
        else if (state == 3)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidParameter("counter variable name must be a litteral"));

            counterVarName = token;
            nextState = 4;
        }
        else if (state == 4)
        {
            if (token == "[")
                nextState = 5;
            else
            {
                tk.cancel();
                nextState = 10;
            }
        }
        else if (state == 5)
        {
            if (!tk.isName(token) && !UInteger32::isInteger(token))
                O3D_ERROR(E_InvalidParameter("counter paramater must be a litteral on a immediate unsigned integer"));

            counterVarParam = token;
            nextState = 6;
        }
        else if (state == 6)
        {
            if (token != "]")
                O3D_ERROR(E_InvalidParameter("] excpected"));

            nextState = 10;
        }
        else if (state == 10)
        {
            if (token == "{")
            {
                begin = True;
                nextState = 20;
            }
        }
        else if (state == 20)
        {
            O3D_ERROR(E_InvalidParameter("End of line excpeted after opening bracket {"));
        }
    }

    if (nextState < 10)
        O3D_ERROR(E_InvalidParameter("invalid condition expression"));

    Member *varMember = nullptr;
    for (Member *m : data->members[m_currentType])
    {
        if (m->getName() == counterVarName)
        {
            varMember = m;
            break;
        }
    }
    // may be in the common
    if (!varMember)
    {
        for (Member *m : data->members[T_COMMON])
        {
            if (m->getName() == counterVarName)
            {
                varMember = m;
                break;
            }
        }
    }

    if (!varMember && parent)
        varMember = parent->findMember(counterVarName);

    if (!varMember)
        O3D_ERROR(E_InvalidParameter("unable to find the counter variable " + counterVarName));

    Member *constMember = nullptr;

    // create the const member if counterVarParam is defined and it is not an integer
    if (counterVarParam.isValid())
    {
        if (UInteger32::isInteger(counterVarParam))
        {
            constMember = MemberFactory::instance()->buildFromTypeName("immediate", nullptr);
            constMember->setName(UInteger32::toString(varMember->getNewUIntId()));

            addMember(m_currentType, data, constMember, nullptr);
        }
        else
        {
            constMember = MemberFactory::instance()->buildFromTypeName("const uint32", nullptr);
            constMember->setName(counterVarParam);
            constMember->setValue(UInteger32::toString(varMember->getNewUIntId()));

            addMember(m_currentType, data, constMember, nullptr);
        }
    }

    // create the loop member
    Member *member = MemberFactory::instance()->buildFromTypeName("loop", parent);
    member->setName(loopName);
    member->setCond(varMember, constMember);

    addMember(m_currentType, data, member, parent);

    String line;
    Bool ispublic;

    while (is->readLine(line) != EOF)
    {
        // trim white spaces
        line.trimLeftChars(" \t");
        line.trimRightChars(" \t");
        line.replace('\t', ' ');

        // ignore comment lines
        if (line.startsWith("#"))
            continue;

        if (line.isEmpty())
            continue;

        if (line.startsWith("{"))
        {
            // begin
            if (begin)
                O3D_ERROR(E_InvalidFormat("opening bracket { must not succed another {"));

            begin = True;
        }
        else
        {
            ispublic = False;

            if (!begin)
                O3D_ERROR(E_InvalidFormat("missing prior opening bracket {"));

            if (line.startsWith("}"))
            {
                // end
                if (line.length() > 1)
                    O3D_ERROR(E_InvalidFormat("ending bracket } line must only contain ending bracket"));

                break;
            }

            // public declaration
            if (line.startsWith("public "))
            {
                ispublic = True;
                line.remove(0, 6);
                line.trimLeftChars(" ");
            }

            // a loop
            if (line.startsWith("loop "))
            {
                O3D_ERROR(E_InvalidFormat("loop in loop is forbidden"));
                //parseMessageLoop(is, line, msg, member);
            }
            // a condition
            else if (line.startsWith("if "))
            {
                O3D_ERROR(E_InvalidFormat("if in loop is forbidden"));
                //parseMessageIf(is, line, msg, member);
            }
            // a const
            else if (line.startsWith("const "))
            {
                // should be a const member
                parseDataConst(is, line, data, member, ispublic);
            }
            // a bit const
            else if (line.startsWith("bit "))
            {
                // should be a const member
                parseDataBit(is, line, data, member);
            }
            // a static sized array
            else if (line.find('[') != -1)
            {
                // should be a const member
                parseDataArray(is, line, data, member);
            }
            else
            {
                // should be a membre
                parseDataMember(is, line, data, member);
            }
        }
    }
}

void DataFile::parseDataIf(
        InStream *is,
        const String &_line,
        Data *data,
        Member *parent)
{
    Bool begin = False;

    // parse the condition
    Tokenizer tk(_line, "{}[]");
    String token;
    String condVarName;
    String condVarParam;

    Int32 state = -1, nextState = 0;

    while ((token = tk.nextToken()).isValid())
    {
        if (state != nextState)
        {
            state = nextState;
        }

        if (state == 0)
        {
            if (token != "if")
                O3D_ERROR(E_InvalidParameter("missing if keyword"));

            nextState = 1;
        }
        else if (state == 1)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidParameter("condition variable must be a litteral"));

            condVarName = token;
            nextState = 2;
        }
        else if (state == 2)
        {
            if (token != "[")
                O3D_ERROR(E_InvalidParameter("[ excpected"));

            nextState = 3;
        }
        else if (nextState == 3)
        {
            if (!tk.isName(token) && !UInteger32::isInteger(token))
                O3D_ERROR(E_InvalidParameter("condition paramater must be a litteral on a immediate unsigned integer"));

            condVarParam = token;
            nextState = 4;
        }
        else if (nextState == 4)
        {
            if (token != "]")
                O3D_ERROR(E_InvalidParameter("] excpected"));

            nextState = 10;
        }
        else if (state == 10)
        {
            if (token == "{")
            {
                begin = True;
                nextState = 20;
            }
        }
        else if (state == 20)
        {
            O3D_ERROR(E_InvalidParameter("End of line excpeted after opening bracket {"));
        }
    }

    if (nextState < 10)
        O3D_ERROR(E_InvalidParameter("invalid condition expression"));

    Member *varMember = nullptr;
    for (Member *m : data->members[m_currentType])
    {
        if (m->getName() == condVarName)
        {
            varMember = m;
            break;
        }
    }
    // may be in the common
    if (!varMember)
    {
        for (Member *m : data->members[T_COMMON])
        {
            if (m->getName() == condVarName)
            {
                varMember = m;
                break;
            }
        }
    }

    if (!varMember && parent)
        varMember = parent->findMember(condVarName);

    if (!varMember)
        O3D_ERROR(E_InvalidParameter("unable to find the condition variable " + condVarName));

    Member *constMember = nullptr;

    // create the const member if condVarParam is defined and it is not an integer
    if (condVarParam.isValid())
    {
        if (UInteger32::isInteger(condVarParam))
        {
            constMember = MemberFactory::instance()->buildFromTypeName("immediate", nullptr);
            constMember->setName(UInteger32::toString(varMember->getNewUIntId()));

            addMember(m_currentType, data, constMember, nullptr);
        }
        else
        {
            constMember = MemberFactory::instance()->buildFromTypeName("const uint32", nullptr);
            constMember->setName(condVarParam);
            constMember->setValue(UInteger32::toString(varMember->getNewUIntId()));

            addMember(m_currentType, data, constMember, nullptr);
        }
    }

    // create the if member
    Member *member = MemberFactory::instance()->buildFromTypeName("if", parent);
    member->setName("if");
    member->setCond(varMember, constMember);

    addMember(m_currentType, data, member, parent);

    String line;
    Bool ispublic;

    while (is->readLine(line) != EOF)
    {
        // trim white spaces
        line.trimLeftChars(" \t");
        line.trimRightChars(" \t");
        line.replace('\t', ' ');

        // ignore comment lines
        if (line.startsWith("#"))
            continue;

        if (line.isEmpty())
            continue;

        if (line.startsWith("{"))
        {
            // begin
            if (begin)
                O3D_ERROR(E_InvalidFormat("opening bracket { must not succed another {"));

            begin = True;
        }
        else
        {
            ispublic = False;

            if (!begin)
                O3D_ERROR(E_InvalidFormat("missing prior opening bracket {"));

            if (line.startsWith("}"))
            {
                // end
                if (line.length() > 1)
                    O3D_ERROR(E_InvalidFormat("ending bracket } line must only contain ending bracket"));

                break;
            }

            // public const declaration
            if (line.startsWith("public "))
            {
                ispublic = True;
                line.remove(0, 6);
                line.trimLeftChars(" ");
            }

            // a loop
            if (line.startsWith("loop "))
            {
                parseDataLoop(is, line, data, member);
            }
            // a condition
            else if (line.startsWith("if "))
            {
                O3D_ERROR(E_InvalidFormat("if in if is forbidden"));
                parseDataIf(is, line, data, member);
            }
            // a const
            else if (line.startsWith("const "))
            {
                // should be a const member
                parseDataConst(is, line, data, member, ispublic);
            }
            // a bit const
            else if (line.startsWith("bit "))
            {
                // should be a const member
                parseDataBit(is, line, data, member);
            }
            // a static sized array
            else if (line.find('[') != -1)
            {
                // should be a const member
                parseDataArray(is, line, data, member);
            }
            else
            {
                // should be a membre
                parseDataMember(is, line, data, member);
            }
        }
    }
}

void DataFile::parseDataMember(
        InStream *is,
        const String &_line,
        Data *data,
        Member *parent)
{
    Tokenizer tk(_line, "&<>=");
    String token;
    String type;
    String name;
    String value;
    T_StringList templateArgs;
    Bool isRef = False;

    Int32 state = -1, nextState = 0;

    while ((token = tk.nextToken()).isValid())
    {
        if (nextState != state)
        {
            state = nextState;
        }

        if (state == 0)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("type name must be a litteral"));

            type = token;
            nextState = 1;
        }
        else if (state == 1)
        {
            if (token == "<")
                nextState = 2;
            else
            {
                tk.cancel();
                nextState = 10;
            }
        }
        else if (state == 2)
        {
            if (token == ">")
                nextState = 10;
            else if (token == ",")
                continue;
            else
            {
                if (!tk.isName(token))
                    O3D_ERROR(E_InvalidFormat("template argument must be a litteral"));

                templateArgs.push_back(token);
            }
        }
        else if (state == 10)
        {
            if (token == "&")
                isRef = True;
            else
                tk.cancel();

            nextState = 11;
        }
        else if (state == 11)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("name must be a litteral"));

            name = token;
            nextState = 12;
        }
        else if (state == 12)
        {
            if (token != "=")
                O3D_ERROR(E_InvalidFormat("= excpected"));

            nextState = 13;
        }
        else if (state == 13)
        {
            // TODO can manage " " for strings
            value = token;
            nextState = 20;
        }
        else if (state == 20)
        {
            O3D_ERROR(E_InvalidFormat("end of line excepted"));
        }
    }

    if (nextState < 12)
        O3D_ERROR(E_InvalidFormat("invalid member expression"));

    Member *member;

    if (isRef) {
        if (value.isValid()) {
            O3D_ERROR(E_InvalidOperation("a reference member cannot have an initial value"));
        }

        member = MemberFactory::instance()->buildFromTypeName(type + "&", parent);
        // MemberCustomRef *memberRef = dynamic_cast<MemberCustomRef*>(member);
        MemberCustomRef *memberRef = static_cast<MemberCustomRef*>(member);

        // TODO identifier type may be took from referenced member class, if referencable...
        Member *identifier = MemberFactory::instance()->buildFromTypeName("int32", parent);
        identifier->setName(name + "Id");

        auto itd = m_data.find(type);
        Data *refData = itd->second;

        // the ref identifier
        memberRef->setRefMember(identifier);
        // related data
        memberRef->setRefData(refData);
        // templates arguments comes from the refdata
        memberRef->setTemplatesArgs(refData->templatesArgs);

        // resolve templates values
        UInt32 i = 0;
        for (const String &v : m_templatesArgs)
        {
            memberRef->setTemplate(i, v, True);
            ++i;
        }

        // need to satisfy this member in finalize
        data->finalizers.push_back(member);

        // class predeclaration, and we dont update the headers
        if (m_currentImportLevel == 0)
        {
            updateClasses(type + m_suffix);

            // add for later inclusion of headers into the cpp files.
            // later, because here we couldnt be sure to have parsed the annotations
            if (std::find(m_ref.begin(), m_ref.end(), refData) == m_ref.end())
                m_ref.push_back(refData);
        }
    }
    else
    {
        member = MemberFactory::instance()->buildFromTypeName(type, parent);

        // initial value
        if (value.isValid())
        {
            member->setValue(value);
            data->initializers.push_back(member);

            // need satisfy a template value
            for (String &tpl : m_templatesArgs)
            {
                // is a template value to satisfy
                if (tpl == value)
                {
                    // satisfy later the template value < > to know it is to satisfy
                    member->setValue("<" + value + ">");
                    break;
                }
            }
        }

        // finalize if the type name refer to a data type name
        if (m_data.find(type) != m_data.end())
        {
            // need to finalize this member
            data->finalizers.push_back(member);
        }

        // add the header if necessary
        if (m_currentImportLevel == 0)
            updateHeader(member->getHeaders(), F_HPP);
    }

    member->setName(name);

    UInt32 i = 0;
    Bool resolved;
    for (const String &tplValue : templateArgs)
    {
        if (std::find(
                    data->templatesArgs.begin(),
                    data->templatesArgs.end(),
                    member->getTemplateName(i)) != data->templatesArgs.end())
        {
            resolved = False;
        }
        else
        {
            resolved = True;
        }

        member->setTemplate(i, tplValue, resolved);
        ++i;
    }

    addMember(m_currentType, data, member, parent);

    System::print(member->getTypeName(), name);
}

void DataFile::parseDataArray(
        InStream *is,
        const String &_line,
        Data *data,
        Member *parent)
{
    Tokenizer tk(_line, "&<>[]");
    String token;
    String type;
    String name;
    String size;
    T_StringList templateArgs;

    Int32 state = -1, nextState = 0;

    while ((token = tk.nextToken()).isValid())
    {
        if (nextState != state)
        {
            state = nextState;
        }

        if (state == 0)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("type name must be a litteral"));

            type = token;
            nextState = 1;
        }
        else if (state == 1)
        {
            if (token == "<")
                nextState = 2;
            else
            {
                tk.cancel();
                nextState = 10;
            }
        }
        else if (state == 2)
        {
            if (token == ">")
                nextState = 10;
            else if (token == ",")
                continue;
            else
            {
                if (!tk.isName(token))
                    O3D_ERROR(E_InvalidFormat("template argument must be a litteral"));

                templateArgs.push_back(token);
            }
        }
        else if (state == 10)
        {
            if (token == "&")
                O3D_ERROR(E_InvalidFormat("& reference is not compatible with array []"));
            else
                tk.cancel();

            nextState = 20;
        }
        else if (state == 20)
        {
            if (token == "[")
            {
                // supose dynamic size
                size = "";
                nextState = 21;
            }
            else
            {
                tk.cancel();
                nextState = 30;
            }
        }
        else if (state == 21)
        {
            if (token == "]")
                nextState = 30;
            else if (UInteger32::isInteger(token))
            {
                // static size
                size = token;
            }
            else
                O3D_ERROR(E_InvalidFormat("excpected integer array size or ]"));
        }
        else if (state == 30)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("name must be a litteral"));

            name = token;
            nextState = 40;
        }
        else if (state == 40)
        {
            O3D_ERROR(E_InvalidFormat("end of line excepted"));
        }
    }

    if (nextState != 40)
        O3D_ERROR(E_InvalidFormat("invalid array member expression"));

    Member *member = MemberFactory::instance()->buildFromTypeName(type + "[]", parent);
    member->setName(name);
    member->setValue(size);

    // finalize if the type name refer to a data type name
    if (m_data.find(type) != m_data.end())
    {
        // need to finalize this member
        data->finalizers.push_back(member);
    }

    // add the header if necessary
    if (m_currentImportLevel == 0)
        updateHeader(member->getHeaders(), F_HPP);

    addMember(m_currentType, data, member, parent);

    System::print(member->getTypeName(), name);
}

void DataFile::parseDataConst(
        InStream *is,
        const String &_line,
        Data *data,
        Member *parent,
        Bool ispublic)
{
    Tokenizer tk(_line, "=");
    String token;
    String type;
    String name;
    String value;

    Int32 state = -1, nextState = 0;

    // for example "const int8 myConst = 0"
    while ((token = tk.nextToken()).isValid())
    {
        if (nextState != state)
        {
            state = nextState;
        }

        if (state == 0)
        {
            if (token != "const")
                O3D_ERROR(E_InvalidFormat("missing const keyword"));

            nextState = 1;
        }
        if (state == 1)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("type name must be a litteral"));

            type = token;
            nextState = 2;
        }
        else if (state == 2)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("const name must be a litteral"));

            name = token;
            nextState = 3;
        }
        else if (state == 3)
        {
            if (token != "=")
                O3D_ERROR(E_InvalidFormat("= excpected"));

            nextState = 4;
        }
        else if (state == 4)
        {
            if (!tk.isName(token) && !Integer32::isInteger(token))
                O3D_ERROR(E_InvalidFormat("const value must be a litteral or an immediate integer"));

            value = token;
            nextState = 10;
        }
        else if (state == 10)
        {
            O3D_ERROR(E_InvalidFormat("end of line excepted"));
        }
    }

    if (nextState != 10)
        O3D_ERROR(E_InvalidFormat("invalid const member expression"));

    Member *member = MemberFactory::instance()->buildFromTypeName("const " + type, parent);
    member->setName(name);
    member->setValue(value);

    // public const
    if (ispublic)
        member->setPublic();

    // add the header if necessary
    if (m_currentImportLevel == 0)
        updateHeader(member->getHeaders(), F_HPP);

    addMember(m_currentType, data, member, parent);

    //System::print(member->getTypeName(), name);
}

void DataFile::parseDataBit(InStream *is, const String &_line, Data *data, Member *parent)
{
    Tokenizer tk(_line, "=");
    String token;
    String type;
    String bitSetVarName;
    String constName;

    Int32 state = -1, nextState = 0;

    // for example "bit myBitset[CONST]"
    while ((token = tk.nextToken()).isValid())
    {
        if (nextState != state)
        {
            state = nextState;
        }

        if (state == 0)
        {
            if (token != "bit")
                O3D_ERROR(E_InvalidFormat("missing bit keyword"));

            nextState = 1;
        }
        if (state == 1)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("type name must be a litteral"));

            bitSetVarName = token;
            nextState = 2;
        }
        else if (state == 2)
        {
            if (token != "[")
                O3D_ERROR(E_InvalidFormat("[ excpected"));

            nextState = 3;
        }
        else if (state == 3)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("const name between [] must be a litteral"));

            constName = token;
            nextState = 4;
        }
        else if (state == 4)
        {
            if (token != "]")
                O3D_ERROR(E_InvalidFormat("] excpected"));

            nextState = 10;
        }
        else if (state == 10)
        {
            O3D_ERROR(E_InvalidFormat("end of line excepted"));
        }
    }

    if (nextState != 10)
        O3D_ERROR(E_InvalidFormat("invalid bit member expression"));

    Member *varMember = nullptr;
    for (Member *m : data->members[m_currentType])
    {
        if (m->getName() == bitSetVarName)
        {
            varMember = m;
            break;
        }
    }
    // may be in common
    if (!varMember)
    {
        for (Member *m : data->members[T_COMMON])
        {
            if (m->getName() == bitSetVarName)
            {
                varMember = m;
                break;
            }
        }
    }

    if (!varMember && parent)
        varMember = parent->findMember(bitSetVarName);

    if (!varMember)
        O3D_ERROR(E_InvalidParameter("unable to find the bitset variable " + bitSetVarName));

    Member *bitMember = nullptr;

    // create the const member if constName is defined and it is not an integer
    if (constName.isValid())
    {
        if (UInteger32::isInteger(constName))
            O3D_ERROR(E_InvalidParameter("const value must be a litteral"));
        else
        {
            bitMember = MemberFactory::instance()->buildFromTypeName("bit", nullptr);
            bitMember->setName(constName);
            bitMember->setCond(varMember, bitMember);
            bitMember->setValue(UInteger32::toString(varMember->getNewUIntId()));

            addMember(m_currentType, data, bitMember, parent);

            //System::print(bitMember->getTypeName(), bitSetVarName);
        }
    }
}

void DataFile::parseDataExtern(InStream *is, const String &line, Data *data, Member *parent)
{
    // TODO
}

void DataFile::parseDataStatic(InStream *is, const String &line, Data *data, Member *parent)
{
    // TODO
}

void DataFile::parseAnnotation(InStream *is, const String &_line, Data *data)
{
    String token;
    String name;

    struct Param
    {
        String name;
        T_StringList values;
    };

    std::list<Param> params;

    Tokenizer tk(_line, "@=,()");
    Int32 state = -1, nextState = 0;

    while ((token = tk.nextToken()).isValid())
    {
        if (nextState != state)
        {
            state = nextState;
        }

        if (state == 0)
        {
            if (token != "@")
                O3D_ERROR(E_InvalidFormat("annotation begin with @"));

            nextState = 1;
        }
        else if (state == 1)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("annotation must be a litteral"));

            name = token;
            nextState = 2;
        }
        else if (state == 2)
        {
            if (!tk.isName(token))
                O3D_ERROR(E_InvalidFormat("annotation parameter must be a litteral"));

            params.push_back(Param());
            params.back().name = token;

            nextState = 3;
        }
        else if (state == 3)
        {
            if (token == "=")
                nextState = 10;
            else
                O3D_ERROR(E_InvalidFormat("annotation parameter in already defined"));
        }
        else if (state == 10)
        {
            if (token != "," && token != "(" && token != ")")
                params.back().values.push_back(token);
        }
        else if (state == 20)
        {
            O3D_ERROR(E_InvalidFormat("end of line excepted"));
        }
    }

    if (nextState < 10)
        O3D_ERROR(E_InvalidFormat("invalid annotation member expression"));

    // build a specialized template string with comma as separator, when we are in
    // a template specialization
    String templateArgs = "";
    if (m_templateSpe)
    {
        for (UInt32 i = 0; i < m_templatesValue.size(); ++i)
        {
            templateArgs += m_templatesValue[i];

            if (i+1 < m_templatesValue.size())
                templateArgs += ",";
        }
    }

    if (name == "identifier")
    {
        for (Param &p : params)
        {
            if (p.name == "headers")
            {
                if (m_templateSpe)
                    data->identifierMeta[m_currentType].templates[templateArgs].headers = p.values;
                else
                    data->identifierMeta[m_currentType].defaultEntry.headers = p.values;
            }
            else if (p.name == "manager")
            {
                if (m_templateSpe)
                    data->identifierMeta[m_currentType].templates[templateArgs].manager = p.values.front();
                else
                data->identifierMeta[m_currentType].defaultEntry.manager = p.values.front();
            }
            else if (p.name == "method")
            {
                if (m_templateSpe)
                {
                    data->identifierMeta[m_currentType].templates[templateArgs].method = p.values.front();
                    p.values.pop_front();

                    data->identifierMeta[m_currentType].templates[templateArgs].params = p.values;
                }
                else
                {
                    data->identifierMeta[m_currentType].defaultEntry.method = p.values.front();
                    p.values.pop_front();

                    data->identifierMeta[m_currentType].defaultEntry.params = p.values;
                }
            }
            else
                O3D_ERROR(E_InvalidFormat("unsupported annotation parameter"));
        }
    }
    else if (name == "form")
    {
        // TODO
    }
    else
        O3D_ERROR(E_InvalidFormat("unsupported annotation typename"));
}
