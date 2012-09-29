/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef OPTIONSDLG_H
#define OPTIONSDLG_H

#include <QDialog>

namespace Ui {
    class OptionsDlg;
}

class OptionsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDlg(QWidget *parent = 0);
    ~OptionsDlg();

public slots:
    void show();
    void accept();
    void reject();

private:
    Ui::OptionsDlg *ui;
};

#endif // OPTIONSDLG_H
