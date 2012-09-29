/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef STAGEVIEW_H
#define STAGEVIEW_H

#include <QWidget>
#include <QList>
#include "AutoProbe_Objects.h"

class StageView : public QWidget
{
public:
    StageView( QWidget* parent = 0, Qt::WindowFlags f = 0 );

    ~StageView() { for ( int i = 0; i < renderables.count(); i++ ) delete renderables[i]; }

    //void HandleKeyEvent( QKeyEvent* event );
    bool TryZoom( bool bIn );
    void AddRenderable( AutoProbeRenderable* pRenderable );
    bool RemoveRenderable( AutoProbeRenderable* pRenderable );
    void RemoveAllRenderables( void );
    const QList<AutoProbeRenderable*>& GetRenderables( void ) { return renderables; }
    RenderInfo GetRenderInfo( void );
    void SetRenderBounds( const RenderBounds& bounds ) { m_renderBounds = bounds; }
    float GetZoomMag( void );
    //AutoProbeRenderable* RenderableByWItem( QListWidgetItem* pWidgetItem );

    QList<AutoProbeRenderable*> selection;

private:
    QList<AutoProbeRenderable*> renderables;
    RenderBounds m_renderBounds;
protected:
    /*virtual*/ void paintEvent( QPaintEvent* event );
    /*virtual*/ void mouseMoveEvent( QMouseEvent* event );
};

#endif // STAGEVIEW_H
