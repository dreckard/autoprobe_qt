/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef POINTDATA_H
#define POINTDATA_H

#include <QtGui/QApplication>
#include <QAbstractTableModel>
#include <vector>
#include "Constants.h"

#define __attribute__(spec)
#define NUM_POINT_COLS 11
struct point_s //Size: 33 bytes
{
    float flPos[3];
    float flAbsCurrent;
    float flSpecCounts[NUM_SPECTROMETERS]; //Inherently int quantity but inertia and convenience are keeping this float
    bool bCollected;
} __attribute__ ((packed));
#undef __attribute__

class SerializedPoints
{
    public:
    float flCountTime;
    unsigned int iNumPts;
    point_s* points;

    SerializedPoints();
    SerializedPoints( unsigned int num_pts );
    ~SerializedPoints();
};

template <class T>
class Matrix2d
{
private:
    std::vector< std::vector<T> > data;

public:
    Matrix2d() { data.clear(); }
    Matrix2d( unsigned int iRows, unsigned int iCols )
    {
        data.clear();
        data.resize( iRows, std::vector<T>( iCols ) );
    }
    void setRows( unsigned int iRows ) { data.resize( iRows ); }
    void setCols( unsigned int iCols )
    {
        for ( unsigned int i = 0; i < rows(); i++ )
            data.at(i).resize( iCols );
    }
    void setValue( unsigned int iRow, unsigned int iCol, const T& value )
    {
        at( iRow, iCol ) = value;
    }
    unsigned int rows( void ) const { return data.size(); }
    unsigned int cols( void ) const { return data.empty() ? 0 : data.at(0).size(); }
    bool inRange( unsigned int iRow, unsigned int iCol ) const { return iRow < rows() && iCol < cols() && iRow >= 0 && iCol >= 0; }
    unsigned int count( void ) const { return rows()*cols(); }
    const std::vector<T>& row( unsigned int iRow ) const { return data.at(iRow); }
    const T& at( unsigned int iRow, unsigned int iCol ) const { return data.at(iRow).at(iCol); }
    T& at( unsigned int iRow, unsigned int iCol ) { return data.at(iRow).at(iCol); }
    unsigned int rowbyidx( unsigned int iIdx ) const { return iIdx/cols(); }
    unsigned int colbyidx( unsigned int iIdx ) const { return iIdx % cols(); }
    unsigned int idxbyrowcol( unsigned int iRow, unsigned int iCol ) const { return iRow*cols() + iCol; }
    const T& idx( unsigned int iIdx ) const { return at( rowbyidx(iIdx), colbyidx(iIdx) ); }
    T& idx( unsigned int iIdx ) { return at( rowbyidx(iIdx), colbyidx(iIdx) ); }
    void clear( void ) { data.clear(); }
};

class AutoProbeRenderable;
class PointsTableModel : public QAbstractTableModel
{
public:
    void ResetAll( void ) { reset(); }
    void MarkPointDirty( unsigned int iPoint );
    void ClearPointByOverallIdx( unsigned int idx );
    const point_s* PointByOverallIdx( unsigned int idx ) const;
    const AutoProbeRenderable* RenderableByOverallIdx( unsigned int idx ) const;

private:
    point_s* PointByOverallIdx( unsigned int idx );
    virtual int flags( void ) { return Qt::NoItemFlags; }
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int columnCount(const QModelIndex &parent) const { return NUM_POINT_COLS; }
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};


#endif // POINTDATA_H
