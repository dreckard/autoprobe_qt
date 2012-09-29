#ifndef BRIGHTNESSCONTRAST_H
#define BRIGHTNESSCONTRAST_H

#include <QDialog>
#include <QAbstractButton>
#include "AutoProbe_Objects.h"
#include "Constants.h"

namespace Ui {
    class BrightnessContrast;
}

class BrightnessContrast : public QDialog
{
    Q_OBJECT

public:
    explicit BrightnessContrast(QWidget *parent = 0);
    ~BrightnessContrast();

    void RenderModeChanged( unsigned char cRenderMode );
    void SelectionChanged( AutoProbeRenderable* pObj );

private:
    Ui::BrightnessContrast *ui;
    unsigned char m_usRenderMode;
    AutoProbeRenderable* m_pObj;

    void UpdateUIElements( void );
    bool CheckEnable( void );
    void UpdateMaxMin( void );

public slots:
    void show();

private slots:
    void on_buttonBox_clicked(QAbstractButton* button);
    void on_buttonBox_accepted();
    void on_autoMax_stateChanged(int );
    void on_autoMin_stateChanged(int );
};

#endif // BRIGHTNESSCONTRAST_H
