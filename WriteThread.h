/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef WRITETHREAD_H
#define WRITETHREAD_H

#include <QThread>
#include "AutoProbe_Objects.h" //For LScan class
#include "../clib_superprobe733/clib_superprobe733.h"

class WriteThread : public QThread
{
 Q_OBJECT

protected:
    void run();
    void BlankBeam( bool bBlank );

public slots:
    void ScanLine( LScan lscan, int iBlock, float flListenTimeout );
    void RescanLine( LScan lscan, int iBlock, float flListenTimeout );
    void EndRun( bool bBlank );
};

#endif // WRITETHREAD_H
