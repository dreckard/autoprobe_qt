#include "brightnesscontrast.h"
#include "ui_brightnesscontrast.h"
#include "AutoProbe_Objects.h"
#include "mainwindow.h"
#include "Constants.h"
#include <QPushButton>
#include <sstream>

BrightnessContrast::BrightnessContrast(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BrightnessContrast),
    m_usRenderMode( RENDERMODE_SPEC1 ),
    m_pObj( NULL )
{
    ui->setupUi(this);
    QPushButton* pApply = ui->buttonBox->button(QDialogButtonBox::Apply);
    if ( pApply ) pApply->setDefault(true);
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint ); //No "what's this" button
    setFixedSize( width(), height() ); //No resize
}

BrightnessContrast::~BrightnessContrast()
{
    delete ui;
}

void BrightnessContrast::show( void )
{
    UpdateMaxMin();
    QDialog::show();
}

void BrightnessContrast::UpdateUIElements( void )
{
    bool bMin = !ui->autoMin->isChecked();
    bool bMax = !ui->autoMax->isChecked();

    ui->minLineEdit->setEnabled( bMin );
    //ui->minScrollBar->setEnabled( bMin );

    ui->maxLineEdit->setEnabled( bMax );
    //ui->maxScrollBar->setEnabled( bMax );
}

bool BrightnessContrast::CheckEnable( void )
{
    bool bEnable = false;
    if ( m_usRenderMode >= RENDERMODE_FIRST_IMG && m_usRenderMode <= RENDERMODE_LAST_IMG )
    {
        if ( m_pObj )
            bEnable = true;
    }
    setEnabled( bEnable );
    return bEnable;
}

void BrightnessContrast::RenderModeChanged( unsigned char cRenderMode )
{
    m_usRenderMode = cRenderMode;
    switch ( cRenderMode )
    {
    case RENDERMODE_WIREFRAME:
        ui->titleLabel->setText( "Invalid Render Mode" );
        break;
    case RENDERMODE_SPEC1:
        ui->titleLabel->setText( "Spectrometer 1" );
        break;
    case RENDERMODE_SPEC2:
        ui->titleLabel->setText( "Spectrometer 2" );
        break;
    case RENDERMODE_SPEC3:
        ui->titleLabel->setText( "Spectrometer 3" );
        break;
    case RENDERMODE_SPEC4:
        ui->titleLabel->setText( "Spectrometer 4" );
        break;
    case RENDERMODE_CURRENT:
        ui->titleLabel->setText( "Current" );
        break;
    }
    if ( CheckEnable() )
    {
        UpdateMaxMin();
        ui->autoMax->setChecked( m_pObj->GetMapAutoMax( m_usRenderMode ) );
        ui->autoMin->setChecked( m_pObj->GetMapAutoMin( m_usRenderMode ) );
    }
}

//Each obj keeps track of its own B/C info
void BrightnessContrast::SelectionChanged( AutoProbeRenderable* pObj )
{
    m_pObj = pObj;
    if ( pObj )
    {
        ui->objectLabel->setText( pObj->m_strDisplayName );
        UpdateMaxMin();
        ui->autoMax->setChecked( m_pObj->GetMapAutoMax( m_usRenderMode ) );
        ui->autoMin->setChecked( m_pObj->GetMapAutoMin( m_usRenderMode ) );
    }
    CheckEnable();
}

void BrightnessContrast::UpdateMaxMin( void )
{
    if ( !m_pObj )
        return;

    std::stringstream s;
    s << m_pObj->GetMapMin( m_usRenderMode );
    ui->minLineEdit->setText( s.str().c_str() );
    s.str( "" );

    s << m_pObj->GetMapMax( m_usRenderMode );
    ui->maxLineEdit->setText( s.str().c_str() );
}

void BrightnessContrast::on_autoMin_stateChanged( int state )
{
    UpdateUIElements();

    m_pObj->SetMapAutoMin( m_usRenderMode, state );
    UpdateMaxMin();
    MainWindow::Instance().UpdateStageView();
}

void BrightnessContrast::on_autoMax_stateChanged( int state )
{
    UpdateUIElements();

    m_pObj->SetMapAutoMax( m_usRenderMode, state );
    UpdateMaxMin();
    MainWindow::Instance().UpdateStageView();
}

void BrightnessContrast::on_buttonBox_accepted()
{
    /*float flMin = ui->minLineEdit->text().toFloat();
    float flMax = ui->maxLineEdit->text().toFloat();

    if ( flMax <= flMin )
    {
        //Revert
        UpdateMaxMin();
        return;
    }

    m_pObj->SetMapBC( m_usRenderMode, flMin, flMax );
    MainWindow::Instance().UpdateStageView();*/
}

void BrightnessContrast::on_buttonBox_clicked(QAbstractButton* button)
{
    if ( ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole )
    {
        float flMin = ui->minLineEdit->text().toFloat();
        float flMax = ui->maxLineEdit->text().toFloat();

        if ( flMax <= flMin )
        {
            //Revert
            UpdateMaxMin();
            return;
        }

        m_pObj->SetMapBC( m_usRenderMode, flMin, flMax );
        MainWindow::Instance().UpdateStageView();
    }
}
