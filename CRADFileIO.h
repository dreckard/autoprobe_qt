/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
//-----------------------------------------------------
//Description:
//Simple key/value text based file I/O
//-----------------------------------------------------
#ifndef RAD_FILEIO_H
#define RAD_FILEIO_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

class CRADFileIO_Entry
{
    protected:
        std::string m_strKeyName;
        std::string m_strValue;
    
    public:
        CRADFileIO_Entry( const std::string& strKey ) : m_strKeyName(strKey) {}
        CRADFileIO_Entry( const std::string& strKey, const std::string& strValue )
            : m_strKeyName(strKey), m_strValue(strValue) {}

        const std::string GetKeyName( void ) { return m_strKeyName; }
        virtual bool Write( std::ofstream& ofs );
        virtual void Read( const std::string strValue, std::ifstream& ifs );

        //virtual int GetInt( void ) const { int iValue; std::stringstream s( GetString() ); s >> iValue; return iValue; }
        //virtual float GetFloat( void ) const { float flValue; std::stringstream s( GetString() ); s >> flValue; return flValue; }
        virtual std::string GetString( void ) const { return m_strValue; }
		
        //virtual void SetInt( int iValue ) { std::stringstream s; s << iValue; SetString(s.str()); }
        //virtual void SetFloat( float flValue ) { std::stringstream s; s << flValue; SetString(s.str()); }
        virtual void SetString( const std::string& strValue ) { m_strValue = strValue; }
};

template <typename T>
class CRADFileIO_Entry_AutoVar : public CRADFileIO_Entry
{
protected:
    T& m_dataVar;

public:
    CRADFileIO_Entry_AutoVar( const std::string& strKey, T& dataVar ) : m_dataVar( dataVar ), CRADFileIO_Entry( strKey ) {}
    CRADFileIO_Entry_AutoVar( const std::string& strKey, const std::string& strValue, T& dataVar ) : CRADFileIO_Entry( strKey ),
        m_dataVar( dataVar ) { SetString( strValue ); }

    std::string GetString( void ) const { std::stringstream s; s << m_dataVar; return s.str(); }
    void SetString( const std::string& strValue ) { std::stringstream s( strValue ); s >> m_dataVar; }
};

class CRADFileIO_File
{
    protected:
        vector<CRADFileIO_Entry*> m_vecElements;
    public:
        virtual ~CRADFileIO_File()
        {
            //Delete anything the base class added
            Clear();
        }

        CRADFileIO_Entry* GetKey( const std::string& strKeyName );
        CRADFileIO_Entry* GetKey( unsigned int idx ) { return m_vecElements[idx]; }
        void AddEntry( CRADFileIO_Entry* key ) { m_vecElements.push_back( key ); } //NOTE: This class will delete the object
        virtual bool Write( const std::string& strFile );
        virtual bool Read( const std::string& strFile );

        void Clear( void )
        {
            for ( unsigned int i = 0; i < m_vecElements.size(); i++ )
                delete m_vecElements[i];
            m_vecElements.clear();
        }
};

#endif
