/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#include <cstdio>
#include <QPainter>
#include <QKeyEvent>
#include <QToolTip>
#include "StageView.h"
#include "AutoProbe_Objects.h"
#include "mainwindow.h"

StageView::StageView( QWidget *parent, Qt::WindowFlags f ) : QWidget( parent, f )
{
    setMouseTracking(true);
}

/*void StageView::HandleKeyEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_Plus || event->key() == Qt::Key_Minus )
    {
        //Allowable range of mags: 1x-16x, rendering images gets real sluggish at higher mags
        if ( event->key() == Qt::Key_Plus && GetZoomMag() >= 15.9f )
            return;
        if ( event->key() == Qt::Key_Minus && GetZoomMag() <= 1.1f )
            return;

        QPoint ctr = mapFromGlobal(QCursor::pos());
        float flOldW = GetRenderInfo().BoundedWidth();
        float flOldH = GetRenderInfo().BoundedHeight();
        Float2 stgCtr;
        if ( event->key() == Qt::Key_Minus && GetZoomMag() <= 2.1f )
        {
            const AutoProbeSettings& settings = MainWindow::Instance().m_Settings;
            stgCtr.X = (settings.m_flStageLimits[0][0] + settings.m_flStageLimits[0][1])/2;
            stgCtr.Y = (settings.m_flStageLimits[1][0] + settings.m_flStageLimits[1][1])/2;
        }
        else if ( ctr.x() >= 0 && ctr.x() <= width() && ctr.y() >= 0 && ctr.y() <= height() )
            stgCtr = GetRenderInfo().ScreenToStage( Int2( ctr.x(), ctr.y() ) );
        else
            stgCtr = Float2( (m_renderBounds.flX[0]+m_renderBounds.flX[1])/2,
                             (m_renderBounds.flY[0]+m_renderBounds.flY[1])/2 );

        float flScl = 0.5f; flScl *= (event->key() == Qt::Key_Plus) ? 0.5f : 2.0f;
        SetRenderBounds( RenderBounds( stgCtr.X - flOldW*flScl, stgCtr.X + flOldW*flScl,
                                       stgCtr.Y - flOldH*flScl, stgCtr.Y + flOldH*flScl ) );
        update();
    }
}*/

bool StageView::TryZoom( bool bIn )
{
    //Allowable range of mags: 1x-16x, rendering images gets real sluggish at higher mags
    if ( bIn && GetZoomMag() >= 15.9f )
        return false;
    if ( !bIn && GetZoomMag() <= 1.1f )
        return false;

    QPoint ctr = mapFromGlobal(QCursor::pos());
    float flOldW = GetRenderInfo().BoundedWidth();
    float flOldH = GetRenderInfo().BoundedHeight();
    Float2 stgCtr;
    if ( !bIn && GetZoomMag() <= 2.1f )
    {
        const AutoProbeSettings& settings = MainWindow::Instance().m_Settings;
        stgCtr.X = (settings.m_flStageLimits[0][0] + settings.m_flStageLimits[0][1])/2;
        stgCtr.Y = (settings.m_flStageLimits[1][0] + settings.m_flStageLimits[1][1])/2;
    }
    else if ( ctr.x() >= 0 && ctr.x() <= width() && ctr.y() >= 0 && ctr.y() <= height() )
        stgCtr = GetRenderInfo().ScreenToStage( Int2( ctr.x(), ctr.y() ) );
    else
        stgCtr = Float2( (m_renderBounds.flX[0]+m_renderBounds.flX[1])/2,
                         (m_renderBounds.flY[0]+m_renderBounds.flY[1])/2 );

    float flScl = 0.5f; flScl *= bIn ? 0.5f : 2.0f;
    SetRenderBounds( RenderBounds( stgCtr.X - flOldW*flScl, stgCtr.X + flOldW*flScl,
                                   stgCtr.Y - flOldH*flScl, stgCtr.Y + flOldH*flScl ) );
    update();

    return true;
}

void StageView::AddRenderable( AutoProbeRenderable *pRenderable )
{
    renderables.append( pRenderable );
    update();
}
bool StageView::RemoveRenderable( AutoProbeRenderable *pRenderable )
{
    for ( int i = 0; i < renderables.count(); i++ )
    {
        if ( renderables.at(i) == pRenderable )
        {
            delete renderables.at(i);
            renderables.removeAt(i);
            update();
            return true;
        }
    }
    return false;
}
void StageView::RemoveAllRenderables( void )
{
    for ( int i = 0; i < renderables.count(); i++ )
    {
        delete renderables[i];
    }
    renderables.clear();
    update();
}

RenderInfo StageView::GetRenderInfo( void )
{
    return RenderInfo( width(), height(), MainWindow::Instance().m_Settings.m_flStageLimits[0][1],
                       MainWindow::Instance().m_Settings.m_flStageLimits[1][1], m_renderBounds );
}

float StageView::GetZoomMag( void )
{
    //Assumes same in both dimensions
    return (MainWindow::Instance().m_Settings.m_flStageLimits[0][1]-MainWindow::Instance().m_Settings.m_flStageLimits[0][0])
            / GetRenderInfo().BoundedWidth();
}

void StageView::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );

    const AutoProbeSettings& settings = MainWindow::Instance().m_Settings;

    //Coordinate axes
    painter.setPen( Qt::gray );
    Int2 hC = GetRenderInfo().StageToScreen( Float2( (settings.m_flStageLimits[0][1] + settings.m_flStageLimits[0][0])/2, 0.0f ) );
    Int2 vC = GetRenderInfo().StageToScreen( Float2( 0.0f, (settings.m_flStageLimits[1][1] + settings.m_flStageLimits[1][0])/2 ) );
    painter.drawLine( QLine( hC.X, 0.0f, hC.X, height() ) );
    painter.drawLine( QLine( 0.0f, vC.Y, width(), vC.Y ) );
    //painter.drawLine( QLine(width()/2, 0, width()/2, height() ) );
    //painter.drawLine( QLine(0, height()/2, width(), height()/2 ) );

    for ( int i = 0; i < renderables.count(); i++ )
    {
        if ( renderables[i]->IsVisible( GetRenderInfo() ) )
            renderables[i]->Paint( painter, GetRenderInfo() );
    }

    //Zoom
    if ( !fltEq(GetZoomMag(), 1.0f, 0.01f) )
    {
        painter.setPen( Qt::black );
        QFont font = painter.font(); font.setBold( true );
        painter.setFont( font );
        std::stringstream s; s << "Zoom: " << GetZoomMag() << "x";
        painter.drawText( 5, 8, s.str().c_str() );
    }
}

void StageView::mouseMoveEvent( QMouseEvent* event )
{
    Float2 stgCoords = GetRenderInfo().ScreenToStage( Int2(event->pos().x(),event->pos().y()) );
    char szMsg[16];
    snprintf( szMsg, 16, "%.3f %.3f", stgCoords.X, stgCoords.Y );
    MainWindow::Instance().SetStatusBarMsg( szMsg );
}

