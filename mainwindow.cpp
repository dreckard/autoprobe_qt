/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
//Needed for GetConsoleWindow()
#if _WIN32_WINNT < 0x0500
#define _WIN32_WINNT 0x0500
#endif

#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <deque>
#include <QList>
#include <QKeyEvent>
#include <QComboBox>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressBar>
#include <QShortcut>
#include <QMetaObject>
#include <QDesktopServices>
#include <QUrl>
#include "mainwindow.h"
#include "ListenThread.h"
#include "WriteThread.h"
#include "ui_mainwindow.h"
#include "calibratezdlg.h"
#include "optionsdlg.h"
#include "PointData.h"
#include "StageView.h"
#include "AutoProbe_Objects.h"
#include "AutoProbeError.h"
#include "UtilThread.h"
#include "Utility.h"

MainWindow* MainWindow::m_pInstance = NULL;
MainWindow& MainWindow::Instance( void )
{
    if ( !m_pInstance )
        m_pInstance = new MainWindow;
    return *m_pInstance;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_bDevMode( false ), m_tTimeElapsed( 0 ), m_tRunStartTime( 0 ),
    m_pPointsTableModel( NULL ), m_cRenderMode( RENDERMODE_WIREFRAME ), m_bEditLocked( false ), m_bUnsavedData( false ),
    m_bRun_Active( false ), m_iRun_CurrentRenderable( 0 ), m_iCurrentPoints( 0 ), m_iLinePtsRemaining( 0 ), m_iZeroPts( 0 ),
    m_iLastRow( 0 ), m_iLastCol( 0 )
{
    ui->setupUi(this);

    //Extra UI elements
    //ui->statusBar->setLayout( new QBoxLayout( QBoxLayout::RightToLeft, ui->statusBar ) );
    ui->statusBar->setStyleSheet( "QStatusBar { border-top: 1px solid darkGray; }" );
    m_pRun_ProgressBar = new QProgressBar( ui->statusBar );
    m_pRun_StatusLabel = new QLabel( "", ui->statusBar );
    QFont labelFont = m_pRun_StatusLabel->font(); labelFont.setBold( true );
    m_pRun_StatusLabel->setFont( labelFont );
    m_pRun_ProgressBar->setTextVisible( true );
    m_pRun_ProgressBar->setRange( 0, 100 );
    m_pRun_ProgressBar->setValue( 0 );
    m_pRun_ProgressBar->setVisible( false );
    m_pRun_StatusLabel->setVisible( false );
    ui->statusBar->addPermanentWidget( m_pRun_ProgressBar );
    ui->statusBar->addPermanentWidget( m_pRun_StatusLabel );

    //Dialogs
    m_pCalibZDlg = new CalibrateZDlg( this );
    QObject::connect( ui->actionCalib_Z, SIGNAL(triggered()), m_pCalibZDlg, SLOT(show()) );
    m_pOptionsDlg = new OptionsDlg( this );
    QObject::connect( ui->actionOptions, SIGNAL(triggered()), m_pOptionsDlg, SLOT(show()) );
    m_pBCDlg = new BrightnessContrast( this );
    QObject::connect( ui->actionBrightness_Contrast, SIGNAL(triggered()), m_pBCDlg, SLOT(show()) );

    //Bold headers for properties table
    QFont headerFont = ui->tableWidget->font();
    headerFont.setBold( true );
    ui->tableWidget->verticalHeader()->setFont( headerFont );

    QObject::connect( ui->tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(ProbeObj_Property_Updated()));

    //Setup shortcut so delete key can be used to remove objects
    QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), ui->listView);
    connect(shortcut, SIGNAL(activated()), this, SLOT(deleteSelectedObjects()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pCalibZDlg;
    delete m_pOptionsDlg;
    delete m_pPointsTableModel;
    delete m_pListenThread;
    delete m_pWriteThread;
}

void MainWindow::show( void )
{
    SetupPointsTable();
    SetupListView();
    SetRenderBounds( RenderBounds( MainWindow::Instance().m_Settings.m_flStageLimits[0][0],
                                   MainWindow::Instance().m_Settings.m_flStageLimits[0][1],
                                   MainWindow::Instance().m_Settings.m_flStageLimits[1][0],
                                   MainWindow::Instance().m_Settings.m_flStageLimits[1][1] ) );

    QMainWindow::show();
}

void MainWindow::closeEvent( QCloseEvent* event )
{
    if ( Run_Active() )
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle( "Quit" );
        msgBox.setText( "A run is active.\nReally quit?" );
        msgBox.setIcon( QMessageBox::Question );
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
        msgBox.setDefaultButton( QMessageBox::No );
        msgBox.setEscapeButton( QMessageBox::No );
        msgBox.exec();
        if ( msgBox.clickedButton() != msgBox.button( QMessageBox::Yes ) )
        {
            event->ignore();
            return;
        }
    }
    if ( DataUnsaved() )
    {
        if ( !DataUnsavedPrompt() )
        {
            event->ignore();
            return;
        }

        /*QMessageBox msgBox;
        msgBox.setWindowTitle( "Quit" );
        msgBox.setText( "The file has been modified." );
        msgBox.setIcon( QMessageBox::Question );
        msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
        msgBox.setDefaultButton( QMessageBox::Save );
        msgBox.setEscapeButton( QMessageBox::Cancel );
        int iRet = msgBox.exec();
        switch ( iRet )
        {
            case QMessageBox::Cancel:
                event->ignore();
                return;
            case QMessageBox::Save:
                on_actionSave_triggered();
                if ( m_lastFilename.isEmpty() ) //Bad way to check for cancel but it does work
                {
                    event->ignore();
                    return;
                }
            case QMessageBox::Discard:
                break;
        }*/
    }

    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //if ( event->key() == Qt::Key_F3 )
       // on_actionToggle_Console_triggered();
    //if ( event->key() == Qt::Key_Plus || event->key() == Qt::Key_Minus )
    //    ui->StageViewWidget->TryZoom( event->key() == Qt::Key_Plus );
        //ui->StageViewWidget->HandleKeyEvent( event );
    QWidget::keyPressEvent( event );
}

void MainWindow::SetupPointsTable( void )
{
    m_pPointsTableModel = new PointsTableModel;
    ui->tableView->setModel( m_pPointsTableModel );

    for ( int i = 4; i < NUM_POINT_COLS; i++ )
        ui->tableView->resizeColumnToContents( i );
    //50 px for points, x, y, z - fits pretty well
    for ( int i = 0; i < 4; i++ )
        ui->tableView->setColumnWidth( i, 50 );
}

void MainWindow::SetupListView( void )
{
    ui->listView->setModel( new ObjectsListModel );
}

