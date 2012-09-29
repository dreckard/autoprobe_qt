/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef AUTOPROBE_OBJECTS_H
#define AUTOPROBE_OBJECTS_H

#include <QPainter>
#include <QTableWidget>
#include <QStringListModel>
#include <QStringList>
#include <cmath>
#include "PointData.h"
#include "Constants.h"
#include "../clib_superprobe733/clib_superprobe733.h"

enum Object_Type
{
    Obj_Point,
    Obj_Rect
};

class Int2
{
public:
    int X, Y;
    Int2() { X = 0; Y = 0; }
    Int2( int iInitialVal ) { X = iInitialVal; Y = iInitialVal; }
    Int2( int iX, int iY ) { X = iX; Y = iY; }
};

class Float2
{
public:
    float X, Y;
    Float2() { X = 0.0f; Y = 0.0f; }
    Float2( float flInitialVal ) { X = flInitialVal; Y = flInitialVal; }
    Float2( float flX, float flY ) { X = flX; Y = flY; }
};

class LScan
{
private:
    Float3 m_pt1;
    Float3 m_pt2;

    unsigned int m_startRow;
    unsigned int m_startCol;
    float m_flDwellTime;
    int m_iNumPts;
public:
    LScan() : m_pt1( 0.0f ), m_pt2( 0.0f ), m_startRow( 0 ), m_startCol( 0 ), m_flDwellTime( 0.0f ), m_iNumPts( 0 ) {}
    LScan( Float3 p1, Float3 p2, unsigned int row, unsigned int col, float dt, int nPts ) : m_pt1(p1), m_pt2(p2), m_startRow(row), m_startCol(col), m_flDwellTime(dt), m_iNumPts(nPts) {}

    const Float3& pt1( void ) { return m_pt1; }
    const Float3& pt2( void ) { return m_pt2; }
    unsigned int startRow( void ) { return m_startRow; }
    unsigned int startCol( void ) { return m_startCol; }
    float DwellTime( void ) { return m_flDwellTime; }
    int NumPoints( void ) { return m_iNumPts; }
};

//Holds stage coords of render bounds
class RenderBounds
{
public:
    float flX[2];
    float flY[2];

    RenderBounds() { flX[0] = 0.0f; flX[1] = 0.0f; flY[0] = 0.0f; flY[1] = 0.0f; }
    RenderBounds( float flX1, float flX2, float flY1, float flY2 )
    {
        flX[0] = flX1; flX[1] = flX2; flY[0] = flY1; flY[1] = flY2;
    }
};

class RenderInfo
{
public:
    int iWidth, iHeight;
    float flStageLimX, flStageLimY; //Deprecated
    float flScale;
    RenderBounds renderBounds;

    RenderInfo() { iWidth = 0; iHeight = 0; flStageLimX = 0.0f; flStageLimY = 0.0f; }
    RenderInfo( int w, int h, float limX, float limY, const RenderBounds& bounds )
    { iWidth = w; iHeight = h; flStageLimX = limX; flStageLimY = limY; renderBounds = bounds; }

    float BoundedWidth( void ) const { return renderBounds.flX[1] - renderBounds.flX[0]; }
    float BoundedHeight( void ) const { return renderBounds.flY[1] - renderBounds.flY[0]; }

    Float2 ScreenToStage( Int2 ptScr ) const
    {
        Float2 ptStg;

        ptStg = Float2( renderBounds.flX[0]+((BoundedWidth()*ptScr.X)/iWidth),
                         renderBounds.flY[0]+((BoundedHeight()*ptScr.Y)/iHeight) );

        return ptStg;
    }
    Int2 StageToScreen( Float2 ptStg ) const
    {
        Int2 ptScr;
        ptScr = Int2( int(((ptStg.X-renderBounds.flX[0])*iWidth)/BoundedWidth()),
                      int(((ptStg.Y-renderBounds.flY[0])*iHeight)/BoundedHeight()) );

        return ptScr;
    }
    bool StagePointVisible( Float2 ptStg ) const
    {
        return ( ptStg.X >= renderBounds.flX[0] && ptStg.X <= renderBounds.flX[1] &&
                 ptStg.Y >= renderBounds.flY[0] && ptStg.Y <= renderBounds.flY[1] );
    }
};

