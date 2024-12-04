/*****************************************************************************/
/* TSafeUTF8.h                            Copyright (c) Ladislav Zezula 2024 */
/*---------------------------------------------------------------------------*/
/* Easy-to-use string conversions MPQ UTF-8 <-> Wide                         */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 27.10.24  1.00  Lad  Created                                              */
/*****************************************************************************/

template <typename SRCCHAR, typename TRGCHAR>
class TSafeUTF8
{
    public:

    TSafeUTF8()
    {
        m_szStr = NULL;
        m_nLen = 0;
    }

    TSafeUTF8(const SRCCHAR * szSrcStr, const SRCCHAR * szSrcEnd = NULL)
    {
        // Initially, set to NULL string
        m_szStr = NULL;
        m_nLen = 0;

        // Set the string
        SetString(szSrcStr, szSrcEnd);
    }

    ~TSafeUTF8()
    {
        FreeBuffer();
    }

    TRGCHAR * SetString(const SRCCHAR * szSrcStr, const SRCCHAR * szSrcEnd = NULL)
    {
        if(szSrcStr != NULL)
        {
            // Reset the internal buffer
            FreeBuffer();

            // Retrieve the length of the UTF-8 string
            if((m_nLen = ConvertString(NULL, 0, szSrcStr, szSrcEnd)) != 0)
            {
                // Allocate buffer if too long
                if((m_nLen + 1) > _countof(m_StaticBuffer))
                {
                    if((m_szStr = new TRGCHAR[m_nLen + 1]) == NULL)
                    {
                        assert(false);
                        return NULL;
                    }
                }
                else
                {
                    m_szStr = m_StaticBuffer;
                }

                // Convert the string
                ConvertString(m_szStr, m_nLen + 1, szSrcStr, szSrcEnd);
            }
        }
        return m_szStr;
    }

    size_t ConvertString(LPWSTR szTrgStr, size_t ccTrgStr, LPCSTR szSrcStr, LPCSTR szSrcEnd)
    {
        size_t nOutLength = 0;

        // Perform the conversion using StormLib's own function
        SMemUTF8ToFileName(szTrgStr, ccTrgStr, szSrcStr, szSrcEnd, 0, &nOutLength);
        return nOutLength;
    }

    size_t ConvertString(LPSTR szTrgStr, size_t ccTrgStr, LPCWSTR szSrcStr, LPCWSTR szSrcEnd)
    {
        size_t nOutLength = 0;

        // Perform the conversion using StormLib's own function
        SMemFileNameToUTF8(szTrgStr, ccTrgStr, szSrcStr, szSrcEnd, 0, &nOutLength);
        return nOutLength;
    }

    void FreeBuffer()
    {
        if(m_szStr != NULL && m_szStr != m_StaticBuffer)
            delete[] m_szStr;
        m_szStr = NULL;
        m_nLen = 0;
    }

    TRGCHAR * Buffer()
    {
        return m_szStr;
    }

    size_t Length()
    {
        return m_nLen;
    }

    operator const TRGCHAR *()
    {
        return m_szStr;
    }

    protected:

    TRGCHAR * m_szStr;
    TRGCHAR m_StaticBuffer[0x80];
    size_t m_nLen;
};

//-----------------------------------------------------------------------------
// Classes for conversions between MPQ UTF-8 and filename safe strings

typedef TSafeUTF8<char, WCHAR> TMPQ8ToWide;
typedef TSafeUTF8<WCHAR, char> TWideToMPQ8;

#define MPQ8_WSTR(sz)   TMPQ8ToWide(sz).Buffer()
