/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#include <cstdio>
#include <fstream>
#include <sstream>
#include <limits>
#include <QComboBox>
#include <QPainter>
#include <QMessageBox>
#include <QHeaderView>
#include "AutoProbe_Objects.h"
#include "AutoProbeError.h"
#include "mainwindow.h"
#include "Utility.h"
#include "../clib_superprobe733/clib_superprobe733.h"

//-------------------------------------------------------------------------
// ObjectsListModel
//-------------------------------------------------------------------------
int ObjectsListModel::rowCount( const QModelIndex &parent ) const
{
    const QList<AutoProbeRenderable*>* renderables = MainWindow::Instance().GetRenderables();
    if ( renderables )
        return renderables->count();
    return 0;
}

QVariant ObjectsListModel::data( const QModelIndex &index, int role ) const
{
    const QList<AutoProbeRenderable*>* renderables = MainWindow::Instance().GetRenderables();
    if ( role == Qt::DisplayRole || role == Qt::EditRole )
    {
        if ( renderables && index.row() < renderables->count() )
            return renderables->at(index.row())->m_strDisplayName;
    }
    return QVariant();
}

bool ObjectsListModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if ( role == Qt::EditRole )
    {
        const QList<AutoProbeRenderable*>* renderables = MainWindow::Instance().GetRenderables();
        if ( renderables && index.row() < renderables->count() )
        {
            renderables->at(index.row())->m_strDisplayName = value.toString();
            MainWindow::Instance().resetTableView();
            emit dataChanged( index, index );
            return true;
        }
    }
    return false;
}

//-------------------------------------------------------------------------
// Utility functions
//-------------------------------------------------------------------------
/*Float2 ScreenToStage( Int2 ptScr, const RenderInfo& renderInfo )
{
    Float2 rptStg;

    //Can has proportion?
    //rptStg = Float2( (renderInfo.flStageLimX*ptScr.X)/renderInfo.iWidth, (renderInfo.flStageLimY*ptScr.Y)/renderInfo.iHeight );

    const RenderBounds& bounds = renderInfo.renderBounds;
    rptStg = Float2( bounds.flX[0]+((renderInfo.BoundedWidth()*ptScr.X)/renderInfo.iWidth),
                     bounds.flY[0]+((renderInfo.BoundedHeight()*ptScr.Y)/renderInfo.iHeight) );

    return rptStg;
}*/
/*Int2 StageToScreen( Float2 rptStg, const RenderInfo& renderInfo )
{
    Int2 ptScr;

    const RenderBounds& bounds = renderInfo.renderBounds;

    //ptScr = Int2( int((rptStg.X*renderInfo.iWidth)/renderInfo.flStageLimX), int((rptStg.Y*renderInfo.iHeight)/renderInfo.flStageLimY ));
    ptScr = Int2( int(((rptStg.X-bounds.flX[0])*renderInfo.iWidth)/renderInfo.BoundedWidth()),
                  int(((rptStg.Y-bounds.flY[0])*renderInfo.iHeight)/renderInfo.BoundedHeight()) );

    return ptScr;
}
bool StagePointVisible( Float2 ptStg, const RenderInfo& renderInfo )
{
    return ( ptStg.X >= renderInfo.renderBounds.flX[0] && ptStg.X <= renderInfo.renderBounds.flX[1] &&
             ptStg.Y >= renderInfo.renderBounds.flY[0] && ptStg.Y <= renderInfo.renderBounds.flY[1] );
}*/

//-------------------------------------------------------------------------
// AutoProbeRenderable
//-------------------------------------------------------------------------
void AutoProbeRenderable::ClearPointData( void )
{
    for ( unsigned int i = 0; i < m_PointMatrix.count(); i++ )
    {
        m_PointMatrix.idx(i).flAbsCurrent = 0.0f;
        m_PointMatrix.idx(i).flSpecCounts[0] = 0.0f;
        m_PointMatrix.idx(i).flSpecCounts[1] = 0.0f;
        m_PointMatrix.idx(i).flSpecCounts[2] = 0.0f;
        m_PointMatrix.idx(i).flSpecCounts[3] = 0.0f;
        m_PointMatrix.idx(i).bCollected = false;
    }
    BlankImages();
}

bool AutoProbeRenderable::ReadFromFile( std::ifstream &ifs, char cVer )
{
    unsigned short iNameLen = 0;
    ifs.read( (char*)&iNameLen, sizeof(iNameLen) );
    m_strDisplayName.resize( iNameLen/sizeof(QChar) );
    ifs.read( (char*)m_strDisplayName.data(), iNameLen );
    ifs.read( (char*)(&m_flDwellTime), sizeof(m_flDwellTime) );

    //Read points table
    unsigned int iRows = 0;
    unsigned int iCols = 0;
    ifs.read( (char*)(&iRows), sizeof(iRows) );
    ifs.read( (char*)(&iCols), sizeof(iCols) );
    m_PointMatrix.clear();

    if ( iRows > 0 && iCols > 0 )
    {
        m_PointMatrix.setRows( iRows );
        m_PointMatrix.setCols( iCols );
        for ( unsigned int i = 0; i < iRows; i++ )
        {
            for ( unsigned int j = 0; j < iCols; j++ )
                ifs.read( (char*)(&(m_PointMatrix.at(i,j))), sizeof(m_PointMatrix.at(i,j)) );
        }
    }

    return !ifs.fail();
}
bool AutoProbeRenderable::WriteToFile( std::ofstream &ofs )
{
    //Write object type byte, name length, name
    char cType = (char)(ObjectType());
    ofs.write( &cType, sizeof(cType) );
    unsigned short iNameLen = m_strDisplayName.size()*sizeof(QChar);
    ofs.write( (char*)&iNameLen, sizeof(iNameLen) );
    ofs.write( (char*)m_strDisplayName.constData(), iNameLen );
    ofs.write( (char*)(&m_flDwellTime), sizeof(m_flDwellTime) );

    //Write points table
    const Matrix2d<point_s>& matrix = GetPointMatrix();
    unsigned int iRows = matrix.rows();
    unsigned int iCols = matrix.cols();
    ofs.write( (char*)(&iRows), sizeof(iRows) );
    ofs.write( (char*)(&iCols), sizeof(iCols) );
    for ( unsigned int i = 0; i < matrix.rows(); i++ )
    {
        for ( unsigned int j = 0; j < matrix.cols(); j++ )
            ofs.write( (char*)(&matrix.at(i,j)), sizeof(matrix.at(i,j)) );
    }

    return !ofs.fail();
}