void MainWindow::ListViewSelectionUpdated( QModelIndexList& qSelection )
{
    if ( qSelection.count() <= 1 )
    {
        ui->tableWidget->clear();
        ui->tableWidget->setRowCount( 0 );

        if ( qSelection.count() == 1 )
        {
            //FIXME: Drag and drop causes the address of the QListWidgetItem to change which breaks things
            //AutoProbeRenderable* pRenderable = ui->StageViewWidget->RenderableByWItem( qSelection.at(0) );
            //AutoProbeRenderable* pRenderable = static_cast<QListWidget_AutoProbeObj*>(qSelection.at(0))->pRenderable;
            AutoProbeRenderable* pRenderable = GetRenderables()->at(qSelection.at(0).row());
            if ( pRenderable )
            {
                ui->tableWidget->hide();
                ui->tableWidget->blockSignals( true ); //Don't want cellchanged emitted over and over
                pRenderable->SetupPropertyTable( ui->tableWidget );
                ui->tableWidget->blockSignals( false );
                ui->tableWidget->show();
            }
        }
    }

    //Brightness/contrast needs to know what's going on here
    if ( qSelection.empty() )
    {
        //if ( GetRenderables()->empty() )
            m_pBCDlg->SelectionChanged( NULL );
        //else
        //    m_pBCDlg->SelectionChanged(GetRenderables()->at(0));
    }
    else
        m_pBCDlg->SelectionChanged(GetRenderables()->at(qSelection.at(0).row()));

    UpdateStageView();
    CheckEnableUIElements();
}

//God only knows why this is required for anything to take effect
void MainWindow::resetListView( void )
{
    ui->listView->reset();
}
void MainWindow::UpdateStageView( void )
{
    ui->StageViewWidget->update();
}
void MainWindow::resetTableView( void )
{
    ui->tableView->reset();
    ui->tableView->update();
}

void MainWindow::CheckEnableUIElements( void )
{
    bool bEnable = !EditLocked();
    ui->tableWidget->setEnabled( bEnable && (GetSelectedRenderables().count() <= 1) );
    ui->menuExport_Selected->setEnabled( (GetSelectedRenderables().count() > 0) );
}

