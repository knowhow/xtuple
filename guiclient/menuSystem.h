/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef menuSystem_h
#define menuSystem_h

#include <QObject>
#include <QPixmap>

class QMenu;
class QPixmap;
class QToolBar;
class GUIClient;
class Action;

class menuSystem : public QObject
{
  Q_OBJECT

  struct actionProperties {
    const char*		actionName;
    const QString	actionTitle;
    const char*		slot;
    QMenu*		menu;
    QString		priv;
    QPixmap		pixmap;
    QToolBar*		toolBar;
    bool		visible;
  };

  public:
    menuSystem(GUIClient *);

  public slots:
    void sPrepareWindowMenu();
    void sHideWindowMenu();
    void sActivateWindow();
    void sCloseAll();
    void sCloseActive();
    void sRememberPositionToggle();
    void sRememberSizeToggle();

    void sSetup();

    void sEventManager();
    void sPreferences();
    void sHotKeys();
    void sRescanPrivileges();
    void sMaintainUsers();
    void sMaintainGroups();
    void sNewEmployee();
    void sListEmployees();
    void sSearchEmployees();
    void sEmployeeGroups();
    void sErrorLog();

    void sCustomCommands();
    void sScripts();
    void sUIForms();
    void sPackages();
    void sMetasqls();
    void sReports();

    void sFixACL();
    void sFixSerial();
    void sExportData();
    void sImportData();
    void sCSVAtlases();

    void sPrintAlignment();

    void sExit();

    void sAbout();
    void sTOC();

    void sCommunityHome();
    void sCommunityEditAccount();
    void sCommunityForums();
    void sCommunityBlogs();
    void sCommunityIssues();
    void sCommunityWiki();
    void sCommunityDownloads();
    void sRegister();
    void sCommunitySupport();
    void sCommunityTranslation();
    void sCommunityXchange();

    void sCheckForUpdates();

  private:
    GUIClient *parent;

    QToolBar *toolBar;

    QMenu *communityMenu;
    QMenu *designMenu;
    QMenu *employeeMenu;
    QMenu *helpMenu;
    QMenu *masterInfoMenu;
    QMenu *sysUtilsMenu;
    QMenu *systemMenu;
    QMenu *windowMenu;

    QMenu *geometryMenu;

    Action *cascade;
    Action *tile;
    Action *closeActive;
    Action *closeAll;
    Action *_rememberPos;
    Action *_rememberSize;

    QWidget *_lastActive;

    void	addActionsToMenu(actionProperties [], unsigned int);
    bool        loadCSVPlugin();

};

#endif