void AutoProbeRenderable::RegenerateImages( void )
{
    m_Images.clear();
    for ( unsigned int i = 0; i < RENDERMODE_NUM_IMG; i++ )
        RegenerateImage( i );
}

void AutoProbeRenderable::BlankImages( void )
{
    for ( int i = 0; i < m_Images.count(); i++ )
        m_Images[i].fill( 0 );
    for ( unsigned int i = 0; i < RENDERMODE_NUM_IMG; i++ )
    {
        m_flMinValue[i] = 0.0f;
        m_flMaxValue[i] = 0.0f;
    }
}

void AutoProbeRenderable::SetMapBC( unsigned short usRenderMode, float flMin, float flMax )
{
    unsigned int idx = RenderModeToImageIdx(usRenderMode);
    if ( m_bAutoMin[idx] && m_bAutoMax[idx] )
        return;

    if ( !m_bAutoMin[idx] )
        m_flMinValue[idx] = flMin;
    if ( !m_bAutoMax[idx] )
        m_flMaxValue[idx] = flMax;

    RegenerateImage( idx );
}

void AutoProbeRenderable::SetMapAutoMin( unsigned short usRenderMode, bool bAuto )
{
    unsigned int idx = RenderModeToImageIdx(usRenderMode);
    if ( bAuto == m_bAutoMin[idx] )
        return;
    m_bAutoMin[idx] = bAuto;
    if ( bAuto )
        RegenerateImage(idx);
}
void AutoProbeRenderable::SetMapAutoMax( unsigned short usRenderMode, bool bAuto )
{
    unsigned int idx = RenderModeToImageIdx(usRenderMode);
    if ( bAuto == m_bAutoMax[idx] )
        return;
    m_bAutoMax[idx] = bAuto;
    if ( bAuto )
        RegenerateImage(idx);
}

bool AutoProbeRenderable::IsSelected( void )
{
    QList<AutoProbeRenderable*> list = MainWindow::Instance().GetSelectedRenderables();
    return list.contains( this );
}

//Convenience functions for PropertyTableUpdated
template<typename T>
void SetItemText( QTableWidgetItem* pItem, const T& value )
{
    pItem->setText( AsString( value ).c_str() );
}
bool AutoProbeRenderable::VerifyAndSetFlt( QTableWidgetItem* pItem, float* pVar, float flMin, float flMax )
{
    if ( !pItem )
        return false;
    float flItemVal = pItem->text().toFloat();
    if ( flItemVal < flMin || flItemVal > flMax )
    {
        SetItemText( pItem, *pVar );
        return false;
    }

    *pVar = flItemVal;
    return true;
}

float AutoProbeRenderable::GetMinCounts( unsigned char cSpec )
{
    float flMin = 0.0f;
    const Matrix2d<point_s>& matrix = GetPointMatrix();
    for ( unsigned int i = 0; i < matrix.count(); i++ )
    {
        const point_s& pt = matrix.idx(i);
        if ( pt.bCollected )
            flMin = rd_min( flMin, matrix.idx(i).flSpecCounts[cSpec] );
    }
    return flMin;
}
float AutoProbeRenderable::GetMinCurrent( void )
{
    float flMin = 0.0f;
    const Matrix2d<point_s>& matrix = GetPointMatrix();
    for ( unsigned int i = 0; i < matrix.count(); i++ )
    {
        const point_s& pt = matrix.idx(i);
        if ( pt.bCollected )
            flMin = rd_min( flMin, matrix.idx(i).flAbsCurrent );
    }
    return flMin;
}

//Convenience functions for image normalization
float AutoProbeRenderable::GetMaxCounts( unsigned char cSpec )
{
    float flMax = 0.0f;
    const Matrix2d<point_s>& matrix = GetPointMatrix();
    for ( unsigned int i = 0; i < matrix.count(); i++ )
    {
        const point_s& pt = matrix.idx(i);
        if ( pt.bCollected )
            flMax = rd_max( flMax, matrix.idx(i).flSpecCounts[cSpec] );
    }
    return flMax;
}
float AutoProbeRenderable::GetMaxCurrent( void )
{
    float flMax = 0.0f;
    const Matrix2d<point_s>& matrix = GetPointMatrix();
    for ( unsigned int i = 0; i < matrix.count(); i++ )
    {
        const point_s& pt = matrix.idx(i);
        if ( pt.bCollected )
            flMax = rd_max( flMax, matrix.idx(i).flAbsCurrent );
    }
    return flMax;
}