bool MainWindow::InitSerial( short sComPort )
{
    HANDLE hConOut = NULL;

    //This has to be done the long way for a proper handle even with debugger attached
    #ifdef WIN32
        hConOut = CreateFile( L"CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL );
        if ( hConOut == INVALID_HANDLE_VALUE )
            hConOut = NULL;
    #endif

    if ( clib_init( sComPort, "AutoProbe.log", hConOut ) )
    {
        ui->actionExecute->setEnabled( true );
        ui->actionRestart->setEnabled( true );
        return true;
    }
    else
    {
        ui->actionExecute->setEnabled( false );
        ui->actionRestart->setEnabled( false );
        return false;
    }
}

const QList<AutoProbeRenderable*>* MainWindow::GetRenderables( void )
{
    AP_DBGASSERT( ui->StageViewWidget );
    return &(ui->StageViewWidget->GetRenderables());
}

QList<AutoProbeRenderable*> MainWindow::GetSelectedRenderables( void )
{
    QList<AutoProbeRenderable*> rList;
    rList.clear();
    QModelIndexList iList = ui->listView->selectionModel()->selectedRows();
    for ( int i = 0; i < iList.count(); i++ )
        rList.append( GetRenderables()->at(iList.at(i).row()) );
    return rList;
}

bool MainWindow::AddRenderable( AutoProbeRenderable* pRenderable )
{
    if ( GetTotalPoints() + pRenderable->NumPoints() > m_Settings.m_iMaxPts )
    {
        ap_error( "Max points exceeded" );
        return false;
    }

    ui->StageViewWidget->AddRenderable( pRenderable );
    resetListView();
    m_pPointsTableModel->ResetAll();
    ui->tableView->update();
    return true;
}

bool MainWindow::RemoveRenderable( AutoProbeRenderable *pRenderable )
{
    if ( ui->StageViewWidget->RemoveRenderable( pRenderable ) )
    {
        resetListView();
        m_pPointsTableModel->ResetAll();
        ui->tableView->update();
        return true;
    }
    return false;
}

void MainWindow::RemoveAllRenderables( void )
{
    ui->StageViewWidget->RemoveAllRenderables();
    resetListView();
    m_pPointsTableModel->ResetAll();
    ui->tableView->update();
}

unsigned int MainWindow::GetTotalPoints( void )
{
    unsigned int iTotal = 0;
    const QList<AutoProbeRenderable*>* pRenderables = GetRenderables();
    if ( pRenderables )
    {
        for ( int i = 0; i < pRenderables->count(); i++ )
            iTotal += pRenderables->at(i)->NumPoints();
    }
    return iTotal;
}

bool MainWindow::Run_Start( void )
{
    const QList<AutoProbeRenderable*>* pRenderables = GetRenderables();
    if ( pRenderables->empty() )
    {
        ap_error( "Nothing to run!" );
        return false;
    }
    AP_DBGASSERT( Run_CurrentRenderable() >= 0 ); //O_o

    //Recount pts
    Run_CurrentPoint_Overall( true );

    //Must reset these to recheck everything from the beginning
    m_iLastRow = 0;
    m_iLastCol = 0;

    LScan scan;
    if ( !Run_GetNextLScan( scan ) )
    {
        Run_End();
        return false;
    }

    //Must have a filename so autosave will work
    if ( m_Settings.m_sAutoSaveMode != AUTOSAVE_OFF && m_lastFilename.isEmpty() )
    {
        on_actionSave_as_triggered();
        if ( m_lastFilename.isEmpty() ) //If they click cancel just abort the run
            return false;
    }

    //Don't tolerate a zero on the first point (happens sometimes when slow to update after unblank)
    m_iZeroPts = NUM_ZERO_PTS-1;

    LockEdit(); //Automatically turn on edit lock
    Run_LimitedUI( true );

    Run_UpdateProgress();
    m_pRun_ProgressBar->setVisible( true );
    m_pRun_StatusLabel->setVisible( true );

    m_bRun_Active = true;
    m_tRunStartTime = time(NULL);
    
    QMetaObject::invokeMethod( m_pWriteThread, "ScanLine", Qt::QueuedConnection,
                               Q_ARG(LScan,scan), Q_ARG(int,m_Settings.m_iFTLineScanBlock),
                               Q_ARG(float,m_Settings.m_flFTTimeout) );
    m_iLinePtsRemaining = scan.NumPoints();

    //QMetaObject::invokeMethod( m_pListenThread, "SetActive", Qt::QueuedConnection,
    //                           Q_ARG(bool,true), Q_ARG(float,m_Settings.m_flFTTimeout) );

    return true;
}

void MainWindow::Run_End( void )
{
    //QMetaObject::invokeMethod( m_pListenThread, "SetActive", Qt::BlockingQueuedConnection,
    //                           Q_ARG(bool,false), Q_ARG(float,0) );


    QMetaObject::invokeMethod( m_pWriteThread, "EndRun", Qt::QueuedConnection,
                               Q_ARG(bool,m_Settings.m_bBlankBeamOnFinish) );
    /*if ( m_Settings.m_bBlankBeamOnFinish )
    {
        QMetaObject::invokeMethod( m_pWriteThread, "BlankBeam", Qt::QueuedConnection,
                                   Q_ARG(bool,true) );
    }*/

    m_pRun_ProgressBar->setVisible( false );
    m_pRun_StatusLabel->setVisible( false );
    m_bRun_Active = false;
    m_tTimeElapsed += (time(NULL) - m_tRunStartTime);

    Run_LimitedUI( false );
}

void MainWindow::Run_Abort( enum END_STATUS status )
{
    //QMetaObject::invokeMethod( m_pWriteThread, "AbortExec", Qt::QueuedConnection );
    Run_End();

    if ( m_Settings.m_bExecPostScript )
    {
        if ( !ExecPostRunScript( status ) )
            ap_error( "Failed to execute post exec script\n", true );
    }
}

bool MainWindow::ExecPostRunScript( END_STATUS status )
{
    #ifdef WIN32
        if ( !CreateProc( QString(), "cscript.exe post_run.js " + END_STATUS_String( status ) ) )
        {
             ap_error( AsString(GetLastError()), false );
             return false;
        }
        return true;
    #endif
    return false;
}

bool MainWindow::CreateProc( const QString& cmd, const QString& args )
{
    #ifdef WIN32
        STARTUPINFO         siStartupInfo;
        PROCESS_INFORMATION piProcessInfo;

        memset(&siStartupInfo, 0, sizeof(siStartupInfo));
        memset(&piProcessInfo, 0, sizeof(piProcessInfo));

        siStartupInfo.cb = sizeof(siStartupInfo);

        int clen = cmd.length();
        wchar_t* wcmd = NULL;
        if ( clen > 0 )
        {
            wcmd = new wchar_t[clen+1];
            cmd.toWCharArray( wcmd );
            wcmd[clen] = 0;
        }

        int alen = args.length();
        wchar_t* wargs = NULL;
        if ( alen > 0 )
        {
            wargs = new wchar_t[alen+1];
            args.toWCharArray( wargs );
            wargs[alen] = 0;
        }

        bool bRet = CreateProcess( NULL, //(WCHAR*)(L"cscript"),     // Application name
                         wargs, //(WCHAR*)(L"post_run.js testarg"),                 // Application arguments
                         0,
                         0,
                         FALSE,
                         CREATE_DEFAULT_ERROR_MODE,
                         0,
                         0,                              // Working directory
                         &siStartupInfo,
                         &piProcessInfo);
        delete wcmd;
        delete wargs;
        return bRet;
    #endif

    return false;
}

unsigned int MainWindow::Run_CurrentPoint_Overall( bool bRecount /*=false*/ )
{
    const QList<AutoProbeRenderable*>* pRenderables = GetRenderables();
    if ( pRenderables->empty() )
        return 0;
    AP_DBGASSERT( Run_CurrentRenderable() >= 0 ); //O_o

    if ( bRecount )
    {
        m_iCurrentPoints = 0;
        for ( int i = 0; i < Run_CurrentRenderable(); i++ )
            m_iCurrentPoints += pRenderables->at(i)->NumPoints();
        const Matrix2d<point_s>& matrix = pRenderables->at(Run_CurrentRenderable())->GetPointMatrix();
        for ( unsigned int i = 0; i < matrix.count(); i++ ) //No assumptions about collection sequence
        {
            if ( matrix.idx(i).bCollected )
                m_iCurrentPoints++;
        }
    }
    return m_iCurrentPoints;
}

void MainWindow::Run_UpdateProgress( void )
{
    m_pRun_ProgressBar->setValue( (Run_CurrentPoint_Overall()*100) / GetTotalPoints() );

    std::stringstream s;
    s << "Point " << Run_CurrentPoint_Overall()+1 << '/' << GetTotalPoints();
    m_pRun_StatusLabel->setText( s.str().c_str() );
}

void MainWindow::Run_LimitedUI( bool bSetLimited )
{
    ui->actionNew->setEnabled( !bSetLimited );
    ui->actionOpen->setEnabled( !bSetLimited );
    ui->actionExecute->setEnabled( !bSetLimited );
    ui->actionClear->setEnabled( !bSetLimited );
    ui->actionAbort->setEnabled( bSetLimited );
    ui->actionRestart->setEnabled( !bSetLimited );
    ui->actionCalib_Z->setEnabled( !bSetLimited );
    ui->actionLock_Editing->setEnabled( !bSetLimited );
    ui->actionOptions->setEnabled( !bSetLimited );
    ui->menuInsert->setEnabled( !bSetLimited );
}

bool MainWindow::Run_GetNextLScan( LScan& lscan )
{
    const QList<AutoProbeRenderable*>* pRenderables = GetRenderables();
    AutoProbeRenderable* pRenderable = pRenderables->at(Run_CurrentRenderable());
    while ( !pRenderable->GetNextLScan( m_iLastRow, m_iLastCol, lscan ) ) //TODO: Cache last point or something? - Ok
    {
        if ( m_iRun_CurrentRenderable < pRenderables->count()-1 )
            pRenderable = pRenderables->at(++m_iRun_CurrentRenderable);
        else
            return false;
    }
    return true;
}

void MainWindow::sltDataReceived( TN5500_Data ptData )
{
    if ( !Run_Active() )
        return;

    const QList<AutoProbeRenderable*>* pRenderables = GetRenderables();
    AP_DBGASSERT( Run_CurrentRenderable() < pRenderables->count() );

    AutoProbeRenderable* pRenderable = pRenderables->at(Run_CurrentRenderable());
    AP_DBGASSERT( pRenderable );

    //Require a few pts in a row to engage this (some tiny fraction of the time the 5500 gives a zero falsely)
    if ( m_Settings.m_bHaltOnZeroCurrent && TN5500_Data_Current(ptData.sCurrent) < BEAM_MIN )
    {
        if ( ++m_iZeroPts >= NUM_ZERO_PTS )
        {
            //Clear out bad points
            for ( unsigned int i = 0; i < qZeroPts.size(); i++ )
            {
                point_s& pt = *qZeroPts[i];
                pt.bCollected = false;
                pt.flAbsCurrent = 0.0f;
                pt.flSpecCounts[0] = pt.flSpecCounts[1] = pt.flSpecCounts[2] = pt.flSpecCounts[3] = 0.0f;
                resetTableView();
                pRenderable->RegenerateImages(); //This won't work correctly if the points span multiple renderables
            }

            Run_Abort(FAILED_NOCURRENT);

            //Resave with bad points removed
            if ( m_Settings.m_sAutoSaveMode != AUTOSAVE_OFF )
                SaveCurrentFile( m_lastFilename );

            ap_error( "Zero beam current detected, run halted", true );
            return;
        }
    }
    else
    {
        m_iZeroPts = 0;
        qZeroPts.clear();
    }

    Matrix2d<point_s>& matrix = pRenderable->GetPointMatrix();

    AP_RLSASSERT( pRenderable->DataReceived( ptData, m_iLastRow, m_iLastCol ) );

    if ( m_Settings.m_bHaltOnZeroCurrent && TN5500_Data_Current(ptData.sCurrent) < BEAM_MIN )
    {
        qZeroPts.push_back( &matrix.at( m_iLastRow, m_iLastCol ) );
        if ( qZeroPts.size() > NUM_ZERO_PTS )
            qZeroPts.pop_front();
    }

    //Autosave
    if ( m_Settings.m_sAutoSaveMode == AUTOSAVE_POINT )
        SaveCurrentFile( m_lastFilename );

    UpdateStageView();

    MarkDataUnsaved();
    m_pPointsTableModel->MarkPointDirty( matrix.idxbyrowcol(m_iLastRow, m_iLastCol) );

    LScan scan;
    if ( --m_iLinePtsRemaining <= 0 ) //End of current line
    {
        if ( m_Settings.m_sAutoSaveMode == AUTOSAVE_LINE )
            SaveCurrentFile( m_lastFilename );

        if ( !Run_GetNextLScan( scan ) )
        {
            //Completed successfully
            Run_End();

            //TODO: Clean up, there's two identical blocks like this
            if ( m_Settings.m_bExecPostScript )
            {
                if ( !ExecPostRunScript( SUCCEEDED ) )
                    ap_error( "Failed to execute post exec script\n", true );
            }

            return;
        }
        QMetaObject::invokeMethod( m_pWriteThread, "ScanLine", Qt::QueuedConnection,
                                   Q_ARG(LScan,scan), Q_ARG(int,m_Settings.m_iFTLineScanBlock),
                                   Q_ARG(float,m_Settings.m_flFTTimeout) );
        m_iLinePtsRemaining = scan.NumPoints();
    }

    m_iCurrentPoints++;
    Run_UpdateProgress();
}

void MainWindow::sltListenTimedOut( void )
{
    //Autoresume: option in menu, halt if no points collected successfully after resume to avoid infinite loop
    if ( Run_Active() )
    {
        LScan scan;
        if ( m_Settings.m_bAutoresume && Run_GetNextLScan( scan ) )
        {
            ap_logerror( "Timed out waiting for TN-5500, attempting autoresume" );

            QMetaObject::invokeMethod( m_pWriteThread, "RescanLine", Qt::QueuedConnection, Q_ARG(LScan,scan),
                                       Q_ARG(int,m_Settings.m_iFTLineScanBlock), Q_ARG(float,m_Settings.m_flFTTimeout) );
        }
        else
        {
            Run_Abort(FAILED_COMMTIMEOUT);
            ap_error( "Timed out waiting for TN-5500", true );
        }
    }
}

void MainWindow::SetRenderMode( unsigned char cRenderMode )
{
    m_cRenderMode = cRenderMode;
    ui->actionWireframe->setChecked( false );
    ui->actionSpec_1->setChecked( false );
    ui->actionSpec_2->setChecked( false );
    ui->actionSpec_3->setChecked( false );
    ui->actionSpec_4->setChecked( false );
    ui->actionCurrent->setChecked( false );

    switch ( cRenderMode )
    {
    case RENDERMODE_WIREFRAME:
        ui->actionWireframe->setChecked( true );
        break;
    case RENDERMODE_SPEC1:
        ui->actionSpec_1->setChecked( true );
        break;
    case RENDERMODE_SPEC2:
        ui->actionSpec_2->setChecked( true );
        break;
    case RENDERMODE_SPEC3:
        ui->actionSpec_3->setChecked( true );
        break;
    case RENDERMODE_SPEC4:
        ui->actionSpec_4->setChecked( true );
        break;
    case RENDERMODE_CURRENT:
        ui->actionCurrent->setChecked( true );
        break;
    }

    m_pBCDlg->RenderModeChanged( cRenderMode );
    UpdateStageView();
}
void MainWindow::SetRenderBounds( const RenderBounds& bounds )
{
    ui->StageViewWidget->SetRenderBounds( bounds );
}

void MainWindow::SetStatusBarMsg( const QString &str )
{
    ui->statusBar->showMessage( str, 0 );
}

bool MainWindow::WriteHeader( std::ofstream &ofs )
{
    ofs.write( (char*)&g_iMagicNumber, sizeof(g_iMagicNumber) );
    char cVer = FILE_FMT_REV;
    ofs.write( &cVer, 1 );

    time_t tElap = m_tTimeElapsed;
    if ( Run_Active() )
        tElap += time(NULL) - m_tRunStartTime;
    ofs.write( (char*)&tElap, sizeof(tElap) );

    ofs.write( (char*)&m_bEditLocked, sizeof(m_bEditLocked) );
    ofs.write( (char*)&(m_Plane.m_vecNormal), sizeof(m_Plane.m_vecNormal) );
    ofs.write( (char*)&m_planePt1, sizeof(m_planePt1) );
    ofs.write( (char*)&m_planePt2, sizeof(m_planePt2) );
    ofs.write( (char*)&m_planePt3, sizeof(m_planePt3) );

    return !ofs.bad();
}

bool MainWindow::ReadHeader( std::ifstream &ifs, char& cVer )
{
    int iMagicNumber;
    ifs.read( (char*)&iMagicNumber, sizeof(g_iMagicNumber) );
    if ( iMagicNumber != g_iMagicNumber )
    {
        ap_error( "File corrupt or invalid" );
        return false;
    }

    ifs.read( &cVer, 1 );

    if ( cVer < LOWEST_FMT_REV || cVer > FILE_FMT_REV )
    {
        //char szStr[64];
        //snprintf( szStr, 64, "Bad file version\n\nFile: %i\nExpected: %i", cVer, FILE_FMT_REV );
        std::stringstream s;
        s << "Bad file version\n\nFile: " << int(cVer) << "\nExpected: " << int(LOWEST_FMT_REV) << " <= ver <= " << int(FILE_FMT_REV);
        ap_error( s.str() );
        return false;
    }

    if ( cVer >= 5 )
        ifs.read( (char*)&m_tTimeElapsed, sizeof(m_tTimeElapsed) );
    else
        m_tTimeElapsed = 0;

    bool bEditLocked = false;
    ifs.read( (char*)&bEditLocked, sizeof(bEditLocked) );
    LockEdit( bEditLocked );

    ifs.read( (char*)&(m_Plane.m_vecNormal), sizeof(m_Plane.m_vecNormal) );
    ifs.read( (char*)&m_planePt1, sizeof(m_planePt1) );
    ifs.read( (char*)&m_planePt2, sizeof(m_planePt2) );
    ifs.read( (char*)&m_planePt3, sizeof(m_planePt3) );

    m_Plane.m_vecPoint = m_planePt1;
    SetPlane( m_Plane, m_planePt1, m_planePt2, m_planePt3 );

    return !ifs.bad();
}

bool MainWindow::SaveCurrentFile( const QString &file )
{
    std::ofstream ofs( file.toAscii(), std::ofstream::binary | std::ofstream::trunc );
    if ( !ofs.good() )
        return false;

    if ( !WriteHeader( ofs ) )
        return false;

    const QList<AutoProbeRenderable*>* pRenderables = GetRenderables();
    unsigned short iCount = pRenderables->count();
    ofs.write( (char*)(&iCount), sizeof(iCount) );
    for ( int i = 0; i < pRenderables->count(); i++ )
    {
        pRenderables->at(i)->WriteToFile( ofs );
    }

    ofs.close();

    if ( ofs.fail() )
        return false;

    SetLastFileName( file );
    m_bUnsavedData = false;
    return true;
}

bool MainWindow::LoadFile( const QString &file )
{
    std::ifstream ifs( file.toAscii(), std::ifstream::binary );
    if ( !ifs.good() )
        return false;

    ResetAll();

    char cVer;
    if ( !ReadHeader( ifs, cVer ) )
        return false;

    unsigned short iCount = 0;
    ifs.read( (char*)&iCount, sizeof(iCount) );

    for ( unsigned short i = 0; i < iCount; i++ )
    {
        char cType = 0;
        ifs.read( &cType, sizeof(cType) ); //Written by AutoProbeRenderable::WriteToFile()
        Object_Type type = Object_Type(cType);

        AutoProbeRenderable* pRenderable = NULL;
        switch ( type )
        {
        case Obj_Point:
            pRenderable = new AutoProbePoint( ifs, cVer );
            break;
        case Obj_Rect:
            pRenderable = new AutoProbeRect( ifs, cVer );
            break;
        }

        //Great error handling here chief
        if ( ifs.fail() || !( pRenderable && AddRenderable( pRenderable ) ) )
        {
            ResetAll();
            return false;
        }

        //if ( pRenderable )
           // AddRenderable( pRenderable );
    }

    UpdateStageView();

    resetListView();
    //Automatically select the first object if there is one
    QModelIndex idx = ui->listView->model()->index( 0, 0 );
    if ( idx.isValid() )
        ui->listView->selectionModel()->select( idx, QItemSelectionModel::Select );

    m_bUnsavedData = false;
    SetLastFileName( file );
    return true;
}

bool MainWindow::SetPlane( const Plane &plane, const Vec3 &pt1, const Vec3 &pt2, const Vec3 &pt3 )
{
    m_Plane = plane;
    m_planePt1 = pt1;
    m_planePt2 = pt2;
    m_planePt3 = pt3;

    const QList<AutoProbeRenderable*>* pRenderables = GetRenderables();
    for ( int i = 0; i < pRenderables->count(); i++ )
        pRenderables->at(i)->UpdatePointMatrix( !MainWindow::Instance().DevModeEnabled() );
    m_pPointsTableModel->ResetAll();
    resetTableView();

    /*QMessageBox msgBox;
    msgBox.setWindowTitle( "Plane" );
    char szStr[128];
    snprintf( szStr, 128, "vecPoint: %f %f %f\nvecNormal: %f %f %f", plane.m_vecPoint.x,plane.m_vecPoint.y,plane.m_vecPoint.z,
              plane.m_vecNormal.x,plane.m_vecNormal.y,plane.m_vecNormal.z );
    msgBox.setText( szStr );
    msgBox.setIcon( QMessageBox::Information );
    msgBox.exec();*/

    return true;
}
bool MainWindow::GetPlanePts( Vec3& pt1, Vec3& pt2, Vec3& pt3 )
{
    if ( m_Plane.m_vecNormal.IsZero() )
        return false;
    pt1 = m_planePt1; pt2 = m_planePt2; pt3 = m_planePt3;
    return true;
}
void MainWindow::ClearPlane( void )
{
    m_Plane = Plane();
    m_planePt1 = Vec3_Zero;
    m_planePt2 = Vec3_Zero;
    m_planePt3 = Vec3_Zero;
}

void MainWindow::SetLastFileName( const QString &lastFileName )
{
    m_lastFilename = lastFileName;
    if ( lastFileName.isEmpty() )
        setWindowTitle( "AutoProbe" );
    else
        setWindowTitle( lastFileName );
}

void MainWindow::on_actionToggle_Console_triggered()
{
    #ifdef WIN32
        if ( !IsWindow(GetConsoleWindow()) )
            return;

        if ( IsWindowVisible( GetConsoleWindow() ) )
            ShowWindow( GetConsoleWindow(), SW_HIDE );
        else
            ShowWindow( GetConsoleWindow(), SW_SHOWNOACTIVATE );
    #endif
}

void MainWindow::ProbeObj_Property_Updated()
{
    if ( EditLocked() )
        return;

    QList<AutoProbeRenderable*> rList = GetSelectedRenderables();
    if ( rList.empty() )
        return;

    ui->tableWidget->blockSignals( true ); //Renderables may set cell values, but we don't want signals fired for them
    rList.at(0)->PropertyTableUpdated( ui->tableWidget );
    ui->tableWidget->blockSignals( false );

    MarkDataUnsaved();
    UpdateStageView();

    m_pPointsTableModel->ResetAll();
    resetTableView();
}

void MainWindow::on_actionAbout_triggered()
{
    std::stringstream s;
    s <<
        "AutoProbe Version 1.2 - Built at " __TIME__ " on " __DATE__ "\n"
        "Using file format revision " << (int)(FILE_FMT_REV) << "\n\n"

        "This application coordinates a JEOL Superprobe 733 EPMA for large scale mapping by interfacing "
        "with attached Tractor Northern TN-5500 and TN-5600 systems.\n\n"

        "Uses Qt 4.7.0 and UPX 3.07.  Licenses are included in the program folder.\n\n"

        "Developed by Richard Deist (rdeist@ufl.edu)\n"
        "Major Analytical Instrumentation Center\n"
        "University of Florida";

    QMessageBox::about( this, "AutoProbe", s.str().c_str() );
}

void MainWindow::on_actionWireframe_triggered() { SetRenderMode( RENDERMODE_WIREFRAME ); }
void MainWindow::on_actionSpec_1_triggered() { SetRenderMode( RENDERMODE_SPEC1 ); }
void MainWindow::on_actionSpec_2_triggered() { SetRenderMode( RENDERMODE_SPEC2 ); }
void MainWindow::on_actionSpec_3_triggered() { SetRenderMode( RENDERMODE_SPEC3 ); }
void MainWindow::on_actionSpec_4_triggered() { SetRenderMode( RENDERMODE_SPEC4 ); }
void MainWindow::on_actionCurrent_triggered() { SetRenderMode( RENDERMODE_CURRENT ); }

void MainWindow::on_actionRectangle_triggered()
{
    if ( EditLocked() )
    {
        EditLockNotification();
        return;
    }

    AutoProbeRect* pRect = new AutoProbeRect( Float2(0, 0), Float2(10, 10), 1, 1, 0.1, AutoProbeRect::AUTO );
    AddRenderable( pRect );
    MarkDataUnsaved();
}

void MainWindow::on_actionPoint_triggered()
{
    if ( EditLocked() )
    {
        EditLockNotification();
        return;
    }

    AutoProbePoint* pPoint = new AutoProbePoint( Float2(5,5) );
    AddRenderable( pPoint );
    MarkDataUnsaved();
}

void MainWindow::ResetAll( void )
{
    RemoveAllRenderables();
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount( 0 );

    m_pPointsTableModel->ResetAll();

    setWindowTitle( "AutoProbe" );
    m_lastFilename.clear();
    m_bUnsavedData = false;
    m_tTimeElapsed = 0;
    ClearPlane();

    LockEdit( false );
}

void MainWindow::ClearPointData( void )
{
    const QList<AutoProbeRenderable*>* pRenderables = GetRenderables();
    for ( int i = 0; i < pRenderables->count(); i++ )
        pRenderables->at(i)->ClearPointData();
    UpdateStageView();
    m_pPointsTableModel->ResetAll();
}

bool MainWindow::LockEdit( bool bLock /*=true*/ )
{
    if ( bLock )
        m_bEditLocked = true;
    else
    {
        //Situations may exist where the user shouldn't be allowed to lift edit lock
        m_bEditLocked = false;
    }

    CheckEnableUIElements();
    ui->actionLock_Editing->setChecked( m_bEditLocked );
    return m_bEditLocked == bLock;
}

void MainWindow::EditLockNotification( void )
{
    QMessageBox msgBox;
    msgBox.setText( "This action may not be performed with editing locked" );
    msgBox.setIcon( QMessageBox::Warning );
    msgBox.exec();
}

//Returns true if ok to continue, false if cancel clicked
bool MainWindow::DataUnsavedPrompt( void )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( "Close" );
    msgBox.setText( "The file has been modified." );
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
    msgBox.setDefaultButton( QMessageBox::Save );
    msgBox.setEscapeButton( QMessageBox::Cancel );
    int iRet = msgBox.exec();
    switch ( iRet )
    {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Save:
            on_actionSave_triggered();
            if ( m_lastFilename.isEmpty() ) //Bad way to check for cancel but it does work
                return false;
        case QMessageBox::Discard:
            break;
    }
    return true;
}

