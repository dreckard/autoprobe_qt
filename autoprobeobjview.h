/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef AUTOPROBEOBJVIEW_H
#define AUTOPROBEOBJVIEW_H

#include <QListView>
#include <QModelIndexList>

class AutoProbeObjView : public QListView
{
    Q_OBJECT
public:
    explicit AutoProbeObjView(QWidget *parent = 0);

    virtual void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

};

#endif // AUTOPROBEOBJVIEW_H