void AutoProbeRenderable::GetMinMaxCounts( unsigned char cSpec, float& flMin, float& flMax )
{
    const Matrix2d<point_s>& matrix = GetPointMatrix();
    if ( matrix.count() < 1 )
    {
        flMin = flMax = 0.0f;
        return;
    }

    flMin = flMax = matrix.idx(0).flSpecCounts[cSpec]; //Initialize to the first value then spread from there
    for ( unsigned int i = 1; i < matrix.count(); i++ )
    {
        const point_s& pt = matrix.idx(i);
        if ( pt.bCollected )
        {
            flMin = rd_min( flMin, matrix.idx(i).flSpecCounts[cSpec] );
            flMax = rd_max( flMax, matrix.idx(i).flSpecCounts[cSpec] );
        }
    }
}
void AutoProbeRenderable::GetMinMaxCurrent( float& flMin, float& flMax )
{
    const Matrix2d<point_s>& matrix = GetPointMatrix();
    if ( matrix.count() < 1 )
    {
        flMin = flMax = 0.0f;
        return;
    }

    flMin = flMax = matrix.idx(0).flAbsCurrent; //Initialize to the first value then spread from there
    for ( unsigned int i = 1; i < matrix.count(); i++ )
    {
        const point_s& pt = matrix.idx(i);
        if ( pt.bCollected )
        {
            flMin = rd_min( flMin, matrix.idx(i).flAbsCurrent );
            flMax = rd_max( flMax, matrix.idx(i).flAbsCurrent );
        }
    }
}

//-------------------------------------------------------------------------
// AutoProbePoint
//-------------------------------------------------------------------------
void AutoProbePoint::UpdatePointMatrix( bool bInvalidate )
{
    m_PointMatrix.clear();
    point_s pt = {};
    if ( !bInvalidate && m_PointMatrix.inRange(0,0) )
        pt = m_PointMatrix.at(0,0);
    else
        pt.bCollected = false;

    pt.flPos[0] = m_pt.X; pt.flPos[1] = m_pt.Y;
    pt.flPos[2] = MainWindow::Instance().GetPlane().FindIntersection(
            Line( Vec3( pt.flPos[0], pt.flPos[1], 0 ), Vec3(0,0,1) ) ).z;

    m_PointMatrix.setRows( 1 ); m_PointMatrix.setCols( 1 );
    m_PointMatrix.setValue( 0, 0, pt );
}

bool AutoProbePoint::GetNextLScan( unsigned int curRow, unsigned int curCol, LScan &lscan )
{
    if ( !m_PointMatrix.inRange( 0, 0 ) || m_PointMatrix.at(0,0).bCollected )
        return false;
    const point_s& pt = m_PointMatrix.at( 0, 0 );
    Float3 pos( pt.flPos[0], pt.flPos[1], pt.flPos[2] );
    LScan ret( pos, pos, 0, 0, DwellTime(), NumPoints() );
    lscan = ret;
    return true;
}

bool AutoProbePoint::DataReceived( const TN5500_Data &ptData, unsigned int& row, unsigned int& col )
{
    Matrix2d<point_s>& matrix = GetPointMatrix();
    if ( !matrix.inRange(0,0) )
        return false;

    row = col = 0;
    point_s& pt = matrix.at( 0, 0 );
    pt.flAbsCurrent = TN5500_Data_Current(ptData.sCurrent);
    pt.bCollected = true;
    for ( unsigned int i = 0; i < 4; i++ )
        pt.flSpecCounts[i] = TN5500_Data_Counts(ptData.sCounts[i]);
    return false;
}

bool AutoProbePoint::GetScreenSpaceCenter( Int2 &Center, const RenderInfo &renderInfo ) const
{
    Center = renderInfo.StageToScreen( m_pt );
    return true;
}

bool AutoProbePoint::GetClickableArea( Int2 &Mins, Int2 &Maxs, const RenderInfo &renderInfo ) const
{
    Int2 scr;
    if ( !GetScreenSpaceCenter( scr, renderInfo ) )
        return false;

    //Totally arbitrary choice of 10 px
    Mins.X = scr.X - 10;
    Mins.Y = scr.Y - 10;
    Maxs.X = scr.X + 10;
    Maxs.Y = scr.Y + 10;

    return true;
}

void AutoProbePoint::Paint( QPainter &painter, const RenderInfo &renderInfo )
{
    if ( MainWindow::Instance().GetRenderMode() == RENDERMODE_WIREFRAME )
    {
        Int2 scrPt;
        if ( GetScreenSpaceCenter( scrPt, renderInfo ) )
        {
            if ( IsSelected() )
            {
                painter.setPen( Qt::red );
                painter.setBrush( Qt::red );
            }
            else
            {
                painter.setPen( Qt::black );
                painter.setBrush( Qt::black );
            }
            painter.drawEllipse( scrPt.X-2, scrPt.Y-2, 4, 4 );
        }
    }
}

bool AutoProbePoint::IsVisible( const RenderInfo &renderInfo )
{
    //Int2 scrPt = StageToScreen( Float2( m_pt.X, m_pt.Y ), renderInfo );

    return renderInfo.StagePointVisible( m_pt );
    //return ( m_pt.X >= renderInfo.renderBounds.flX[0] && m_pt.X <= renderInfo.renderBounds.flX[1] &&
    //         m_pt.Y >= renderInfo.renderBounds.flY[0] && m_pt.Y <= renderInfo.renderBounds.flY[1] );
}

void AutoProbePoint::SetupPropertyTable( QTableWidget* pTableWidget )
{
    pTableWidget->setRowCount( 3 );
    QStringList strList;
    strList.append( "X" );
    pTableWidget->setItem( 0, 0, new QTableWidgetItem( AsString(m_pt.X).c_str() ) );
    strList.append( "Y" );
    pTableWidget->setItem( 1, 0, new QTableWidgetItem( AsString(m_pt.Y).c_str() ) );
    strList.append( "Dwell Time (s)" );
    pTableWidget->setItem( 2, 0, new QTableWidgetItem( AsString(m_flDwellTime).c_str() ) );

    pTableWidget->setVerticalHeaderLabels( strList );
}

