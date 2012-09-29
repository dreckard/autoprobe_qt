/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#include "PointData.h"
#include "mainwindow.h"

//-------------------------------------------------------------------------
// SerializedPoints
//-------------------------------------------------------------------------
SerializedPoints::SerializedPoints()
{
    flCountTime = -1.0f;
    iNumPts = 0;
    points = NULL;
}

SerializedPoints::SerializedPoints( unsigned int num_pts )
{
    flCountTime = -1.0f;
    points = new point_s[num_pts];
    iNumPts = num_pts;
    for ( unsigned int i = 0; i < num_pts; i++ )
    {
        points[i].flAbsCurrent = -1.0f;
        for ( int j = 0; j < 3; j++ )
            points[i].flPos[j] = -1.0f;
            //points[i].flPos[j] = float(num_pts)-float(i);

        for ( int j = 0; j < 4; j++ )
            points[i].flSpecCounts[j] = -1.0f;

        points[i].bCollected = false;
    }
}

SerializedPoints::~SerializedPoints()
{
    delete[] points;
}

//-------------------------------------------------------------------------
// PointsTableModel
//-------------------------------------------------------------------------
int PointsTableModel::rowCount(const QModelIndex &parent) const
{
    return MainWindow::Instance().GetTotalPoints();
}

void PointsTableModel::ClearPointByOverallIdx( unsigned int idx )
{
    point_s* pt = PointByOverallIdx( idx );
    for ( unsigned int i = 0; i < NUM_SPECTROMETERS; i++ )
        pt->flSpecCounts[i] = 0.0f;
    pt->flAbsCurrent = 0.0f;
    pt->bCollected = false;
    MarkPointDirty( idx );
}

void PointsTableModel::MarkPointDirty( unsigned int iPoint )
{
    emit dataChanged( index(iPoint,0), index(iPoint,NUM_POINT_COLS) );
}

const AutoProbeRenderable* PointsTableModel::RenderableByOverallIdx( unsigned int idx ) const
{
    const QList<AutoProbeRenderable*>* pRenderables = MainWindow::Instance().GetRenderables();
    unsigned int iTally = 0;
    for ( int i = 0; i < pRenderables->count(); i++ )
    {
        iTally += pRenderables->at(i)->NumPoints();
        if ( iTally > idx )
            return pRenderables->at(i);
    }
    return NULL;
}

const point_s* PointsTableModel::PointByOverallIdx( unsigned int idx ) const
{
    const QList<AutoProbeRenderable*>* pRenderables = MainWindow::Instance().GetRenderables();
    unsigned int iTally = 0;
    for ( int i = 0; i < pRenderables->count(); i++ )
    {
        if ( iTally + pRenderables->at(i)->NumPoints() > idx && pRenderables->at(i)->GetPointMatrix().count() > idx-iTally )
            return &pRenderables->at(i)->GetPointMatrix().idx(idx-iTally);
        else
            iTally += pRenderables->at(i)->NumPoints();
    }
    return NULL;
    //return PointByOverallIdx( idx );
}
point_s* PointsTableModel::PointByOverallIdx( unsigned int idx )
{
    const QList<AutoProbeRenderable*>* pRenderables = MainWindow::Instance().GetRenderables();
    unsigned int iTally = 0;
    for ( int i = 0; i < pRenderables->count(); i++ )
    {
        if ( iTally + pRenderables->at(i)->NumPoints() > idx && pRenderables->at(i)->GetPointMatrix().count() > idx-iTally )
            return &pRenderables->at(i)->GetPointMatrix().idx(idx-iTally);
        else
            iTally += pRenderables->at(i)->NumPoints();
    }
    return NULL;
}

QVariant PointsTableModel::data(const QModelIndex &index, int role) const
{
    if ( role == Qt::DisplayRole )
    {
        if ( index.column() < 0 || index.column() > NUM_POINT_COLS )
            return QVariant();

        if ( index.column() == 0 )
            return index.row()+1;

        const AutoProbeRenderable* pRenderable = RenderableByOverallIdx( (unsigned int)index.row() );
        const point_s* pPoint = PointByOverallIdx( (unsigned int)index.row() );
        if ( !pRenderable || !pPoint )
            return QVariant();

        if ( index.column() > 0 && index.column() < 4 )
            return pPoint->flPos[index.column()-1];
        else if ( index.column() == 4 )
            return pPoint->flAbsCurrent;
        else if ( index.column() < 9 )
            return pPoint->flSpecCounts[index.column()-5];
        else if ( index.column() == 9 )
            return pPoint->bCollected;
        else if ( index.column() == 10 )
            return pRenderable->m_strDisplayName;
    }
    return QVariant();
}

QVariant PointsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    {
        switch ( section )
        {
        case 0:
            return "Point";
        case 1:
            return "X";
        case 2:
            return "Y";
        case 3:
            return "Z";
        case 4:
            return "Current (nA)";
        case 5:
            return "Spec 1 Counts";
        case 6:
            return "Spec 2 Counts";
        case 7:
            return "Spec 3 Counts";
        case 8:
            return "Spec 4 Counts";
        case 9:
            return "Collected";
        case 10:
            return "Associated Object";
        default:
            return QVariant();
        }
    }
    return QVariant();
}