//Deletes selected objects or points in table depending on which widget has focus
void MainWindow::deleteSelectedObjects()
{
    if ( !ui->listView->hasFocus() && !ui->tableView->hasFocus() )
        return;

    if ( EditLocked() )
    {
        EditLockNotification();
        return;
    }
    if ( ui->listView->hasFocus() )
    {
        QList<AutoProbeRenderable*> list = GetSelectedRenderables();
        int i;
        for ( i = 0; i < list.count(); i++ )
            RemoveRenderable( list.at(i) );
        if ( i > 0 )
        {
            ui->tableWidget->clearSelection();
            ui->tableWidget->setRowCount( 0 );
            ui->tableWidget->clear();

            UpdateStageView();
            MarkDataUnsaved();
        }
    }
    else if ( ui->tableView->hasFocus() )
    {
        QModelIndexList list = ui->tableView->selectionModel()->selectedRows();
        if ( list.empty() )
            return;

        QMessageBox msgBox;
        msgBox.setWindowTitle( "Clear Data" );
        msgBox.setText( "This will clear the selected points and mark them as uncollected.\nContinue?" );
        msgBox.setIcon( QMessageBox::Question );
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
        msgBox.setDefaultButton( QMessageBox::No );
        msgBox.setEscapeButton( QMessageBox::No );
        msgBox.exec();
        if ( msgBox.clickedButton() == msgBox.button( QMessageBox::Yes ) )
        {
            for ( int i = 0; i < list.count(); i++ )
                m_pPointsTableModel->ClearPointByOverallIdx(list.at(i).row());

            const QList<AutoProbeRenderable*>* pRenderables = GetRenderables();
            if ( pRenderables )
            {
                for ( int i = 0; i < pRenderables->count(); i++ )
                    pRenderables->at(i)->RegenerateImages();
            }

            UpdateStageView();
            MarkDataUnsaved();
        }
    }
}

