/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#include <QMessageBox>
#include "optionsdlg.h"
#include "ui_optionsdlg.h"
#include "mainwindow.h"
#include "AutoProbeError.h"
#include "Constants.h"
#include "Utility.h"

OptionsDlg::OptionsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDlg)
{
    ui->setupUi(this);
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint ); //No "what's this" button
    setFixedSize( width(), height() ); //No resize
}

OptionsDlg::~OptionsDlg()
{
    delete ui;
}

void OptionsDlg::show()
{
    AutoProbeSettings& settings = MainWindow::Instance().m_Settings;
    ui->CB_ComPort->setCurrentIndex( rd_bound( settings.m_sComPort - 1, 0, 4 ) ); //- 1 for zero based indices

    ui->LE_MaxPts->setText( AsString( settings.m_iMaxPts ).c_str() );
    ui->LE_LScanBlock->setText( AsString( settings.m_iFTLineScanBlock ).c_str() );
    ui->LE_PtBlock->setText( AsString( settings.m_iFTPointBlock ).c_str() );
    ui->LE_Timeout->setText( AsString( settings.m_flFTTimeout ).c_str() );

    ui->CB_AutoSave->setCurrentIndex( settings.m_sAutoSaveMode );
    ui->CB_CurHalt->setChecked( settings.m_bHaltOnZeroCurrent );
    ui->CB_BlankBeam->setChecked( settings.m_bBlankBeamOnFinish );
    ui->CB_Autoresume->setChecked( settings.m_bAutoresume );
    ui->CB_ExecPostScript->setChecked( settings.m_bExecPostScript );

    ui->LE_XMin1->setText( AsString( settings.m_flStageLimits[0][0] ).c_str() );
    ui->LE_XMax1->setText( AsString( settings.m_flStageLimits[0][1] ).c_str() );
    ui->LE_YMin1->setText( AsString( settings.m_flStageLimits[1][0] ).c_str() );
    ui->LE_YMax1->setText( AsString( settings.m_flStageLimits[1][1] ).c_str() );
    ui->LE_ZMin1->setText( AsString( settings.m_flStageLimits[2][0] ).c_str() );
    ui->LE_ZMax1->setText( AsString( settings.m_flStageLimits[2][1] ).c_str() );

    QDialog::show();
}

void OptionsDlg::accept( void )
{
    AutoProbeSettings& settings = MainWindow::Instance().m_Settings;

    if ( settings.m_sComPort != ui->CB_ComPort->currentIndex() + 1 )
    {
        if ( !MainWindow::Instance().InitSerial( ui->CB_ComPort->currentIndex() + 1 ) )
            ap_error( "Failed to initialize serial port" );
    }

    settings.m_sComPort = ui->CB_ComPort->currentIndex() + 1;

    FromString( settings.m_iMaxPts, ui->LE_MaxPts->text().toStdString() );
    FromString( settings.m_iFTLineScanBlock, ui->LE_LScanBlock->text().toStdString() );
    FromString( settings.m_iFTPointBlock, ui->LE_PtBlock->text().toStdString() );
    FromString( settings.m_flFTTimeout, ui->LE_Timeout->text().toStdString() );

    settings.m_sAutoSaveMode = ui->CB_AutoSave->currentIndex();
    settings.m_bHaltOnZeroCurrent = ui->CB_CurHalt->isChecked();
    settings.m_bBlankBeamOnFinish = ui->CB_BlankBeam->isChecked();
    settings.m_bAutoresume = ui->CB_Autoresume->isChecked();
    settings.m_bExecPostScript = ui->CB_ExecPostScript->isChecked();

    FromString( settings.m_flStageLimits[0][0], ui->LE_XMin1->text().toStdString() );
    FromString( settings.m_flStageLimits[0][1], ui->LE_XMax1->text().toStdString() );
    FromString( settings.m_flStageLimits[1][0], ui->LE_YMin1->text().toStdString() );
    FromString( settings.m_flStageLimits[1][1], ui->LE_YMax1->text().toStdString() );
    FromString( settings.m_flStageLimits[2][0], ui->LE_ZMin1->text().toStdString() );
    FromString( settings.m_flStageLimits[2][1], ui->LE_ZMax1->text().toStdString() );

    MainWindow::Instance().SetRenderBounds( RenderBounds( MainWindow::Instance().m_Settings.m_flStageLimits[0][0],
                                                          MainWindow::Instance().m_Settings.m_flStageLimits[0][1],
                                                          MainWindow::Instance().m_Settings.m_flStageLimits[1][0],
                                                          MainWindow::Instance().m_Settings.m_flStageLimits[1][1] ) );

    settings.Write( CFG_FILE );

    QDialog::accept();
}

void OptionsDlg::reject( void )
{
    QDialog::reject();
}
