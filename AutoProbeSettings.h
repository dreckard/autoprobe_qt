/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef AUTOPROBESETTINGS_H
#define AUTOPROBESETTINGS_H

#include "CRADFileIO.h"

class AutoProbeSettings
{
private:
    CRADFileIO_File m_settingsFile;
public:
    unsigned short m_sComPort;
    unsigned int m_iMaxPts;
    unsigned int m_iFTLineScanBlock;
    unsigned int m_iFTPointBlock;
    float m_flFTTimeout;
    unsigned short m_sAutoSaveMode;
    bool m_bHaltOnZeroCurrent;
    bool m_bBlankBeamOnFinish;
    bool m_bAutoresume;
    bool m_bExecPostScript;
    float m_flStageLimits[3][2]; //CBA to type names for all these so [axis][0: min, 1:max]

    AutoProbeSettings()
    {
        m_settingsFile.Clear();
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<unsigned short>( "COM_Port", "1", m_sComPort ) );
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<unsigned int>( "Max_Points", "250000", m_iMaxPts ) );

        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<unsigned int>( "Flextran_LineScanBlock", "250", m_iFTLineScanBlock ) );
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<unsigned int>( "Flextran_PointBlock", "251", m_iFTPointBlock ) );
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<float>( "Flextran_Timeout", "60.000", m_flFTTimeout ) );

        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<float>( "Stage_HardLimit_X_Min", "0.000", m_flStageLimits[0][0] ) );
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<float>( "Stage_HardLimit_X_Max", "80.000" , m_flStageLimits[0][1] ) );
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<float>( "Stage_HardLimit_Y_Min", "0.000", m_flStageLimits[1][0] ) );
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<float>( "Stage_HardLimit_Y_Max", "80.000", m_flStageLimits[1][1] ) );
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<float>( "Stage_HardLimit_Z_Min", "9.500", m_flStageLimits[2][0] ) );
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<float>( "Stage_HardLimit_Z_Max", "12.500", m_flStageLimits[2][1] ) );

        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<unsigned short>( "AutoSaveMode", "1", m_sAutoSaveMode ) );
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<bool>( "HaltOnZeroCurrent", "1", m_bHaltOnZeroCurrent ) );
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<bool>( "BlankBeamOnFinish", "1", m_bBlankBeamOnFinish ) );
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<bool>( "Autoresume", "1", m_bAutoresume ) );
        m_settingsFile.AddEntry( new CRADFileIO_Entry_AutoVar<bool>( "ExecPostScript", "0", m_bExecPostScript ) );
    }

    bool Read( const std::string& file );
    bool Write( const std::string& file );
};

#endif // AUTOPROBESETTINGS_H