void MainWindow::on_actionNew_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( "New" );
    msgBox.setText( "Clear all? (unsaved data will be lost)" );
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::No );
    msgBox.setEscapeButton( QMessageBox::No );
    msgBox.exec();
    if ( msgBox.clickedButton() == msgBox.button( QMessageBox::Yes ) )
        ResetAll();
}

void MainWindow::on_actionSave_triggered()
{
    if ( m_lastFilename.isEmpty() )
        on_actionSave_as_triggered();
    else
        SaveCurrentFile( m_lastFilename );
}

void MainWindow::on_actionSave_as_triggered()
{
    QFileInfo fInfo( m_lastFilename );
    QString fileName = QFileDialog::getSaveFileName(this,
         "Save File", fInfo.dir().path(), "AutoProbe File (*.aprobe)");

    if ( !fileName.isEmpty() )
        SaveCurrentFile( fileName );
        //SetLastFileName( fileName );
}

void MainWindow::on_actionOpen_triggered()
{
    QFileInfo fInfo( m_lastFilename );
    QString fileName = QFileDialog::getOpenFileName(this,
         "Open File", fInfo.dir().path(), "AutoProbe File (*.aprobe)");

    if ( !fileName.isEmpty() )
    {
        if ( DataUnsaved() && !DataUnsavedPrompt() )
            return;

        LoadFile( fileName );
    }
}

