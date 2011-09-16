/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef GUICLIENT_H
#define GUICLIENT_H

#include <QDate>
#include <QDateTime>
#include <QMainWindow>
#include <QTimer>
#include <QAction>
#include <QCloseEvent>
#include <QFileSystemWatcher>
#include <QList>
#include <QPixmap>
#include <QMenu>
#include <QMenuBar>

#include <xsqlquery.h>

#include "../common/format.h"
#include "../hunspell/hunspell.hxx"

class QSplashScreen;
class QWorkspace;
class QPushButton;
class QIntValidator;
class QDoubleValidator;
class QCheckBox;
class QScriptEngine;

class menuProducts;
class menuInventory;
class menuSchedule;
class menuPurchase;
class menuManufacture;
class menuCRM;
class menuSales;
class menuAccounting;
class menuSystem;

class TimeoutHandler;
class InputManager;
class ReportHandler;

class XMainWindow;
class QMainWindow;
class XWidget;

#define q omfgThis->_q

#define cNew                  1
#define cEdit                 2
#define cView                 3
#define cCopy                 4
#define cRelease              5
#define cPost                 6
#define cReplace              7

#define cPostedCounts         0x01
#define cUnpostedCounts       0x02
#define cAllCounts            0x04

//  Possible Location/Itemloc Sources
#define cLocation             0x01
#define cItemloc              0x02

//  Possible Transactions Groups
#define cTransAll             0xFF
#define cTransReceipts        0x01
#define cTransIssues          0x02
#define cTransShipments       0x04
#define cTransAdjCounts       0x08
#define cTransTransfers       0x10
#define cTransScraps          0x20

//  Possible Journal Types
#define SalesJournal          0x01
#define CreditMemoJournal     0x02
#define CashReceiptsJournal   0x03
#define PayablesJournal       0x04
#define CheckJournal          0x05

// Possible return values from submitReport
#define cNoReportDefinition   -2

int  systemError(QWidget *, const QString &);
int  systemError(QWidget *, const QString &, const QString &, const int);
void message(const QString &, int = 0);
void resetMessage();
void audioAccept();
void audioReject();
QString translationFile(const QString localestr, const QString component);
QString translationFile(const QString localestr, const QString component, QString &version);

extern bool _evaluation;

extern QSplashScreen *_splash;

#include "../common/metrics.h"
extern Metrics     *_metrics;
extern Preferences *_preferences;
extern Privileges  *_privileges;
#include "../common/metricsenc.h"
extern Metricsenc  *_metricsenc;


enum SetResponse
{
  NoError, NoError_Cancel, NoError_Run, NoError_Print, NoError_Submit,
  Error_NoSetup, UndefinedError
};


class Action : public QAction
{
  public:
    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, bool );

    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, bool,
            const QPixmap &, QWidget *);  
            
    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, bool,
            const QPixmap &, QWidget *,
            const QString &); 

    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, const QString & );

    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, const QString &,
            const QPixmap &, QWidget *);  
            
    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, const QString &,
            const QPixmap &, QWidget *,
            const QString &); 

  private:
    void init( QWidget *, const char *, const QString &,
               QObject *, const char *,
               QWidget *, const QString & );
};

class GUIClient : public QMainWindow
{
  friend class XWidget;
  friend class XMainWindow;
  friend class XDialog;
  friend class xTupleGuiClientInterface;

  Q_OBJECT

  Q_PROPERTY(QString key READ key)
  
  public:
    enum WindowSystem {
      Unknown, X11, WIN, MAC, QWS, WINCE, S60
    };

    GUIClient(const QString &, const QString &);
    virtual ~GUIClient();

    Q_INVOKABLE void setCaption();
    Q_INVOKABLE void initMenuBar();
    Q_INVOKABLE void saveToolbarPositions();

    Q_INVOKABLE inline QWorkspace *workspace()         { return _workspace;    }
    Q_INVOKABLE inline InputManager *inputManager()    { return _inputManager; }
    Q_INVOKABLE inline QString databaseURL()           { return _databaseURL;  }
    Q_INVOKABLE inline QString username()              { return _username;     }

    Q_INVOKABLE inline const QDate startOfTime()       { return _startOfTime;  }
    Q_INVOKABLE inline const QDate endOfTime()         { return _endOfTime;    }
    Q_INVOKABLE inline const QDate dbDate()            { return _dbDate;       }

