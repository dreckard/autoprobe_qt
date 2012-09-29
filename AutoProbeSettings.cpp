/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#include "CRADFileIO.h"
#include "AutoProbeSettings.h"

bool AutoProbeSettings::Read( const std::string &file )
{
    return m_settingsFile.Read( file );
}

bool AutoProbeSettings::Write( const std::string &file )
{
    return m_settingsFile.Write( file );
}
