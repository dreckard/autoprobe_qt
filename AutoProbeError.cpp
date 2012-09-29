/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <QMessageBox>
#include "AutoProbeError.h"

using namespace std;
void ap_logerror( const string& strEntry, bool bTimestamp /*= true*/ )
{
    ofstream ofs( "ap_error.log", ios_base::out | ios_base::app );
    if ( ofs.good() )
    {
        if ( bTimestamp )
        {
            time_t curtime;
            time( &curtime );
            string time = ctime( &curtime );
            time.resize( time.length()-1 ); //Get rid of newline
            ofs << time << " - ";
        }
        ofs << strEntry << endl;
    }
    ofs.close();
}

//Ordinary error with mbox, optional log
void ap_error( const string& strError, bool bLog /*= false*/ )
{
    if ( bLog )
        ap_logerror( strError );

    QMessageBox msgBox( QMessageBox::Critical, "Error", strError.c_str() );
    msgBox.exec();
}

bool ap_error( bool expr, const string &strError, bool bLog /*=false*/ )
{
    if ( !expr )
        ap_error( strError, bLog );
    return expr;
}

void ap_fatalerror( const string& strError )
{
    ap_logerror( strError );
    QMessageBox msgBox( QMessageBox::Critical, "Fatal error", strError.c_str() );
    msgBox.exec();
    exit( EXIT_FAILURE ); //Goodbye, Dave
}

bool ap_assert( bool expr, const string &strError )
{
    if ( !expr )
        ap_fatalerror( strError );
    return expr;
}