void MainWindow::on_actionLock_Editing_triggered()
{
    if ( EditLocked() )
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle( "Unlock editing" );
        msgBox.setText( "With editing unlocked it is possible to make modifications that will invalidate existing "
                        "data points and cause them to be discarded.\n\nReally unlock?" );
        msgBox.setIcon( QMessageBox::Warning );
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
        msgBox.setDefaultButton( QMessageBox::No );
        msgBox.setEscapeButton( QMessageBox::No );
        msgBox.exec();
        if ( msgBox.clickedButton() != msgBox.button( QMessageBox::Yes ) )
        {
            ui->actionLock_Editing->setChecked( true );
            return;
        }
    }
    if ( !LockEdit( !EditLocked() ) )
    {
        QMessageBox msgBox;
        msgBox.setText( "Failed to unlock edit" );
        msgBox.setIcon( QMessageBox::Information );
        msgBox.exec();
    }
    MarkDataUnsaved();
}

//TODO: Create an image export options dialog or something - filenames & raw counts vs normalized, etc
bool MainWindow::ExportTSV( const QString &baseFile, const QList<AutoProbeRenderable*>* pRenderables )
{
    std::string baseFile_trunc = baseFile.toStdString(); baseFile_trunc.erase( baseFile_trunc.size() - 4, std::string::npos );
    for ( int i = 0; i < pRenderables->count(); i++ )
    {
        const Matrix2d<point_s>& matrix = pRenderables->at(i)->GetPointMatrix();
        for ( unsigned int s = 0; s < NUM_SPECTROMETERS; s++ )
        {
            std::stringstream ss; ss << baseFile_trunc << "_Spec" << s+1 << ".tsv";
            std::ofstream ofs( ss.str().c_str(), std::ofstream::trunc );
            if ( !ofs.good() )
                return false;

            for ( unsigned int j = 0; j < matrix.rows(); j++ )
            {
                for ( unsigned int k = 0; k < matrix.cols(); k++ )
                {
                    if ( matrix.at(j,k).bCollected && matrix.at(j,k).flAbsCurrent > 0.0f )
                        ofs << matrix.at(j,k).flSpecCounts[s] / (pRenderables->at(i)->DwellTime()*matrix.at(j,k).flAbsCurrent) << '\t';
                    else
                        ofs << 0.0f << "\t";
                }
                ofs << '\n';
            }

            ofs.close();
            if ( ofs.fail() )
                return false;
        }

        //Current
        std::ofstream ofs( (baseFile_trunc + "_Cur.tsv").c_str(), std::ofstream::trunc );
        if ( !ofs.good() )
            return false;

        for ( unsigned int j = 0; j < matrix.rows(); j++ )
        {
            for ( unsigned int k = 0; k < matrix.cols(); k++ )
                ofs << matrix.at(j,k).flAbsCurrent << '\t';
            ofs << '\n';
        }

        ofs.close();
        if ( ofs.fail() )
            return false;
    }

    return true;
}