void AutoProbePoint::PropertyTableUpdated( QTableWidget* pTableWidget )
{
    AutoProbeSettings& settings = MainWindow::Instance().m_Settings;

    VerifyAndSetFlt( pTableWidget->item(0,0), &m_pt.X, settings.m_flStageLimits[0][0], settings.m_flStageLimits[0][1] );
    VerifyAndSetFlt( pTableWidget->item(1,0), &m_pt.Y, settings.m_flStageLimits[1][0], settings.m_flStageLimits[1][1] );
    VerifyAndSetFlt( pTableWidget->item(2,0), &m_flDwellTime, 0.001f, 999999.0f );

    UpdatePointMatrix( !MainWindow::Instance().DevModeEnabled() );
}

//Save
bool AutoProbePoint::WriteToFile( std::ofstream& ofs )
{
    if ( !AutoProbeRenderable::WriteToFile( ofs ) )
        return false;

    ofs.write( (char*)(&m_pt), sizeof(m_pt) );

    return !ofs.fail();
}

//Load
bool AutoProbePoint::ReadFromFile( std::ifstream& ifs, char cVer )
{
    if ( !AutoProbeRenderable::ReadFromFile( ifs, cVer ) )
        return false;

    ifs.read( (char*)(&m_pt), sizeof(m_pt) );

    return true;
}

//-------------------------------------------------------------------------
// AutoProbeRect
//-------------------------------------------------------------------------
void AutoProbeRect::UpdatePointMatrix( bool bInvalidate )
{
    //Zero step-size won't work
    if ( m_flXStep < 0.001f || m_flYStep < 0.001f )
        return;

    //Having two copies of the matrix in memory simultaneously could be an issue with very large objects
    Matrix2d<point_s> newMatrix( Rows(), Columns() );
    for ( int i = 0; i < Rows(); i++ )
    {
        for ( int j = 0; j < Columns(); j++ )
        {
            point_s pt = {};
            pt.flPos[0] = m_pt1.X + j*m_flXStep; pt.flPos[1] = m_pt1.Y + i*m_flYStep;
            pt.flPos[2] = MainWindow::Instance().GetPlane().FindIntersection(
                    Line( Vec3( pt.flPos[0], pt.flPos[1], 0 ), Vec3(0,0,1) ) ).z;

            if ( !bInvalidate )
            {
                //Try to figure out where each data point should go
                for ( unsigned int k = 0; k < m_PointMatrix.rows(); k++ )
                {
                    if ( fltEq( pt.flPos[1], m_PointMatrix.at(k,0).flPos[1], m_flYStep / 4.0f ) )
                    {
                        for ( unsigned int l = 0; l < m_PointMatrix.cols(); l++ )
                        {
                            if ( fltEq( pt.flPos[0], m_PointMatrix.at(k,l).flPos[0], m_flXStep / 4.0f ) )
                            {
                                point_s& oldPt = m_PointMatrix.at(k,l);
                                pt.flAbsCurrent = oldPt.flAbsCurrent;
                                pt.bCollected = oldPt.bCollected;
                                for ( unsigned int m = 0; m < NUM_SPECTROMETERS; m++ )
                                    pt.flSpecCounts[m] = oldPt.flSpecCounts[m];
                                break;
                            }
                        }
                        break;
                    }
                }
            }

            newMatrix.setValue( i, j, pt );
        }
    }
    m_PointMatrix = newMatrix;
}

//Return: true if point is found with specified bCollected
//        false if no match is found
//Output: row, col - matching point coords, will be written to even if the function returns false
bool AutoProbeRect::FindNextPoint( unsigned int curRow, unsigned int curCol, bool bCollected, unsigned int& row, unsigned int& col, bool bReverse /*= false*/ ) const
{
    const Matrix2d<point_s>& matrix = GetPointMatrix();
    row = curRow; col = curCol;
    do
    {
        if ( matrix.at(row,col).bCollected == bCollected )
            return true;
    }
    while ( StepRowCol( row, col, false ) );
    return false;
}

//Return: true if point is found on line with specified bCollected
//        false if EOL is reached without finding a matching pt
//Output: row, col - point coords, will be EOL if function returns false
bool AutoProbeRect::FindNextPointOnLine( unsigned int curRow, unsigned int curCol, bool bCollected, unsigned int& row, unsigned int& col ) const
{
    const Matrix2d<point_s>& matrix = GetPointMatrix();
    row = curRow; col = curCol;
    do
    {
        if ( matrix.at(row,col).bCollected == bCollected )
            return true;
    }
    while ( StepRowCol( row, col, true ) );
    return false;
}

//Stupid name, checks to see if two points share the same secondary axis coord
bool AutoProbeRect::Collinear( unsigned int r1, unsigned int c1, unsigned int r2, unsigned int c2 ) const
{
    enum ScanType scanType = EffectiveScanType(r1,c1);
    if ( scanType == LR || scanType == RL )
        return r1 == r2;
    else
        return c1 == c2;
}