    Q_INVOKABLE inline QDoubleValidator *qtyVal()      { return _qtyVal;       }
    Q_INVOKABLE inline QDoubleValidator *transQtyVal() { return _transQtyVal;  }
    Q_INVOKABLE inline QDoubleValidator *qtyPerVal()   { return _qtyPerVal;    }
    Q_INVOKABLE inline QDoubleValidator *scrapVal()    { return _scrapVal;     } 
    Q_INVOKABLE inline QDoubleValidator *percentVal()  { return _percentVal;   }
    Q_INVOKABLE inline QDoubleValidator *negPercentVal()  { return _negPercentVal;   }
    Q_INVOKABLE inline QDoubleValidator *moneyVal()    { return _moneyVal;     }
    Q_INVOKABLE inline QDoubleValidator *negMoneyVal() { return _negMoneyVal;  }
    Q_INVOKABLE inline QDoubleValidator *priceVal()    { return _priceVal;     }
    Q_INVOKABLE inline QDoubleValidator *costVal()     { return _costVal;      }
    Q_INVOKABLE inline QDoubleValidator *ratioVal()    { return _ratioVal;     }
    Q_INVOKABLE inline QDoubleValidator *weightVal()   { return _weightVal;    }
    Q_INVOKABLE inline QDoubleValidator *runTimeVal()  { return _runTimeVal;   }
    Q_INVOKABLE inline QIntValidator *orderVal()       { return _orderVal;     }
    Q_INVOKABLE inline QIntValidator *dayVal()         { return _dayVal;       }

    Q_INVOKABLE inline QFont systemFont()              { return *_systemFont;  }
    Q_INVOKABLE inline QFont fixedFont()               { return *_fixedFont;   }
    Q_INVOKABLE GUIClient::WindowSystem getWindowSystem();

    Q_INVOKABLE bool singleCurrency();
    Q_INVOKABLE bool showTopLevel() const { return _showTopLevel; }
    Q_INVOKABLE QWidgetList windowList();
    Q_INVOKABLE void populateCustomMenu(QMenu*, const QString &);

    Q_INVOKABLE void handleNewWindow(QWidget *, Qt::WindowModality = Qt::NonModal);
    Q_INVOKABLE QMenuBar *menuBar();

    // Used by scripting
    Q_INVOKABLE void addDockWidget ( Qt::DockWidgetArea area, QDockWidget * dockwidget );
    Q_INVOKABLE void addToolBar ( QToolBar * toolbar );
    Q_INVOKABLE void addToolBar ( Qt::ToolBarArea area, QToolBar * toolbar );
    Q_INVOKABLE void addToolBarBreak ( Qt::ToolBarArea area = Qt::TopToolBarArea );
    Q_INVOKABLE void tabifyDockWidget ( QDockWidget * first, QDockWidget * second );
    Q_INVOKABLE void setCentralWidget(QWidget * widget);

    XSqlQuery        _q;
    XSqlQuery        __item;
    int              __itemListSerial;
    XSqlQuery        __cust;
    int              __custListSerial;
    TimeoutHandler   *_timeoutHandler;
    ReportHandler    *_reportHandler;

    QMap<const QObject*,int> _customCommands;

    QString _key;
    Q_INVOKABLE QString key() { return _key; }

    QString _company;
    Q_INVOKABLE QString company() { return _company; }

    bool _useCloud;
    Q_INVOKABLE bool useCloud() { return _useCloud; }

    QString _singleWindow;

    Q_INVOKABLE        void  launchBrowser(QWidget*, const QString &);
    Q_INVOKABLE     QWidget *myActiveWindow();
    Q_INVOKABLE inline bool  shuttingDown() { return _shuttingDown; }

    void loadScriptGlobals(QScriptEngine * engine);

    //check hunspell is ready
    Q_INVOKABLE bool hunspell_ready();
    //spellcheck word, returns 1 if word ok otherwise 0
    Q_INVOKABLE int hunspell_check(const QString word);
    //suggest words for word, returns number of words in slst
    Q_INVOKABLE const QStringList hunspell_suggest(const QString word);
    //add word to dict (word is valid until spell object is not destroyed)
    Q_INVOKABLE int hunspell_add(const QString word);
    //add word to dict (word is valid until spell object is not destroyed)
    Q_INVOKABLE int hunspell_ignore(const QString word);

  public slots:
    void sReportError(const QString &);
    void sTick();

    void sAssortmentsUpdated(int, bool);
    void sBBOMsUpdated(int, bool);
    void sBOMsUpdated(int, bool);
    void sBOOsUpdated(int, bool);
    void sBankAccountsUpdated();
    void sBankAdjustmentsUpdated(int, bool);
    void sBillingSelectionUpdated(int, int);
    void sBudgetsUpdated(int, bool);
    void sCashReceiptsUpdated(int, bool);
    void sChecksUpdated(int, int, bool);
    void sConfigureGLUpdated();
    void sCreditMemosUpdated();
    void sCrmAccountsUpdated(int);
    void sCustomCommand();
    void sCustomersUpdated(int, bool);
    void sEmployeeUpdated(int);
    void sGlSeriesUpdated();
    void sInvoicesUpdated(int, bool);
    void sItemGroupsUpdated(int, bool);
    void sItemsUpdated(int, bool);
    void sItemsitesUpdated();
    void sPaymentsUpdated(int, int, bool);
    void sProjectsUpdated(int);
    void sProspectsUpdated();
    void sPurchaseOrderReceiptsUpdated();
    void sPurchaseOrdersUpdated(int, bool);
    void sPurchaseRequestsUpdated();
    void sQOHChanged(int, bool);
    void sQuotesUpdated(int);
    void sReportsChanged(int, bool);
    void sReturnAuthorizationsUpdated();
    void sSalesOrdersUpdated(int);
    void sSalesRepUpdated(int);
    void sStandardPeriodsUpdated();
    void sTaxAuthsUpdated(int);
    void sTransferOrdersUpdated(int);
    void sUserUpdated(QString);
    void sVendorsUpdated();
    void sVouchersUpdated();
    void sWarehousesUpdated();
    void sWorkCentersUpdated();
    void sWorkOrderMaterialsUpdated(int, int, bool);
    void sWorkOrderOperationsUpdated(int, int, bool);
    void sWorkOrdersUpdated(int, bool);