const char cszCSVHeader[] = "Object,Point #,X,Y,Z,Dwell Time (s),Specimen Current (nA),Spec 1 Counts,Spec 2 Counts,Spec 3 Counts,Spec 4 Counts\n";
//Old way
/*const char cszCSVHeader[] = "Point #,X,Y,Z,Specimen Current (nA),Spec 1 Counts,Spec 1 Count Rate (C/s),Spec 1 Count Rate per Current (C/s/nA),"
                            "Spec 2 Counts,Spec 2 Count Rate (C/s),Spec 2 Count Rate per Current (C/s/nA),Spec 3 Counts,Spec 3 Count Rate (C/s),"
                            "Spec 3 Count Rate per Current (C/s/nA),Spec 4 Counts,Spec 4 Count Rate (C/s),Spec 4 Count Rate per Current (C/s/nA)\n";*/
bool MainWindow::ExportCSV( const QString &file, const QList<AutoProbeRenderable*>* pRenderables )
{
    std::ofstream ofs( file.toAscii(), std::ofstream::trunc );
    if ( !ofs.good() )
        return false;

    //CSV Header
    ofs << cszCSVHeader;

    for ( int i = 0; i < pRenderables->count(); i++ )
    {
        const Matrix2d<point_s>& matrix = pRenderables->at(i)->GetPointMatrix();
        for ( unsigned int j = 0; j < matrix.count(); j++ )
        {
            //Messy
            if ( matrix.idx(j).bCollected )
            {
                ofs << pRenderables->at(i)->m_strDisplayName.toStdString() << ',' << j+1
                    << ',' << matrix.idx(j).flPos[0] << ',' << matrix.idx(j).flPos[1] << ',' << matrix.idx(j).flPos[2] << ','
                    << pRenderables->at(i)->DwellTime() << ',' << matrix.idx(j).flAbsCurrent
                    << ',' << matrix.idx(j).flSpecCounts[0] << ',' << matrix.idx(j).flSpecCounts[1]
                    << ',' << matrix.idx(j).flSpecCounts[2] << ',' << matrix.idx(j).flSpecCounts[3]
                    << '\n';

                //The old format conforming to the TN-5500 text output
                /*ofs << "0" << ',' << matrix.idx(j).flPos[0] << ',' << matrix.idx(j).flPos[1] << ',' << matrix.idx(j).flPos[2] << ','
                    << matrix.idx(j).flAbsCurrent << ','
                    << matrix.idx(j).flSpecCounts[0] << ',' << matrix.idx(j).flSpecCounts[0] / pRenderables->at(i)->DwellTime()
                        << ',' << (matrix.idx(j).flSpecCounts[0] / pRenderables->at(i)->DwellTime()) / matrix.idx(j).flAbsCurrent << ','
                    << matrix.idx(j).flSpecCounts[1] << ',' << matrix.idx(j).flSpecCounts[1] / pRenderables->at(i)->DwellTime()
                        << ',' << (matrix.idx(j).flSpecCounts[1] / pRenderables->at(i)->DwellTime()) / matrix.idx(j).flAbsCurrent << ','
                    << matrix.idx(j).flSpecCounts[2] << ',' << matrix.idx(j).flSpecCounts[2] / pRenderables->at(i)->DwellTime()
                        << ',' << (matrix.idx(j).flSpecCounts[2] / pRenderables->at(i)->DwellTime()) / matrix.idx(j).flAbsCurrent << ','
                    << matrix.idx(j).flSpecCounts[3] << ',' << matrix.idx(j).flSpecCounts[3] / pRenderables->at(i)->DwellTime()
                        << ',' << (matrix.idx(j).flSpecCounts[3] / pRenderables->at(i)->DwellTime()) / matrix.idx(j).flAbsCurrent << ','
                    << '\n';*/
            }
        }
    }

    ofs.close();

    return !ofs.fail();
}

