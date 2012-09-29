/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef CALIBRATEZDLG_H
#define CALIBRATEZDLG_H

#include <QDialog>
#include <QDoubleValidator>
#include <QLineEdit>

namespace Ui {
    class CalibrateZDlg;
}

class CalibrateZDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrateZDlg(QWidget *parent = 0);
    ~CalibrateZDlg();

private:
    void SetLineEditValue( QLineEdit* lineEdit, float flValue );
    bool ValidateLineEdit( QLineEdit* lineEdit, const QValidator& validator );

    Ui::CalibrateZDlg *ui;
    QDoubleValidator doubleValidator;

public slots:
    void show();

private slots:
    void on_okButton_clicked();
};

#endif // CALIBRATEZDLG_H