bool AutoProbeRect::FindFirstPointOnLine( unsigned int curRow, unsigned int curCol, bool bCollected, unsigned int& row, unsigned int& col ) const
{
    const Matrix2d<point_s>& matrix = GetPointMatrix();
    enum ScanType scanType = EffectiveScanType(curRow,curCol);
    switch ( scanType )
    {
    case LR:
        curCol = 0;
        break;
    case RL:
        curCol = matrix.cols()-1;
        break;
    case TB:
        curRow = 0;
        break;
    case BT:
        curRow = matrix.rows()-1;
        break;
    default:
        break;
    }
    return FindNextPointOnLine( curRow, curCol, bCollected, row, col );
}

//TODO: Move these somewhere else?
template <typename T>
void Inc2D( T& a, T& b, unsigned int numa ) //Iterate through 2d array
{
    if ( ++a >= numa )
    {
        a = 0;
        b++;
    }
}
template <typename T>
void Dec2D( T& a, T& b, unsigned int numa ) //R-Iterate through 2d array
{
    if ( a <= 0 )
    {
        a = numa-1;
        b++;
    }
    else
        a--;
}
template <typename T>
void Snake2D( T& a, T& b, unsigned int numa )
{
    int dir = (b % 2 == 0) ? 1 : -1;
    if ( a + dir >= 0 && a + dir < numa )
        a += dir;
    else
        b++;
}
template <typename T>
void RSnake2D( T& a, T& b, unsigned int numa )
{
    int dir = (b % 2 != 0) ? 1 : -1;
    if ( a + dir >= 0 && a + dir < numa )
        a += dir;
    else
        b++;
}

//Input: row, col - variables to step
//       bLineOnly - if true, will not increment past end of current line
//Returns true if next point is in bounds and satisfies bLineOnly
bool AutoProbeRect::StepRowCol( unsigned int &row, unsigned int &col, bool bLineOnly, bool bReverse /*=false*/ ) const
{
    const Matrix2d<point_s>& matrix = GetPointMatrix();
    enum ScanType scanType = EffectiveScanType(row,col);
    unsigned int rrow = row, rcol = col;
    switch ( scanType )
    {
    case LR:
    case RL:
        if ( bReverse ) RSnake2D( rcol, rrow, matrix.cols() );
        else Snake2D( rcol, rrow, matrix.cols() );
        break;
    case TB:
    case BT:
        if ( bReverse ) RSnake2D( rrow, rcol, matrix.rows() );
        else Snake2D( rrow, rcol, matrix.rows() );
        break;
    default:
        ap_error( "Got EffectiveScanType()=AUTO!" );
        return false;
    }

    if ( !matrix.inRange( rrow, rcol )  )
        return false;

    if ( bLineOnly )
    {
        if ( (scanType == LR || scanType == RL) && rrow != row )
            return false;
        if ( (scanType == TB || scanType == BT) && rcol != col )
            return false;
    }

    row = rrow;
    col = rcol;
    return true;
}

bool AutoProbeRect::GetNextLScan( unsigned int curRow, unsigned int curCol, LScan &lscan )
{
    unsigned int r1,c1,r2,c2;
    int np = 0;
    const Matrix2d<point_s>& matrix = GetPointMatrix();

    if ( FindNextPoint( curRow, curCol, false, r1, c1 ) )
    {
        point_s p1 = matrix.at(r1,c1);
        point_s p2 = {};

        //Iterate to next collected pt or EOL
        if ( FindNextPointOnLine( r1, c1, true, r2, c2 ) )
            StepRowCol( r2, c2, true, true ); //Step back one point if the endpt is collected

        p2 = matrix.at(r2,c2);
        np = (r1 == r2) ? abs(int(c2)-int(c1)) : abs(int(r2)-int(r1));
        np++;

        //Look at the two end points and decide which to start at based on dist from last pt
        /*const point_s& lastPt = matrix.at(curRow,curCol);
        Vec3 vec1( p1.flPos[0], p1.flPos[1], p1.flPos[2] ); Vec3 vec2( p2.flPos[0], p2.flPos[1], p2.flPos[2] );
        Vec3 vecLast( lastPt.flPos[0], lastPt.flPos[1], lastPt.flPos[2] );
        if ( vec1.Dist( vecLast ) > vec1.Dist( vecLast ) )
            swap( p1,p2 );*/

        LScan ret( Float3( p1.flPos[0], p1.flPos[1], p1.flPos[2] ), Float3( p2.flPos[0], p2.flPos[1], p2.flPos[2] ), r1, c1, DwellTime(), np );
        lscan = ret;
        m_iLastRow = r1; m_iLastCol = c1; //Prepare cache for FindNextPoint use
        return true;
    }
    return false;
}

//Returns false when this object is entirely collected
//Output: row, col - dst coords for this data
//Return: true on success, false on failure to find a place to put the data (should never happen)
bool AutoProbeRect::DataReceived( const TN5500_Data &ptData, unsigned int& row, unsigned int& col )
{
    Matrix2d<point_s>& matrix = GetPointMatrix();

    if ( !FindNextPoint( m_iLastRow, m_iLastCol, false, row, col ) )
        return false;

    //Update cache
    m_iLastRow = row;
    m_iLastCol = col;

    point_s& pt = matrix.at( row, col );
    pt.flAbsCurrent = TN5500_Data_Current(ptData.sCurrent);
    pt.bCollected = true;
    for ( unsigned int i = 0; i < 4; i++ )
        pt.flSpecCounts[i] = TN5500_Data_Counts(ptData.sCounts[i]);

    UpdateImages( row, col );

    return true;
}