Float2 ScreenToStage( Int2 ptScr, const RenderInfo& renderInfo );
Int2 StageToScreen( Float2 rptStg, const RenderInfo& renderInfo );
bool StagePointVisible( Float2 ptStg, const RenderInfo& renderInfo );
class AutoProbeRenderable
{
public:
    QString m_strDisplayName;

protected:
    float m_flDwellTime;
    Matrix2d<point_s> m_PointMatrix;
    QList<QImage> m_Images; //The reason this isn't a static array is that a QImage must be initialized with its desired width and height
    float m_flMaxValue[RENDERMODE_NUM_IMG];
    float m_flMinValue[RENDERMODE_NUM_IMG];
    bool m_bAutoMin[RENDERMODE_NUM_IMG];
    bool m_bAutoMax[RENDERMODE_NUM_IMG];
    QVector<QRgb> m_colorTable;

public:
    virtual Object_Type ObjectType( void ) const = 0;
    virtual const Matrix2d<point_s>& GetPointMatrix( void ) const { return m_PointMatrix; }
    virtual Matrix2d<point_s>& GetPointMatrix( void ) { return m_PointMatrix; }
    virtual void UpdatePointMatrix( bool bInvalidate ) = 0;
    virtual bool GetNextLScan( unsigned int curRow, unsigned int curCol, LScan& lscan ) = 0;
    virtual bool DataReceived( const TN5500_Data& data, unsigned int& row, unsigned int& col ) = 0;
    virtual bool GetClickableArea( Int2& Mins, Int2& Maxs, const RenderInfo& renderInfo ) const = 0;
    virtual bool GetScreenSpaceCenter( Int2& Center, const RenderInfo& renderInfo ) const = 0;
    virtual void Paint( QPainter& painter, const RenderInfo& renderInfo ) = 0;
    virtual bool IsVisible( const RenderInfo& renderInfo ) = 0;
    virtual void SetupPropertyTable( QTableWidget* pTableWidget ) = 0;
    virtual void PropertyTableUpdated( QTableWidget* pTableWidget ) = 0;
    virtual void ClearPointData( void );
    virtual bool WriteToFile( std::ofstream& ofs );
    virtual bool ReadFromFile( std::ifstream& ifs, char cVer );
    virtual int NumPoints( void ) const = 0;

    float DwellTime( void ) const { return m_flDwellTime; }
    virtual void UpdateImages( unsigned int iRow, unsigned int iCol ) = 0;
    virtual void RegenerateImage( unsigned int idx ) = 0;
    virtual void RegenerateImages( void ); //This function does NOT implicitly update the point matrix
    void BlankImages( void );
    unsigned short RenderModeToImageIdx( unsigned short usRenderMode ) { return usRenderMode-RENDERMODE_FIRST_IMG; }
    virtual const QImage& GetImage( unsigned short usRenderMode )
    {
        unsigned short idx = RenderModeToImageIdx( usRenderMode );
        if ( m_Images.empty() )
            RegenerateImages();
        if ( idx > m_Images.count() )
            return m_Images.at(0);
        return m_Images.at(idx);
    }
    void SetMapBC( unsigned short usRenderMode, float flMin, float flMax ); //Has no effect if AutoMin and AutoMax are true
    void SetMapAutoMin( unsigned short usRenderMode, bool bAuto );
    void SetMapAutoMax( unsigned short usRenderMode, bool bAuto );
    bool GetMapAutoMin( unsigned short usRenderMode ) { return m_bAutoMin[RenderModeToImageIdx(usRenderMode)]; }
    bool GetMapAutoMax( unsigned short usRenderMode ) { return m_bAutoMax[RenderModeToImageIdx(usRenderMode)]; }
    float GetMapMin( unsigned short usRenderMode ) { return m_flMinValue[RenderModeToImageIdx(usRenderMode)]; }
    float GetMapMax( unsigned short usRenderMode ) { return m_flMaxValue[RenderModeToImageIdx(usRenderMode)]; }

protected:
    bool IsSelected( void );
    bool VerifyAndSetFlt( QTableWidgetItem* pItem, float* pVar, float flMin, float flMax );
    float GetMinCounts( unsigned char cSpec );
    float GetMinCurrent( void );
    float GetMaxCounts( unsigned char cSpec );
    float GetMaxCurrent( void );
    void GetMinMaxCounts( unsigned char cSpec, float& flMin, float& flMax );
    void GetMinMaxCurrent( float& flMin, float& flMax );

    AutoProbeRenderable() : m_flDwellTime( 0.1 ), m_colorTable( 256 )
    {
        for ( int i = 0; i < 256; i++ )
            m_colorTable[i] = qRgb( i, i, i );
        m_Images.reserve( RENDERMODE_NUM_IMG );
        for ( unsigned int i = 0; i < RENDERMODE_NUM_IMG; i++ )
        {
            m_flMinValue[i] = 0.0f;
            m_flMaxValue[i] = 0.0f;
            m_bAutoMin[i] = true;
            m_bAutoMax[i] = true;
        }
    }
};

class AutoProbePoint : public AutoProbeRenderable
{
protected:
    Float2 m_pt;

public:
    AutoProbePoint( Float2 pt )
    {
        m_pt = pt;
        m_strDisplayName = "New Point";
        UpdatePointMatrix(true);
        RegenerateImages();
    }
    AutoProbePoint( std::ifstream& ifs, char cVer ) { ReadFromFile( ifs, cVer ); }

