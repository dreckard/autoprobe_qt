/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/*        This thread manages serial comms for the run          */
/****************************************************************/
#include <QMetaObject>
#include "WriteThread.h"
#include "mainwindow.h"
#include "../clib_superprobe733/clib_superprobe733.h"

void WriteThread::run( void )
{
    exec();
}

void WriteThread::ScanLine( LScan lscan, int iBlock, float flListenTimeout )
{
    QMetaObject::invokeMethod( (QObject*)(MainWindow::Instance().m_pListenThread), "SetActive", Qt::BlockingQueuedConnection,
                               Q_ARG(bool,false), Q_ARG(float,0.0f) );

    clib_linescan( lscan.pt1()[0], lscan.pt1()[1], lscan.pt1()[2], lscan.pt2()[0], lscan.pt2()[1], lscan.pt2()[2],
                   lscan.NumPoints(), lscan.DwellTime(), iBlock );

    QMetaObject::invokeMethod( (QObject*)(MainWindow::Instance().m_pListenThread), "SetActive", Qt::BlockingQueuedConnection,
                               Q_ARG(bool,true), Q_ARG(float,flListenTimeout) );
}
void WriteThread::RescanLine( LScan lscan, int iBlock, float flListenTimeout )
{
    clib_probe_abortexec();
    ScanLine( lscan, iBlock, flListenTimeout );
}

void WriteThread::EndRun( bool bBlank )
{
    clib_probe_abortexec();
    QMetaObject::invokeMethod( (QObject*)(MainWindow::Instance().m_pListenThread), "SetActive", Qt::BlockingQueuedConnection,
                               Q_ARG(bool,false), Q_ARG(float,0) );
    if ( bBlank )
        clib_blankbeam( true );
}