//This could use some optimization and refactoring
void AutoProbeRect::RegenerateImage( unsigned int idx )
{
    //Not the safest way to set this up, will cause crash if images aren't first generated in idx order (0,1,2,3...)
    AP_RLSASSERT( idx <= (unsigned int)(m_Images.size()) );
    //if ( (unsigned int)(m_Images.size()) < idx )
        //ap_fatalerror("AutoProbeRect::RegenerateImage() failed! m_Images.size() >= idx");

    const Matrix2d<point_s>& matrix = GetPointMatrix();
    QImage image( Columns(), Rows(), QImage::Format_Indexed8 );
    image.setColorTable( m_colorTable );
    image.fill( 0 );

    //Simple normalization to max value
    if ( idx < NUM_SPECTROMETERS )
    {
        if ( m_bAutoMin[idx] || m_bAutoMax[idx] )
        {
            float flMin, flMax;
            GetMinMaxCounts( idx, flMin, flMax );

            if ( m_bAutoMin[idx] )
                m_flMinValue[idx] = flMin;
            if ( m_bAutoMax[idx] )
                m_flMaxValue[idx] = flMax;
        }
        float flContrast = m_flMaxValue[idx]-m_flMinValue[idx];
        if ( flContrast > 0.01f ) //Can't div by zero
        {
            for ( int j = 0; j < Rows(); j++ )
            {
                for ( int k = 0; k < Columns(); k++ )
                    image.scanLine(j)[k] = (uchar)(rd_bound(((matrix.at(j,k).flSpecCounts[idx]-m_flMinValue[idx])*(255.0f/flContrast)), 0.0f, 255.0f));
            }
        }
    }
    else //Current
    {
        if ( m_bAutoMin[idx] || m_bAutoMax[idx] )
        {
            float flMin, flMax;
            GetMinMaxCurrent( flMin, flMax );

            if ( m_bAutoMin[idx] )
                m_flMinValue[idx] = flMin;
            if ( m_bAutoMax[idx] )
                m_flMaxValue[idx] = flMax;
        }
        float flContrast = m_flMaxValue[idx]-m_flMinValue[idx];
        if ( flContrast > 0.01f ) //Can't div by zero
        {
            for ( int j = 0; j < Rows(); j++ )
            {
                for ( int k = 0; k < Columns(); k++ )
                    image.scanLine(j)[k] = (uchar)(rd_bound(((matrix.at(j,k).flAbsCurrent-m_flMinValue[idx])*(255.0f/flContrast)), 0.0f, 255.0f));
            }
        }
    }
    if ( idx < (unsigned int)(m_Images.size()) )
        m_Images.replace( idx, image );
    else
        m_Images.push_back( image );
}

//Update a particular point on all images
void AutoProbeRect::UpdateImages( unsigned int iRow, unsigned int iCol )
{
    if ( (unsigned int)(m_Images.count()) < RENDERMODE_NUM_IMG )
        return;

    const Matrix2d<point_s>& matrix = GetPointMatrix();
    for ( unsigned int i = 0; i < RENDERMODE_NUM_IMG; i++ )
    {
        //Cache the max value instead of calculating it every time; makes a huge performance difference for large objects
        QImage& image = m_Images[i];
        float flVal;
        if ( i < NUM_SPECTROMETERS )
            flVal = matrix.at(iRow,iCol).flSpecCounts[i];
        else
            flVal = matrix.at(iRow,iCol).flAbsCurrent;

        //Old way - was causing redraws even with auto min/max disabled
        /*if ( flVal > m_flMaxValue[i] || flVal < m_flMinValue[i] )
        {
            m_flMaxValue[i] = rd_max(m_flMaxValue[i],flVal);
            m_flMinValue[i] = rd_min(m_flMinValue[i],flVal);
            RegenerateImage( i );
        }
        else
        {
            float flContrast = m_flMaxValue[i]-m_flMinValue[i];
            if ( flContrast > 0.01f )
                image.scanLine(iRow)[iCol] = (uchar)(rd_bound(((flVal-m_flMinValue[i])*(255.0f/flContrast)), 0.0f, 255.0f));
        }*/

        //Check auto min/max
        bool bRegen = false;
        if ( m_bAutoMin[i] && flVal < m_flMinValue[i] )
        {
            m_flMinValue[i] = flVal;
            bRegen = true;
        }
        if ( m_bAutoMax[i] && flVal > m_flMaxValue[i] )
        {
            m_flMaxValue[i] = flVal;
            bRegen = true;
        }
        if ( bRegen )
            RegenerateImage( i ); //Regen the whole thing
        else
        {
            //Just insert the next pixel value
            float flContrast = m_flMaxValue[i]-m_flMinValue[i];
            if ( flContrast > 0.01f )
                image.scanLine(iRow)[iCol] = (uchar)(rd_bound(((flVal-m_flMinValue[i])*(255.0f/flContrast)), 0.0f, 255.0f));
        }
    }
}

bool AutoProbeRect::GetClickableArea( Int2& Mins, Int2& Maxs, const RenderInfo& renderInfo ) const
{
    Mins = renderInfo.StageToScreen( m_pt1 );
    Maxs = renderInfo.StageToScreen( m_pt2 );

    //WHATIF: This rect isn't on screen?
    return true;
}
bool AutoProbeRect::GetScreenSpaceCenter( Int2 &Center, const RenderInfo& renderInfo ) const
{
    Center = renderInfo.StageToScreen( Float2( m_pt2.X - m_pt1.X, m_pt2.Y - m_pt1.Y ) );

    //WHATIF: This rect isn't on screen?
    return true;
}

