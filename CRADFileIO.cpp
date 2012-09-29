/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#include <string>
#include "CRADFileIO.h"

std::string GetToken( std::string str, size_t startPos = 0, size_t* end = NULL )
{
    size_t pos1 = str.find_first_of( '\"', startPos );
    if ( pos1 == std::string::npos )
        return std::string();
    size_t pos2 = str.find_first_of( '\"', pos1+1 );
    if ( pos2 == std::string::npos )
        return std::string();
    if ( end )
        *end = pos2+1;
    return str.substr( pos1+1, pos2-pos1-1 );
}

bool CRADFileIO_Entry::Write( std::ofstream& ofs )
{
    ofs << '\"' << GetKeyName() << '\"' << ' ' << '\"' << GetString() << '\"' << '\n';
    return !ofs.fail();
}

void CRADFileIO_Entry::Read( const std::string strValue, std::ifstream& ifs )
{
    SetString( strValue );
}

bool CRADFileIO_File::Write( const std::string& strFile )
{
    std::ofstream ofs( strFile.c_str(), std::ofstream::trunc );
    if ( !ofs.good() )
        return false;

    for ( unsigned int i = 0; i < m_vecElements.size(); i++ )
    {
        if ( !m_vecElements[i]->Write( ofs ) )
            return false;
    }

    ofs.close();
    return !ofs.fail();
}

bool CRADFileIO_File::Read( const std::string& strFile )
{
    std::ifstream ifs( strFile.c_str() );
    if ( !ifs.good() )
        return false;

    std::string str, key, value;
    while ( getline( ifs, str ) )
    {
        size_t pos = 0;
        key = GetToken( str, 0, &pos );
        if ( !key.empty() )
        {
            value = GetToken( str, pos );
            if ( !value.empty() )
            {
                CRADFileIO_Entry* pEntry = GetKey( GetToken( str, 0, &pos ) );
                if ( pEntry )
                    pEntry->Read( value, ifs );
                else
                    m_vecElements.push_back( new CRADFileIO_Entry( key, value ) );
            }
        }
    }

    ifs.close();
    return true;
}

CRADFileIO_Entry* CRADFileIO_File::GetKey( const std::string& strKeyName )
{
    for ( unsigned int i = 0; i < m_vecElements.size(); i++ )
    {
        CRADFileIO_Entry* pEntry = m_vecElements[i];
        if ( strKeyName.compare( pEntry->GetKeyName() ) == 0 )
            return pEntry;
    }
    return NULL;
}
