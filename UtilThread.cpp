#include <string>
#include <sstream>
#include <fstream>
#include <QMetaObject>
#include <QDesktopServices>
#include "UtilThread.h"
#include "mainwindow.h"
using namespace std;

//Kids, this is how not to write object oriented code
void UtilThread::run( void )
{
    switch ( m_ThreadAction )
    {
        case TSV:
            if ( !InternalExportTSV() )
            {
                QMetaObject::invokeMethod( MainWindow::Instance().thread(), "sltTriggerError", Qt::QueuedConnection,
                                           Q_ARG( QString, "Export failed\nEnsure that the file is not in use by another program" ) );
            }
            break;
        case CSV:
            if ( !InternalExportCSV() )
            {
                QMetaObject::invokeMethod( MainWindow::Instance().thread(), "sltTriggerError", Qt::QueuedConnection,
                                           Q_ARG( QString, "Export failed\nEnsure that the file is not in use by another program" ) );
            }
            break;
        case URL:
            if ( !QDesktopServices::openUrl(m_Url) )
            {
                QMetaObject::invokeMethod( MainWindow::Instance().thread(), "sltTriggerError", Qt::QueuedConnection,
                                           Q_ARG( QString, "openUrl failed" ) );
            }
            break;
        default:
            break;
    }

    //TODO: Make sure this deletes itself somehow before you start calling new on it
    //assert( 0 );
    //deleteLater();
}

bool UtilThread::InternalExportTSV( void )
{
    string baseFile_trunc = m_strFilename; baseFile_trunc.erase( baseFile_trunc.size() - 4, string::npos );
    const Matrix2d<point_s>& matrix = m_Matrix;
    for ( unsigned int s = 0; s < NUM_SPECTROMETERS; s++ )
    {
        stringstream ss; ss << baseFile_trunc << "_Spec" << s+1 << ".tsv";
        ofstream ofs( ss.str().c_str(), ofstream::trunc );
        if ( !ofs.good() )
            return false;

        for ( unsigned int j = 0; j < matrix.rows(); j++ )
        {
            for ( unsigned int k = 0; k < matrix.cols(); k++ )
            {
                if ( matrix.at(j,k).bCollected && matrix.at(j,k).flAbsCurrent > 0.0f )
                    ofs << matrix.at(j,k).flSpecCounts[s] / (m_flDwellTime*(matrix.at(j,k).flAbsCurrent)) << '\t'; //C/s/nA
                else
                    ofs << 0.0f << "\t";
            }
            ofs << '\n';
        }

        ofs.close();
        if ( ofs.fail() )
            return false;
    }

    //Current
    std::ofstream ofs( (baseFile_trunc + "_Cur.tsv").c_str(), std::ofstream::trunc );
    if ( !ofs.good() )
        return false;

    for ( unsigned int j = 0; j < matrix.rows(); j++ )
    {
        for ( unsigned int k = 0; k < matrix.cols(); k++ )
            ofs << matrix.at(j,k).flAbsCurrent << '\t';
        ofs << '\n';
    }

    ofs.close();
    if ( ofs.fail() )
        return false;

    return true;
}

//const char cszCSVHeader[] = "Object,Point #,X,Y,Z,Dwell Time (s),Specimen Current (nA),Spec 1 Counts,Spec 2 Counts,Spec 3 Counts,Spec 4 Counts\n";
bool UtilThread::InternalExportCSV( void )
{
    return false;
    /*std::ofstream ofs( m_strFilename.toAscii(), std::ofstream::trunc );
    if ( !ofs.good() )
        return false;

    //CSV Header
    ofs << cszCSVHeader;

    for ( int i = 0; i < pRenderables->count(); i++ )
    {
        const Matrix2d<point_s>& matrix = pRenderables->at(i)->GetPointMatrix();
        for ( unsigned int j = 0; j < matrix.count(); j++ )
        {
            //Messy
            if ( matrix.idx(j).bCollected )
            {
                ofs << pRenderables->at(i)->m_strDisplayName.toStdString() << ',' << j+1
                    << ',' << matrix.idx(j).flPos[0] << ',' << matrix.idx(j).flPos[1] << ',' << matrix.idx(j).flPos[2] << ','
                    << m_flDwellTime << ',' << matrix.idx(j).flAbsCurrent
                    << ',' << matrix.idx(j).flSpecCounts[0] << ',' << matrix.idx(j).flSpecCounts[1]
                    << ',' << matrix.idx(j).flSpecCounts[2] << ',' << matrix.idx(j).flSpecCounts[3]
                    << '\n';

                //The old format conforming to the TN-5500 text output
                //ofs << "0" << ',' << matrix.idx(j).flPos[0] << ',' << matrix.idx(j).flPos[1] << ',' << matrix.idx(j).flPos[2] << ','
                //    << matrix.idx(j).flAbsCurrent << ','
                //    << matrix.idx(j).flSpecCounts[0] << ',' << matrix.idx(j).flSpecCounts[0] / pRenderables->at(i)->DwellTime()
                //        << ',' << (matrix.idx(j).flSpecCounts[0] / pRenderables->at(i)->DwellTime()) / matrix.idx(j).flAbsCurrent << ','
                //    << matrix.idx(j).flSpecCounts[1] << ',' << matrix.idx(j).flSpecCounts[1] / pRenderables->at(i)->DwellTime()
                //        << ',' << (matrix.idx(j).flSpecCounts[1] / pRenderables->at(i)->DwellTime()) / matrix.idx(j).flAbsCurrent << ','
                //    << matrix.idx(j).flSpecCounts[2] << ',' << matrix.idx(j).flSpecCounts[2] / pRenderables->at(i)->DwellTime()
                //        << ',' << (matrix.idx(j).flSpecCounts[2] / pRenderables->at(i)->DwellTime()) / matrix.idx(j).flAbsCurrent << ','
                //    << matrix.idx(j).flSpecCounts[3] << ',' << matrix.idx(j).flSpecCounts[3] / pRenderables->at(i)->DwellTime()
                //        << ',' << (matrix.idx(j).flSpecCounts[3] / pRenderables->at(i)->DwellTime()) / matrix.idx(j).flAbsCurrent << ','
                //    << '\n';
            }
        }
    }

    ofs.close();

    return !ofs.fail();*/
}