void AutoProbeRect::Paint( QPainter& painter, const RenderInfo& renderInfo )
{
    if ( MainWindow::Instance().GetRenderMode() == RENDERMODE_WIREFRAME )
    {
        Int2 scrPt1 = renderInfo.StageToScreen( m_pt1 );
        Int2 scrPt2 = renderInfo.StageToScreen( m_pt2 );

        painter.setBrush( Qt::NoBrush );
        if ( IsSelected() )
            painter.setPen( Qt::red );
        else
            painter.setPen( Qt::black );

        painter.drawRect( QRect( QPoint( scrPt1.X, scrPt1.Y ), QPoint( scrPt2.X, scrPt2.Y ) ) );

        float flScrStep = Height_SS(renderInfo)/4.0f;
        for ( int i = 1; i <= 3; i++ )
        {
            painter.drawLine( scrPt1.X, (scrPt1.Y+flScrStep*i)+0.5f, scrPt2.X, (scrPt1.Y+flScrStep*i)+0.5f );
            //painter.drawLine( ((scrPt1.X+scrPt2.X)/2)-5, ((scrPt1.Y+scrPt2.Y)/2)-5, (scrPt1.X+scrPt2.X)/2, (scrPt1.Y+scrPt2.Y)/2 );
            //painter.drawLine( ((scrPt1.X+scrPt2.X)/2)-5, ((scrPt1.Y+scrPt2.Y)/2)+5, (scrPt1.X+scrPt2.X)/2, (scrPt1.Y+scrPt2.Y)/2 );
        }
    }
    else if ( MainWindow::Instance().GetRenderMode() >= RENDERMODE_FIRST_IMG &&
              MainWindow::Instance().GetRenderMode() <= RENDERMODE_LAST_IMG )
    {

        Int2 scrPt1 = renderInfo.StageToScreen( m_pt1 );
        Int2 scrPt2 = renderInfo.StageToScreen( m_pt2 );

        //Bilinear filtering
        painter.drawImage( scrPt1.X, scrPt1.Y,
                           GetImage( MainWindow::Instance().GetRenderMode() ).scaled
                           ( Width_SS(renderInfo), Height_SS(renderInfo), Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
        //painter.drawImage( scrPt1.X, scrPt1.Y, GetImage( MainWindow::Instance().GetRenderMode() ) );
    }

}

bool AutoProbeRect::IsVisible( const RenderInfo &renderInfo )
{
    //Simple AABB test
    if ( renderInfo.renderBounds.flY[0] > m_pt2.Y ) return false; //top1>bot2
    if ( m_pt1.Y > renderInfo.renderBounds.flY[1] ) return false; //top2>bot1
    if ( renderInfo.renderBounds.flX[0] > m_pt2.X ) return false; //left1>right2
    if ( m_pt1.X > renderInfo.renderBounds.flX[1] ) return false; //left2>right1

    return true;
}

void AutoProbeRect::SetupPropertyTable( QTableWidget *pTableWidget )
{
    pTableWidget->setRowCount( 11 );
    QStringList strList;
    strList.append( "X1" );
    pTableWidget->setItem( 0, 0, new QTableWidgetItem( AsString(m_pt1.X).c_str() ) );
    strList.append( "Y1" );
    pTableWidget->setItem( 1, 0, new QTableWidgetItem( AsString(m_pt1.Y).c_str() ) );
    strList.append( "X2" );
    pTableWidget->setItem( 2, 0, new QTableWidgetItem( AsString(m_pt2.X).c_str() ) );
    strList.append( "Y2" );
    pTableWidget->setItem( 3, 0, new QTableWidgetItem( AsString(m_pt2.Y).c_str() ) );
    strList.append( "X Step" );
    pTableWidget->setItem( 4, 0, new QTableWidgetItem( AsString(m_flXStep).c_str() ) );
    strList.append( "Y Step" );
    pTableWidget->setItem( 5, 0, new QTableWidgetItem( AsString(m_flYStep).c_str() ) );
    strList.append( "Dwell Time (s)" );
    pTableWidget->setItem( 6, 0, new QTableWidgetItem( AsString(m_flDwellTime).c_str() ) );

    strList.append( "Scan Type" );
    QComboBox* pCombo = new QComboBox( pTableWidget );
    pCombo->addItem( "Auto" );
    pCombo->addItem( "Horizontal" );
    pCombo->addItem( "Vertical" );
    pCombo->setCurrentIndex( 0 );
    if ( m_scanType == LR || m_scanType == RL )
        pCombo->setCurrentIndex( 1 );
    else if ( m_scanType == TB || m_scanType == BT )
        pCombo->setCurrentIndex( 2 );
    QObject::connect( pCombo, SIGNAL(currentIndexChanged(int)), &MainWindow::Instance(), SLOT(ProbeObj_Property_Updated()) );
    pTableWidget->setCellWidget( 7, 0, pCombo );

    strList.append( "Rows" );
    QTableWidgetItem* pPts = new QTableWidgetItem( AsString(Rows()).c_str() );
    pPts->setFlags( Qt::ItemIsSelectable );
    pPts->setBackgroundColor( Qt::lightGray );
    pTableWidget->setItem( 8, 0, pPts );

    strList.append( "Columns" );
    pPts = new QTableWidgetItem( AsString(Columns()).c_str() );
    pPts->setFlags( Qt::ItemIsSelectable );
    pPts->setBackgroundColor( Qt::lightGray );
    pTableWidget->setItem( 9, 0, pPts );

    strList.append( "Points" );
    pPts = new QTableWidgetItem( AsString(NumPoints()).c_str() );
    pPts->setFlags( Qt::ItemIsSelectable );
    pPts->setBackgroundColor( Qt::lightGray );
    pTableWidget->setItem( 10, 0, pPts );

    pTableWidget->setVerticalHeaderLabels( strList );
}

void AutoProbeRect::PropertyTableUpdated( QTableWidget *pTableWidget )
{
    AutoProbeSettings& settings = MainWindow::Instance().m_Settings;

    bool bInvalidateMatrix = false;

    //Gotta save these off temporarily
    float pt1[2] = { m_pt1.X, m_pt1.Y };
    float pt2[2] = { m_pt2.X, m_pt2.Y };
    float step[2] = { m_flXStep, m_flYStep };
    float dwell = m_flDwellTime;

    VerifyAndSetFlt( pTableWidget->item(0,0), &m_pt1.X, settings.m_flStageLimits[0][0], settings.m_flStageLimits[0][1] );
    VerifyAndSetFlt( pTableWidget->item(1,0), &m_pt1.Y, settings.m_flStageLimits[1][0], settings.m_flStageLimits[1][1] );
    VerifyAndSetFlt( pTableWidget->item(2,0), &m_pt2.X, settings.m_flStageLimits[0][0], settings.m_flStageLimits[0][1] );
    VerifyAndSetFlt( pTableWidget->item(3,0), &m_pt2.Y, settings.m_flStageLimits[1][0], settings.m_flStageLimits[1][1] );
    VerifyAndSetFlt( pTableWidget->item(4,0), &m_flXStep, 0.001f, settings.m_flStageLimits[0][1] );
    VerifyAndSetFlt( pTableWidget->item(5,0), &m_flYStep, 0.001f, settings.m_flStageLimits[1][1] );
    VerifyAndSetFlt( pTableWidget->item(6,0), &m_flDwellTime, 0.001f, 3000.0f );

    bInvalidateMatrix = !fltEq( m_flDwellTime, dwell, 0.001 ); //Gotta clear the points if dwell is changed

    if ( MainWindow::Instance().GetTotalPoints() > MainWindow::Instance().m_Settings.m_iMaxPts )
    {
        m_pt1.X = pt1[0]; m_pt1.Y = pt1[1];
        m_pt2.X = pt2[0]; m_pt2.Y = pt2[1];
        m_flXStep = step[0]; m_flYStep = step[1];

        SetItemText( pTableWidget->item(0,0), pt1[0] );
        SetItemText( pTableWidget->item(1,0), pt1[1] );
        SetItemText( pTableWidget->item(2,0), pt2[0] );
        SetItemText( pTableWidget->item(3,0), pt2[1] );
        SetItemText( pTableWidget->item(4,0), step[0] );
        SetItemText( pTableWidget->item(5,0), step[1] );

        ap_error( "Max points exceeded" );
        return;
    }

    if ( pTableWidget->cellWidget(7,0) )
    {
        QComboBox* pCombo = static_cast<QComboBox*>(pTableWidget->cellWidget(7, 0));
        switch ( pCombo->currentIndex() )
        {
            case 1:
                m_scanType = LR;
                break;
            case 2:
                m_scanType = TB;
                break;
            default:
                m_scanType = AUTO;
        }
    }

    if ( pTableWidget->item( 8, 0 ) )
    {
        char szVal[16];
        snprintf( szVal, 16, "%i", Rows() );
        pTableWidget->item( 8, 0 )->setText( szVal );
    }
    if ( pTableWidget->item( 9, 0 ) )
    {
        char szVal[16];
        snprintf( szVal, 16, "%i", Columns() );
        pTableWidget->item( 9, 0 )->setText( szVal );
    }
    if ( pTableWidget->item( 10, 0 ) )
    {
        char szVal[16];
        snprintf( szVal, 16, "%i", NumPoints() );
        pTableWidget->item( 10, 0 )->setText( szVal );
    }

    UpdatePointMatrix( bInvalidateMatrix && !MainWindow::Instance().DevModeEnabled() );
    RegenerateImages();
}

//Save
bool AutoProbeRect::WriteToFile( std::ofstream& ofs )
{
    if ( !AutoProbeRenderable::WriteToFile( ofs ) )
        return false;

    ofs.write( (char*)(&m_pt1), sizeof(m_pt1) );
    ofs.write( (char*)(&m_pt2), sizeof(m_pt2) );
    ofs.write( (char*)(&m_flXStep), sizeof(m_flXStep) );
    ofs.write( (char*)(&m_flYStep), sizeof(m_flYStep) );

    ofs.write( (char*)(&m_scanType), sizeof(m_scanType) );

    return !ofs.fail();
}

//Load
bool AutoProbeRect::ReadFromFile( std::ifstream &ifs, char cVer )
{
    if ( !AutoProbeRenderable::ReadFromFile( ifs, cVer ) )
        return false;

    ifs.read( (char*)(&m_pt1), sizeof(m_pt1) );
    ifs.read( (char*)(&m_pt2), sizeof(m_pt2) );
    ifs.read( (char*)(&m_flXStep), sizeof(m_flXStep) );
    ifs.read( (char*)(&m_flYStep), sizeof(m_flYStep) );

    if ( cVer >= 4 )
    {
        ifs.read( (char*)(&m_scanType), sizeof(m_scanType) );
    }

    RegenerateImages();
    return true;
}

//Gives effective scan type including alternating directions
enum AutoProbeRect::ScanType AutoProbeRect::EffectiveScanType( unsigned int row, unsigned int col ) const
{
    enum ScanType ret = ScanType();
    if ( ret == AUTO )
    {
        if ( Rows() > Columns() )
            ret = TB;
        else
            ret = LR;
    }

    if ( ret == LR || ret == RL )
    {
        if ( row % 2 != 0 )
            ret = RL;
        else
            ret = LR;
    }
    else
    {
        if ( col % 2 != 0 )
            ret = BT;
        else
            ret = TB;
    }
    return ret;
}
