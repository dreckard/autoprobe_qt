/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#include <QItemSelection>
#include "autoprobeobjview.h"
#include "mainwindow.h"

AutoProbeObjView::AutoProbeObjView(QWidget *parent) :
    QListView(parent)
{
}

void AutoProbeObjView::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
    QModelIndexList list = selectedIndexes();
    MainWindow::Instance().ListViewSelectionUpdated( list );

    QListView::selectionChanged( selected, deselected );
}
