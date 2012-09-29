/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#include <cstdio>
#include <QDoubleValidator>
#include <QMessageBox>
#include "mainwindow.h"
#include "PointData.h"
#include "calibratezdlg.h"
#include "ui_calibratezdlg.h"
#include "3D_Math.h"
#include "AutoProbeError.h"

CalibrateZDlg::CalibrateZDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalibrateZDlg),
    doubleValidator( 0.0, 100.0, 3, parent )
{
    ui->setupUi(this);
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint ); //No "what's this" button
    setFixedSize( width(), height() ); //No resize

    //QObject::connect( ui->okButton, SIGNAL(clicked()), this, SLOT(on_pushButton_clicked()) );

    ui->lineEdit->setValidator( &doubleValidator );
    ui->lineEdit_2->setValidator( &doubleValidator );
    ui->lineEdit_3->setValidator( &doubleValidator );
    ui->lineEdit_4->setValidator( &doubleValidator );
    ui->lineEdit_5->setValidator( &doubleValidator );
    ui->lineEdit_6->setValidator( &doubleValidator );
    ui->lineEdit_7->setValidator( &doubleValidator );
    ui->lineEdit_8->setValidator( &doubleValidator );
    ui->lineEdit_9->setValidator( &doubleValidator );
}

CalibrateZDlg::~CalibrateZDlg()
{
    delete ui;
}

void CalibrateZDlg::SetLineEditValue( QLineEdit* lineEdit, float flValue )
{
    char szTxt[16];
    snprintf( szTxt, 16, "%.3f", flValue );
    lineEdit->setText( szTxt );
}

void CalibrateZDlg::show()
{
    Vec3 pt1, pt2, pt3;
    MainWindow::Instance().GetPlanePts( pt1, pt2, pt3 );

    SetLineEditValue( ui->lineEdit, pt1.x );
    SetLineEditValue( ui->lineEdit_2, pt1.y );
    SetLineEditValue( ui->lineEdit_3, pt1.z );
    SetLineEditValue( ui->lineEdit_4, pt2.x );
    SetLineEditValue( ui->lineEdit_5, pt2.y );
    SetLineEditValue( ui->lineEdit_6, pt2.z );
    SetLineEditValue( ui->lineEdit_7, pt3.x );
    SetLineEditValue( ui->lineEdit_8, pt3.y );
    SetLineEditValue( ui->lineEdit_9, pt3.z );

    QDialog::show();
}

bool CalibrateZDlg::ValidateLineEdit( QLineEdit* lineEdit, const QValidator& validator)
{
    QString qstr = lineEdit->text();
    int i = 0;
    if ( validator.validate( qstr, i ) != QValidator::Acceptable )
    {
        lineEdit->setFocus( Qt::OtherFocusReason );
        lineEdit->selectAll();
        return false;
    }
    return true;
}

void CalibrateZDlg::on_okButton_clicked()
{
    if ( MainWindow::Instance().EditLocked() )
    {
        MainWindow::Instance().EditLockNotification();
        return;
    }

    //Gross
    if ( !ValidateLineEdit( ui->lineEdit, doubleValidator ) || !ValidateLineEdit( ui->lineEdit_2, doubleValidator ) ||
         !ValidateLineEdit( ui->lineEdit_3, doubleValidator ) || !ValidateLineEdit( ui->lineEdit_4, doubleValidator ) ||
         !ValidateLineEdit( ui->lineEdit_5, doubleValidator ) || !ValidateLineEdit( ui->lineEdit_6, doubleValidator ) ||
         !ValidateLineEdit( ui->lineEdit_7, doubleValidator ) || !ValidateLineEdit( ui->lineEdit_8, doubleValidator ) ||
         !ValidateLineEdit( ui->lineEdit_9, doubleValidator ) )
        return;

    //Planes (TO DO: trains and automobiles)
    Vec3 pt1( ui->lineEdit->text().toFloat(), ui->lineEdit_2->text().toFloat(), ui->lineEdit_3->text().toFloat() );
    Vec3 pt2( ui->lineEdit_4->text().toFloat(), ui->lineEdit_5->text().toFloat(), ui->lineEdit_6->text().toFloat() );
    Vec3 pt3( ui->lineEdit_7->text().toFloat(), ui->lineEdit_8->text().toFloat(), ui->lineEdit_9->text().toFloat() );

    if ( pt1 == pt2 || pt1 == pt3 || pt2 == pt3 )
    {
        ap_error( "Points must be non-collinear and unique" );
        return;
    }

    if ( (pt1-pt2).ParallelTo( (pt2-pt3) ) )
    {
        ap_error( "Points must be non-collinear" );
        return;
    }

    Vec3 vecNormal = (pt1-pt2); vecNormal.Cross( (pt2-pt3) ); vecNormal.NormalizeInPlace();
    Plane plane( pt1, vecNormal );

    MainWindow::Instance().SetPlane( plane, pt1, pt2, pt3 );

    close();
}
