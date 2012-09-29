/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/

#ifdef WIN32
    //Needed for GetConsoleWindow()
    #if _WIN32_WINNT < 0x0500
        #define _WIN32_WINNT 0x0500
    #endif
    #include <windows.h>
#endif

#include <QtGui/QApplication>
#include <QMessageBox>
#include <QDir>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "PointData.h"
#include "ListenThread.h"
#include "WriteThread.h"
#include "CRADFileIO.h"
#include "AutoProbeSettings.h"
#include "AutoProbeError.h"
#include "AutoProbe_Objects.h"
#include "../clib_superprobe733/clib_superprobe733.h"

#ifdef WIN32
    BOOL CtrlHandler( DWORD fdwCtrlType )
    {
        if ( fdwCtrlType == CTRL_C_EVENT || fdwCtrlType == CTRL_BREAK_EVENT )
            return TRUE;

        return FALSE;
    }
#endif

using namespace std;
int main(int argc, char *argv[])
{
    #ifdef WIN32
        if ( AllocConsole() )
        {
            SetConsoleCtrlHandler( (PHANDLER_ROUTINE)CtrlHandler, TRUE );
            HMENU hm = GetSystemMenu(GetConsoleWindow(),false);
            DeleteMenu(hm, SC_CLOSE, MF_BYCOMMAND);
            SetConsoleTitle( L"AutoProbe" );
            ShowWindow( GetConsoleWindow(), SW_HIDE );
        }
    #endif

    QApplication a(argc, argv);

    if ( argc < 1 )
        return 0;

    //The cfg file code just uses cd, so set it to the executable directory
    string str = QDir( argv[0] ).path().toStdString();
    string::size_type pos = str.find_last_of("/\\");
    if ( pos != string::npos )
        QDir::setCurrent( str.substr(0,str.find_last_of("/\\")+1).c_str() );

    if ( !MainWindow::Instance().m_Settings.Read( CFG_FILE ) )
        MainWindow::Instance().m_Settings.Write( CFG_FILE );

    MainWindow::Instance().show();

    if ( !MainWindow::Instance().InitSerial( MainWindow::Instance().m_Settings.m_sComPort ) )
    {
        MainWindow::Instance().ShowOptionsDlg();
        ap_error( "Failed to initialize serial port" );
    }

    qRegisterMetaType<TN5500_Data>("TN5500_Data"); //Needed for queued sigs/slots to work with this type
    qRegisterMetaType<LScan>("LScan");
    MainWindow::Instance().m_pListenThread = new ListenThread;
    MainWindow::Instance().m_pWriteThread = new WriteThread;

    MainWindow::Instance().m_pListenThread->moveToThread(MainWindow::Instance().m_pListenThread);
    MainWindow::Instance().m_pListenThread->start();

    MainWindow::Instance().m_pWriteThread->moveToThread(MainWindow::Instance().m_pWriteThread);
    MainWindow::Instance().m_pWriteThread->start();

    for ( int i = 1; i < argc; i++ )
    {
        string str = argv[i];
        if ( str[0] == '-' )
        {
            if ( str == "-dev" )
            {
                QMessageBox msgBox( QMessageBox::Information, "Dev Mode", "Dev mode enabled" );
                msgBox.exec();
                MainWindow::Instance().EnableDevMode();
            }
        }
        else
        {
            MainWindow::Instance().LoadFile( argv[i] );
            break;
        }
    }

    return a.exec(); //Enter main UI loop for Qt
}