void MainWindow::on_actionSpreadsheet_CSV_triggered()
{
    QFileInfo fInfo( m_lastFilename );
    QString fileName = QFileDialog::getSaveFileName(this,
         "Export CSV", fInfo.dir().path(), "Comma separated variable (*.csv)");

    if ( !fileName.isEmpty() )
    {
        if ( !ExportCSV( fileName, GetRenderables() ) )
            ap_error( "Export failed\nEnsure that the file is not in use by another program" );
    }
}

void MainWindow::on_actionSpreadsheet_CSV_Selected_triggered()
{
    QFileInfo fInfo( m_lastFilename );
    QString fileName = QFileDialog::getSaveFileName(this,
         "Export CSV", fInfo.dir().path(), "Comma separated variable (*.csv)");

    if ( !fileName.isEmpty() )
    {
        const QList<AutoProbeRenderable*> renderables = GetSelectedRenderables();
        if ( !ExportCSV( fileName, &renderables ) )
            ap_error( "Export failed\nEnsure that the file is not in use by another program" );
    }
}

void MainWindow::on_actionImage_TSV_Selected_triggered()
{
    QFileInfo fInfo( m_lastFilename );
    QString fileName = QFileDialog::getSaveFileName(this,
         "Export TSV", fInfo.dir().path(), "Tab separated variable (*.tsv)");

    if ( !fileName.isEmpty() )
    {
        const QList<AutoProbeRenderable*> renderables = GetSelectedRenderables();
        for ( int i = 0; i < renderables.count(); i++ )
        {
            UtilThread* pThread = new UtilThread;
            pThread->ExportTSV( fileName.toStdString(), renderables.at(i)->GetPointMatrix(), renderables.at(i)->DwellTime() );
        }
        //if ( !ExportTSV( fileName, &renderables ) )
            //ap_error( "Export failed\nEnsure that the file is not in use by another program" );
    }
}

void MainWindow::on_actionClear_triggered()
{
    if ( EditLocked() )
    {
        EditLockNotification();
        return;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle( "Clear Data" );
    msgBox.setText( "This will clear any point data and mark all points as uncollected.\nContinue?" );
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::No );
    msgBox.setEscapeButton( QMessageBox::No );
    msgBox.exec();
    if ( msgBox.clickedButton() == msgBox.button( QMessageBox::Yes ) )
    {
        ClearPointData();
        m_tTimeElapsed = 0;
        MarkDataUnsaved();
    }
}

void MainWindow::on_actionExecute_triggered()
{
    Run_Start();
}

void MainWindow::on_actionRestart_triggered()
{
    if ( EditLocked() )
    {
        EditLockNotification();
        return;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle( "Clear Data" );
    msgBox.setText( "This will clear any point data and mark all points as uncollected.\nContinue?" );
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::No );
    msgBox.setEscapeButton( QMessageBox::No );
    msgBox.exec();
    if ( msgBox.clickedButton() == msgBox.button( QMessageBox::Yes ) )
    {
        ClearPointData();
        MarkDataUnsaved();
        Run_Start();
    }
}

void MainWindow::on_actionAbort_triggered()
{
    Run_Abort(USER_INTERRUPTED);
}

void MainWindow::on_actionReset_Zoom_triggered()
{
    SetRenderBounds( RenderBounds( MainWindow::Instance().m_Settings.m_flStageLimits[0][0],
                                   MainWindow::Instance().m_Settings.m_flStageLimits[0][1],
                                   MainWindow::Instance().m_Settings.m_flStageLimits[1][0],
                                   MainWindow::Instance().m_Settings.m_flStageLimits[1][1] ) );
    UpdateStageView();
}

void MainWindow::on_actionIn_triggered()
{
    ui->StageViewWidget->TryZoom( true );
}

void MainWindow::on_actionOut_triggered()
{
    ui->StageViewWidget->TryZoom( false );
}

void MainWindow::on_actionFile_Details_triggered()
{
    if ( m_lastFilename.isEmpty() )
    {
        QMessageBox msgBox( QMessageBox::Warning, "File Details", "No file currently open" );
        msgBox.exec();
        return;
    }

    unsigned int iTotal, iCurrent;
    iTotal = GetTotalPoints();
    iCurrent = Run_CurrentPoint_Overall(!Run_Active());

    time_t tTotalElapsed = m_tTimeElapsed;
    if ( Run_Active() )
        tTotalElapsed += (time(NULL)-m_tRunStartTime);

    float flRate = tTotalElapsed > 0 ? float(iCurrent)/float(tTotalElapsed) : 0.0f;
    time_t tTimeRemaining = flRate > 0 ? (iTotal-iCurrent)/flRate : -1;

    using namespace std;
    stringstream s;
    s.setf( ios::right, ios::adjustfield );
    s.fill( '0' );
    s <<
        "File: " << m_lastFilename.toStdString() << endl <<
        "Points: " << iTotal << endl <<
        "   Collected: " << iCurrent;
            if ( iTotal > 0 )
                s << " (" << 100.0f*(float(iCurrent)/float(iTotal)) << "%)";
            s << endl <<
        "   Remaining: " << iTotal-iCurrent << endl;
        s << "Total run time: " << setw(2) << int(tTotalElapsed/3600) << ':' << setw(2) << int((tTotalElapsed/60) % 60) << ':' << setw(2) << int(tTotalElapsed % 60) << endl <<
        "Avg. Rate: " << flRate << " points/sec" << endl <<
        "Approx. time remaining: ";
            if ( tTimeRemaining >= 0 )
                s << setw(2) << int(tTimeRemaining/3600) << ':' << setw(2) << int((tTimeRemaining/60) % 60) << ':' << setw(2) << int((tTimeRemaining) % 60);
            else
                s << "???";
            s << endl;


    QMessageBox::about( this, "File Details", s.str().c_str() );
}

void MainWindow::sltTriggerError( QString str )
{
    ap_error( str.toStdString() );
}

void MainWindow::on_actionIndex_triggered()
{
    QFileInfo info( "help/AutoProbe.html" );
    if ( info.exists() )
    {
        //QUrl url( "file:///e:/Code/Rick/MAIC/AutoProbe/AutoProbe_qt/bin/redist/help/AutoProbe.html" );
        QUrl url( "file:///" + info.absoluteFilePath() );
        UtilThread* pThread = new UtilThread;
        pThread->OpenURL( url );
    }
    else
        ap_error( "Failed to find AutoProbe.html" );
}
