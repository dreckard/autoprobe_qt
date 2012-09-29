/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#include <time.h>
#include <QObject>
#include <QApplication>
#include "ListenThread.h"
#include "mainwindow.h"
#include "../clib_superprobe733/clib_superprobe733.h"

void ListenThread::dataReceived( const TN5500_Data& data )
{
    RefreshClock();
    emit sigDataReceived( data );
}

void ListenThread::RefreshClock( void )
{
    if ( m_flTimeout > 0.0f )
        m_Clock = clock() + m_flTimeout * CLOCKS_PER_SEC;
    else
        m_Clock = 0;
}

void ListenThread::SetActive( bool bActive, float flTimeout )
{
    m_bActive = bActive;
    if ( bActive )
        m_flTimeout = flTimeout;
    else
        m_flTimeout = 0.0f;

    clib_serial_flushrecv(); //Don't want to read anything that's been sitting in the buffer
    clib_probe_listen_reset();
    RefreshClock();
}

void ListenThread::run( void )
{
    QObject::connect( this, SIGNAL(sigDataReceived(TN5500_Data)), &(MainWindow::Instance()), SLOT(sltDataReceived(TN5500_Data)), Qt::QueuedConnection );
    QObject::connect( this, SIGNAL(sigListenTimedOut()), &(MainWindow::Instance()), SLOT(sltListenTimedOut()), Qt::QueuedConnection );
    while ( 1 )
    {
        if ( m_bActive )
        {
            if ( m_Clock >= clock() )
            {
                clib_probe_listen( this );
                //clib_serial_flushrecv(); //Flush, only "packets" of data are considered valid
            }
            else
            {
                emit sigListenTimedOut();
                m_bActive = false;
            }
        }
        QCoreApplication::processEvents( QEventLoop::AllEvents, 5 );
        msleep( 1 );
    }
}
