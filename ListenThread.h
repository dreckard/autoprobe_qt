/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef LISTENTHREAD_H
#define LISTENTHREAD_H

#include <QThread>
#include <time.h>
#include "PointData.h"
#include "../clib_superprobe733/clib_superprobe733.h"

class ListenThread : public QThread, public clib_callback_class
{
 Q_OBJECT

protected:
    bool m_bActive;
    float m_flTimeout;
    clock_t m_Clock;

public:
    ListenThread() : m_bActive( false ), m_flTimeout(0), m_Clock(0) {}

protected:
    void run();
    void dataReceived( const TN5500_Data &data );
    void RefreshClock( void );

signals:
    void sigDataReceived( TN5500_Data );
    void sigListenTimedOut();

public slots:
    void SetActive( bool bActive, float flTimeout );
};

#endif // LISTENTHREAD_H
