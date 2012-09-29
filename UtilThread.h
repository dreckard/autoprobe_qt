#ifndef UTILTHREAD_H
#define UTILTHREAD_H

#include <string>
#include <QThread>
#include <QObject>
#include <QUrl>
#include "PointData.h"

using namespace std;
class UtilThread : public QThread
{
 Q_OBJECT

public:
    UtilThread() : m_ThreadAction( UNDEFINED )
    {
        //Supposedly this is the one safe way to have it delete itself...just when I thought I liked Qt
        QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    }

    //Note that these must make a copy of the entire point matrix which can be many MB in size (33 bytes per point)
    void ExportTSV( const string& fileName, const Matrix2d<point_s>& matrix, float flDwellTime )
                    { m_ThreadAction = TSV; m_strFilename = fileName; m_Matrix = matrix; m_flDwellTime = flDwellTime; start(); }
    //void ExportCSV( const string& fileName, const Matrix2d<point_s>& matrix, float flDwellTime )
    //                { m_ThreadAction = CSV; m_strFilename = fileName; m_Matrix = matrix; m_flDwellTime = flDwellTime; start(); }

    void OpenURL( const QUrl& url )
                    { m_ThreadAction = URL; m_Url = url; start(); }

private:
    enum UtilThreadAction
    {
        UNDEFINED = 0,
        TSV,
        CSV,
        URL
    } m_ThreadAction;

    string m_strFilename;
    Matrix2d<point_s> m_Matrix;
    float m_flDwellTime;

    QUrl m_Url;

    bool InternalExportTSV( void );
    bool InternalExportCSV( void );

protected:
    void run();

};

#endif // UTILTHREAD_H
