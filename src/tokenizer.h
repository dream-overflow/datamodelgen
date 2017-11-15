/**
 * @file tokenizer.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DMG_TOKENIZER_H
#define _O3D_DMG_TOKENIZER_H

#include <o3d/core/char.h>
#include <o3d/core/string.h>

namespace o3d {
namespace dmg {

/**
 * @brief Tokenize by delim and return found delims as token.
 * between " " no tokenization is done.
 */
class Tokenizer
{
public:

    Tokenizer(const String &str, const String &delim) :
        m_pos(0),
        m_str(str),
        m_delim(delim),
        m_cancel(False),
        m_string(False)
    {
    }

    String nextToken()
    {
        if (m_cancel)
        {
            m_cancel = False;
            return m_token;
        }

        m_token.destroy();
        WChar c, d;

        for (UInt32 n = m_pos; n < m_str.length(); ++n)
        {
            c = m_str[n];

            if (c == '"')
            {
                m_string = !m_string;
                continue;
            }

            // comment, ignore any with #
            if (m_string)
            {
                m_token += c;
            }
            else
            {
                if (c == '#')
                {
                    m_pos = m_str.length();
                    return "";
                }
                else if (iswspace(c))
                {
                    if (m_token.isEmpty())
                        continue;
                    else
                    {
                        m_pos = n + 1;
                        return m_token;
                    }
                }

                for (UInt32 i = 0; i < m_delim.length(); ++i)
                {
                    d = m_delim[i];

                    if (c == d)
                    {
                        if (m_token.isEmpty())
                        {
                            m_token = c;
                            m_pos = n + 1;
                        }
                        else
                            m_pos = n;

                        return m_token;
                    }
                }

                m_token += c;
            }
        }

        m_pos = m_str.length();
        return m_token;
    }

    /**
     * @brief hasMoreTokens
     * @return True until the end of the string in not reached.
     */
    Bool hasMoreTokens() const
    {
        return m_pos < m_str.length();
    }

    /**
     * @brief cancel the lask token.
     * The next call to nextToken() will returns the previous token.
     */
    void cancel()
    {
        m_cancel = True;
    }

    /**
     * @brief isName
     * @param name
     * @return True if name the format is ([a-z]|[A-Z]|_)+([a-z]|[A-Z]|[0..9])*
     */
    Bool isName(const String &name)
    {
        // name cannot be empty
        if (name.isEmpty())
            return False;

        // name start by alpha or '_'
        if (!WideChar::isAlpha(name[0]) && (name[0] != '_'))
            return False;

        // name continue with alpha or numeric or '_'
        for (UInt32 p = 0; p < name.length(); ++p)
        {
            if (!WideChar::isAlphaNum(name[p]) && (name[p] != '_'))
                return False;
        }

        return True;
    }

private:

    UInt32 m_pos;

    String m_str;
    String m_delim;

    String m_token;
    Bool m_cancel;
    Bool m_string;
};

} // namespace dmg
} // namespace o3d

#endif // _O3D_DMG_TOKENIZER_H