    Object_Type ObjectType( void ) const { return Obj_Point; }
    void UpdatePointMatrix( bool bInvalidate );
    bool GetNextLScan( unsigned int curRow, unsigned int curCol, LScan& lscan );
    bool DataReceived( const TN5500_Data& data, unsigned int& row, unsigned int& col );
    void RegenerateImage( unsigned int idx ) {}
    void UpdateImages( unsigned int iRow, unsigned int iCol ) {}
    bool GetClickableArea( Int2 &Mins, Int2 &Maxs, const RenderInfo &renderInfo ) const;
    bool GetScreenSpaceCenter( Int2 &Center, const RenderInfo &renderInfo ) const;
    void Paint( QPainter &painter, const RenderInfo &renderInfo );
    bool IsVisible( const RenderInfo &renderInfo );
    void SetupPropertyTable( QTableWidget* pTableWidget );
    void PropertyTableUpdated( QTableWidget* pTableWidget );
    bool WriteToFile( std::ofstream& ofs );
    bool ReadFromFile( std::ifstream& ifs, char cVer );
    int NumPoints( void ) const { return 1; }
};

class AutoProbeRect : public AutoProbeRenderable
{
public:
    enum ScanType
    {
        AUTO = 0,
        LR,
        RL,
        TB,
        BT
    };

private:
    Float2 m_pt1, m_pt2;
    float m_flXStep, m_flYStep;
    unsigned int m_iLastRow, m_iLastCol; //Cache last pos collected to speed up finding the next
    enum ScanType m_scanType;

private:
    bool FindNextPoint( unsigned int curRow, unsigned int curCol, bool bCollected, unsigned int& row, unsigned int& col, bool bReverse = false ) const;
    bool FindFirstPointOnLine( unsigned int curRow, unsigned int curCol, bool bCollected, unsigned int& row, unsigned int& col ) const;
    bool FindNextPointOnLine( unsigned int curRow, unsigned int curCol, bool bCollected, unsigned int& row, unsigned int& col ) const;
    bool StepRowCol( unsigned int& row, unsigned int& col, bool bLineOnly, bool bReverse = false ) const;
    bool Collinear( unsigned int r1, unsigned int c1, unsigned int r2, unsigned int c2 ) const;

public:
    AutoProbeRect( Float2 pt1, Float2 pt2, float xStep, float yStep, float flDwellTime, enum ScanType scanType ) :
            m_pt1(pt1), m_pt2(pt2), m_flXStep(xStep), m_flYStep(yStep), m_iLastRow(0), m_iLastCol(0), m_scanType( scanType )
    {
        m_strDisplayName = "New Rect";
        UpdatePointMatrix(true);
        RegenerateImages();
    }
    AutoProbeRect( std::ifstream& ifs, char cVer ) : m_scanType( AUTO ) { ReadFromFile( ifs, cVer ); }

    Object_Type ObjectType( void ) const { return Obj_Rect; }
    void UpdatePointMatrix( bool bInvalidate );
    bool GetNextLScan( unsigned int curRow, unsigned int curCol, LScan& lscan );
    bool DataReceived( const TN5500_Data& data, unsigned int& row, unsigned int& col );
    void RegenerateImage( unsigned int idx );
    void UpdateImages( unsigned int iRow, unsigned int iCol );
    bool GetClickableArea( Int2& Mins, Int2& Maxs, const RenderInfo& renderInfo ) const;
    bool GetScreenSpaceCenter( Int2& Center, const RenderInfo& renderInfo ) const;
    void Paint( QPainter& painter, const RenderInfo& renderInfo );
    bool IsVisible( const RenderInfo &renderInfo );
    void SetupPropertyTable( QTableWidget* pTableWidget );
    void PropertyTableUpdated( QTableWidget* pTableWidget );
    bool WriteToFile( std::ofstream& ofs );
    bool ReadFromFile( std::ifstream& ifs, char cVer );

    enum ScanType ScanType( void ) const { return m_scanType; }
    enum ScanType EffectiveScanType( unsigned int row, unsigned int col ) const;
    float Width( void ) const { return fabs(m_pt2.X - m_pt1.X); }
    float Height( void ) const { return fabs(m_pt2.Y - m_pt1.Y); }

    //Careful about using too many 9's here, it'll cause overflow instead of normal rounding up with very small step sizes
    int Rows( void ) const { return int((( Height() / m_flYStep ) + 0.999f)) + 1; }
    int Columns( void ) const { return int((( Width() / m_flXStep ) + 0.999f)) + 1; }

    int NumPoints( void ) const { return Rows()*Columns(); }

    int Width_SS( const RenderInfo& renderInfo ) const { return renderInfo.iWidth*(Width()/renderInfo.BoundedWidth()); }
    int Height_SS( const RenderInfo& renderInfo ) const { return renderInfo.iHeight*(Height()/renderInfo.BoundedHeight()); }

};

class ObjectsListModel : public QStringListModel
{
public:
    //virtual int flags( void ) { return Qt::NoItemFlags; }
    virtual int rowCount( const QModelIndex &parent ) const;
    virtual QVariant data( const QModelIndex &index, int role ) const;
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role );

};

#endif // AUTOPROBE_OBJECTS_H