    void sIdleTimeout();

    void sFocusChanged(QWidget* old, QWidget* now);

    void sClearErrorMessages();
    void sNewErrorMessage();
    void setWindowTitle();

  signals:
    void tick();

    void assortmentsUpdated(int, bool);
    void bankAccountsUpdated();
    void bankAdjustmentsUpdated(int, bool);
    void bbomsUpdated(int, bool);
    void billingSelectionUpdated(int, int);
    void bomsUpdated(int, bool);
    void boosUpdated(int, bool);
    void budgetsUpdated(int, bool);
    void cashReceiptsUpdated(int, bool);
    void checksUpdated(int, int, bool);
    void configureGLUpdated();
    void creditMemosUpdated();
    void crmAccountsUpdated(int);
    void customersUpdated(int, bool);
    void employeeUpdated(int);
    void glSeriesUpdated();
    void invoicesUpdated(int, bool);
    void itemGroupsUpdated(int, bool);
    void itemsUpdated(int, bool);
    void itemsitesUpdated();
    void paymentsUpdated(int, int, bool);
    void projectsUpdated(int);
    void prospectsUpdated();
    void purchaseOrderReceiptsUpdated();
    void purchaseOrdersUpdated(int, bool);
    void purchaseRequestsUpdated();
    void qohChanged(int, bool);
    void quotesUpdated(int, bool);
    void reportsChanged(int, bool);
    void returnAuthorizationsUpdated();
    void salesOrdersUpdated(int, bool);
    void salesRepUpdated(int);
    void standardPeriodsUpdated();
    void taxAuthsUpdated(int);
    void transferOrdersUpdated(int);
    void userUpdated(QString);
    void vendorsUpdated();
    void vouchersUpdated();
    void warehousesUpdated();
    void workCentersUpdated();
    void workOrderMaterialsUpdated(int, int, bool);
    void workOrderOperationsUpdated(int, int, bool);
    void workOrdersUpdated(int, bool);

  protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);

    void addDocumentWatch(QString path, int id);
    bool removeDocumentWatch(QString path);

  protected slots:
    void windowDestroyed(QObject*);

  private slots:
    void handleDocument(QString path);
    void hunspell_initialize();
    void hunspell_uninitialize();

  private:
    QWorkspace   *_workspace;
    QTimer       _tick;
    QPushButton  *_eventButton;
    QPushButton  *_registerButton;
    QPushButton  *_errorButton;
    QString      _databaseURL;
    QString      _username;
    bool         _showTopLevel;
    QWidgetList  _windowList;
    QMenuBar	*_menuBar;
    QWidget     *_activeWindow;

    InputManager   *_inputManager;

    menuProducts    *productsMenu;
    menuInventory   *inventoryMenu;
    menuSchedule    *scheduleMenu;
    menuPurchase    *purchaseMenu;
    menuManufacture *manufactureMenu;
    menuCRM         *crmMenu;
    menuSales       *salesMenu;
    menuAccounting  *accountingMenu;
    menuSystem      *systemMenu;

    QDate _startOfTime;
    QDate _endOfTime;
    QDate _dbDate;

    QDoubleValidator *_qtyVal;
    QDoubleValidator *_transQtyVal;
    QDoubleValidator *_qtyPerVal;
    QDoubleValidator *_scrapVal;
    QDoubleValidator *_percentVal;
    QDoubleValidator *_negPercentVal;
    QDoubleValidator *_moneyVal;
    QDoubleValidator *_negMoneyVal;
    QDoubleValidator *_priceVal;
    QDoubleValidator *_costVal;
    QDoubleValidator *_ratioVal;
    QDoubleValidator *_weightVal;
    QDoubleValidator *_runTimeVal;
    QIntValidator    *_orderVal;
    QIntValidator    *_dayVal;

    QFont *_systemFont;
    QFont *_fixedFont;

    bool _shown;
    bool _shuttingDown;

    QFileSystemWatcher* _fileWatcher;
    QMap<QString, int> _fileMap;
    QTextCodec * _spellCodec;
    Hunspell * _spellChecker;
    bool _spellReady;
    QStringList _spellAddWords;
};
extern GUIClient *omfgThis;

#endif

