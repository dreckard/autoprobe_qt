/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <ctime>
#include <deque>
#include <QMainWindow>
#include <QList>
#include <QLabel>
#include <QProgressBar>
#include "calibratezdlg.h"
#include "optionsdlg.h"
#include "brightnesscontrast.h"
#include "AutoProbe_Objects.h"
#include "PointData.h"
#include "3D_Math.h"
#include "CRADFileIO.h"
#include "AutoProbeSettings.h"
#include "Constants.h"
#include "../clib_superprobe733/clib_superprobe733.h"

class PointsTableModel;
class SerializedPoints;
class ListenThread;
class WriteThread;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    AutoProbeSettings m_Settings;
    ListenThread* m_pListenThread;
    WriteThread* m_pWriteThread;

    enum END_STATUS
    {
        SUCCEEDED = 0,
        USER_INTERRUPTED = 1,
        FAILED_GENERIC = 2,
        FAILED_NOCURRENT = 3,
        FAILED_COMMTIMEOUT = 4
    };

    //WARNING: These strings are passed directly to the script so modifying them will break old scripts
    const QString END_STATUS_String( enum END_STATUS status )
    {
        switch (status)
        {
            case SUCCEEDED:
                return "Succeeded";
            case USER_INTERRUPTED:
                return "User_Interrupted";
            case FAILED_GENERIC:
                return "Failed";
            case FAILED_NOCURRENT:
                return "Failed_No_Current";
            case FAILED_COMMTIMEOUT:
                return "Failed_Comm_Timeout";
        }
        return "Unknown run error";
    }

private:
    static MainWindow* m_pInstance;
    Ui::MainWindow *ui;

    bool m_bDevMode;
    time_t m_tTimeElapsed;
    time_t m_tRunStartTime;

    CalibrateZDlg* m_pCalibZDlg;
    OptionsDlg* m_pOptionsDlg;
    BrightnessContrast* m_pBCDlg;
    PointsTableModel* m_pPointsTableModel;
    QString m_lastFilename;
    unsigned char m_cRenderMode;
    bool m_bEditLocked;
    bool m_bUnsavedData;

    Plane m_Plane;
    Vec3 m_planePt1, m_planePt2, m_planePt3;

    bool m_bRun_Active;
    int m_iRun_CurrentRenderable;

    unsigned int m_iCurrentPoints; //Cache
    unsigned int m_iLinePtsRemaining;
    unsigned int m_iZeroPts;
    unsigned int m_iLastRow; //Cache
    unsigned int m_iLastCol; //Cache
    deque<point_s*> qZeroPts;

    QProgressBar* m_pRun_ProgressBar;
    QLabel* m_pRun_StatusLabel;

private:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    MainWindow(const MainWindow&);
    MainWindow & operator=(const MainWindow&);

    void SetLastFileName( const QString& lastFileName );

protected:
    /*virtual*/ void closeEvent( QCloseEvent* event );

public:
    static MainWindow& Instance( void );
    void show( void );
    void keyPressEvent( QKeyEvent * event );

    void EnableDevMode( bool bEnable = true ) { m_bDevMode = bEnable; }
    bool DevModeEnabled( void ) { return m_bDevMode; }

    void ShowOptionsDlg( void ) { m_pOptionsDlg->show(); }
    void SetupPointsTable( void );
    void SetupListView( void );
    void ListViewSelectionUpdated( QModelIndexList& qSelection );
    void resetListView( void );
    void UpdateStageView( void );
    void resetTableView( void );
    void CheckEnableUIElements( void );
    PointsTableModel* GetPointsTableModel( void ) { return m_pPointsTableModel; }

    bool InitSerial( short sComPort );

    const QList<AutoProbeRenderable*>* GetRenderables( void );
    QList<AutoProbeRenderable*> GetSelectedRenderables( void );
    bool AddRenderable( AutoProbeRenderable* pRenderable );
    bool RemoveRenderable( AutoProbeRenderable* pRenderable );
    void RemoveAllRenderables( void );
    unsigned int GetTotalPoints( void );

    bool Run_Active( void ) { return m_bRun_Active; }
    int Run_CurrentRenderable( void ) { return m_iRun_CurrentRenderable; }

    bool Run_Start( void );
    void Run_ScanCurrentLine( void );
    void Run_Abort( enum END_STATUS );
    unsigned int Run_CurrentPoint_Overall( bool bRecount = false );
    void Run_UpdateProgress( void );
    void Run_LimitedUI( bool bSetLimited );
    bool Run_GetNextLScan( LScan& lscan );

    void ResetAll( void );
    void ClearPointData( void );

    void SetRenderMode( unsigned char cRenderMode );
    unsigned char GetRenderMode( void ) { return m_cRenderMode; }
    void SetRenderBounds( const RenderBounds& bounds );

    bool EditLocked( void ) { return m_bEditLocked; }
    bool LockEdit( bool bLock = true );
    void EditLockNotification( void ); //Message box

    bool DataUnsaved( void ) { return m_bUnsavedData; }
    void MarkDataUnsaved( void ) { m_bUnsavedData = true; }
    bool DataUnsavedPrompt( void );

    void SetStatusBarMsg( const QString& str );

    bool SaveCurrentFile( const QString& file );
    bool LoadFile( const QString& file );

    bool ExportTSV( const QString& file, const QList<AutoProbeRenderable*>* pRenderables );
    bool ExportCSV( const QString& file, const QList<AutoProbeRenderable*>* pRenderables );

    bool CreateProc( const QString& file, const QString& args );

    //The three points provided will be written to the save file for the user's reference
    bool SetPlane( const Plane& plane, const Vec3& pt1, const Vec3& pt2, const Vec3& pt3 );
    const Plane& GetPlane( void ) { return m_Plane; }
    bool GetPlanePts( Vec3& pt1, Vec3& pt2, Vec3& pt3 );
    void ClearPlane( void );

private:
    bool WriteHeader( std::ofstream& ofs );
    bool ReadHeader( std::ifstream& ifs, char& cVer );

    void Run_End( void );
    bool ExecPostRunScript( enum END_STATUS status );

public slots:
    void ProbeObj_Property_Updated();
    void sltDataReceived( TN5500_Data ptData );
    void sltListenTimedOut();
    void sltTriggerError( QString str );

private slots:
    void on_actionIndex_triggered();
    void on_actionFile_Details_triggered();
    void on_actionOut_triggered();
    void on_actionIn_triggered();
    void on_actionReset_Zoom_triggered();
    void on_actionImage_TSV_Selected_triggered();
    void on_actionSpreadsheet_CSV_Selected_triggered();
    void on_actionAbort_triggered();
    void on_actionRestart_triggered();
    void on_actionExecute_triggered();
    void on_actionClear_triggered();
    void on_actionSpreadsheet_CSV_triggered();
    void on_actionLock_Editing_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_as_triggered();
    void on_actionSave_triggered();
    void on_actionNew_triggered();
    void on_actionPoint_triggered();
    void on_actionRectangle_triggered();
    void on_actionCurrent_triggered();
    void on_actionSpec_4_triggered();
    void on_actionSpec_3_triggered();
    void on_actionSpec_2_triggered();
    void on_actionSpec_1_triggered();
    void on_actionWireframe_triggered();
    void on_actionAbout_triggered();
    //void on_listView_itemSelectionChanged();
    void on_actionToggle_Console_triggered();
    void deleteSelectedObjects();
};

#endif // MAINWINDOW_H
