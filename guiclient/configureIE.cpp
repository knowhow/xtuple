/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureIE.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"
#include "atlasMap.h"
#include "xsltMap.h"

bool configureIE::userHasPriv()
{
  return _privileges->check("ConfigureImportExport");
}

configureIE::configureIE(QWidget* parent, const char* name, bool /*modal*/, Qt::WFlags fl)
    : XAbstractConfigure(parent, fl)
{
  setupUi(this);

  if (name)
    setObjectName(name);

  connect(_atlasMap, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(sEditAtlasMap()));
  connect(_atlasMap,   SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_deleteAtlasMap, SIGNAL(clicked()), this, SLOT(sDeleteAtlasMap()));
  connect(_deleteMap,      SIGNAL(clicked()), this, SLOT(sDeleteMap()));
  connect(_editAtlasMap,   SIGNAL(clicked()), this, SLOT(sEditAtlasMap()));
  connect(_editMap,        SIGNAL(clicked()), this, SLOT(sEditMap()));
  connect(_map,      SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(sEditMap()));
  connect(_map,        SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_newAtlasMap, SIGNAL(clicked()), this, SLOT(sNewAtlasMap()));
  connect(_newMap,      SIGNAL(clicked()), this, SLOT(sNewMap()));

  _map->addColumn(tr("Name"),             -1, Qt::AlignLeft, true, "xsltmap_name");
  _map->addColumn(tr("Document Type"),    -1, Qt::AlignLeft, true, "xsltmap_doctype");
  _map->addColumn(tr("System Identifier"),-1, Qt::AlignLeft, true, "xsltmap_system");
  _map->addColumn(tr("Import XSLT File"), -1, Qt::AlignLeft, true, "xsltmap_import");
  _map->addColumn(tr("Export XSLT File"), -1, Qt::AlignLeft, true, "xsltmap_export");

  _atlasMap->addColumn(tr("Name"),       -1, Qt::AlignLeft, true, "atlasmap_name");
  _atlasMap->addColumn(tr("Type"),       -1, Qt::AlignLeft, true, "atlasmap_filtertype");
  _atlasMap->addColumn(tr("Matches"),    -1, Qt::AlignLeft, true, "atlasmap_filter");
  _atlasMap->addColumn(tr("Atlas File"), -1, Qt::AlignLeft, true, "atlasmap_atlas");
  _atlasMap->addColumn(tr("CSV Map"),    -1, Qt::AlignLeft, true, "atlasmap_map");

  // TODO: fix these when support for an internal XSLT processor is enabled
  _internal->setEnabled(false);
  _internal->setVisible(false);
  _external->setVisible(false);

#ifdef Q_WS_WIN
  _os->setCurrentIndex(1);
#endif
#ifdef Q_WS_MAC
  _os->setCurrentIndex(2);
#endif

  sPopulate();
}

configureIE::~configureIE()
{
  // no need to delete child widgets, Qt does it all for us
}

void configureIE::languageChange()
{
  retranslateUi(this);
}

bool configureIE::sSave()
{
  emit saving();

  if (_importErrorXMLHandling->isChecked() &&
      _importErrorSelector->option() != FileMoveSelector::Suffix &&
      _importErrorSelector->option() != FileMoveSelector::ChangeDir)
  {
    QMessageBox::critical(this, tr("Inconsistent Selection"),
                          tr("<p>Please choose to add a suffix after errors, "
                             "move the import file after errors, or "
                             "uncheck '%1'.")
                          .arg(_importErrorXMLHandling->text()));
    return false;
  }

  _metrics->set("XMLSuccessTreatment",  _importSuccessSelector->code());

  if (_internal->isChecked())
    _metrics->set("XSLTLibrary",          _internal->isChecked());
  else if (_external->isChecked())
  {
    if (_linuxCmd->text().isEmpty() && _macCmd->text().isEmpty() &&
        _windowsCmd->text().isEmpty())
    {
      QMessageBox::critical(this, tr("Incomplete Data"),
                            tr("<p>Please enter the XSLT processor command "
                               "line for at least one platform."));
      _tabs->setCurrentIndex(_tabs->indexOf(_importTab));
      _linuxCmd->setFocus();
      return false;
    }
    else
      _metrics->set("XSLTLibrary",        ! _external->isChecked());
  }
  else
  {
    QMessageBox::critical(this, tr("Incomplete Data"),
                          tr("<p>Please choose whether to use the internal "
                             "XSLT processor or an external XSLT processor."));
    _tabs->setCurrentIndex(_tabs->indexOf(_importTab));
    _internal->setFocus();
    return false;
  }

  _metrics->set("XSLTDefaultDirLinux",    _xsltLinuxDir->text());
  _metrics->set("XSLTDefaultDirMac",      _xsltMacDir->text());
  _metrics->set("XSLTDefaultDirWindows",  _xsltWindowsDir->text());

  _metrics->set("XSLTProcessorLinux",     _linuxCmd->text());
  _metrics->set("XSLTProcessorMac",       _macCmd->text());
  _metrics->set("XSLTProcessorWindows",   _windowsCmd->text());

  _metrics->set("XMLDefaultDirLinux",          _importLinuxDir->text());
  _metrics->set("XMLDefaultDirMac",            _importMacDir->text());
  _metrics->set("XMLDefaultDirWindows",        _importWindowsDir->text());

  _metrics->set("CSVAtlasDefaultDirLinux",      _atlasLinuxDir->text());
  _metrics->set("CSVAtlasDefaultDirMac",        _atlasMacDir->text());
  _metrics->set("CSVAtlasDefaultDirWindows",    _atlasWindowsDir->text());

  _metrics->set("XMLSuccessSuffix",       _importSuccessSelector->suffix());
  _metrics->set("XMLSuccessDir",          _importSuccessSelector->destdir());

  _metrics->set("ImportFailureTreatment", _importErrorSelector->code());
  _metrics->set("ImportFailureSuffix",    _importErrorSelector->suffix());
  _metrics->set("ImportFailureDir",       _importErrorSelector->destdir());

  _metrics->set("ImportXMLCreateErrorFile", _importErrorXMLHandling->isChecked());

  _metrics->set("XMLExportDefaultDirLinux",    _exportLinuxDir->text());
  _metrics->set("XMLExportDefaultDirMac",      _exportMacDir->text());
  _metrics->set("XMLExportDefaultDirWindows",  _exportWindowsDir->text());

  return true;
}

void configureIE::sPopulate()
{
  _xsltLinuxDir->setText(_metrics->value("XSLTDefaultDirLinux"));
  _xsltMacDir->setText(_metrics->value("XSLTDefaultDirMac"));
  _xsltWindowsDir->setText(_metrics->value("XSLTDefaultDirWindows"));

  //  TODO: start using Qt's XSLT processor
  _external->setChecked(TRUE);

  _linuxCmd->setText(_metrics->value("XSLTProcessorLinux"));
  _macCmd->setText(_metrics->value("XSLTProcessorMac"));
  _windowsCmd->setText(_metrics->value("XSLTProcessorWindows"));

  _importLinuxDir->setText(_metrics->value("XMLDefaultDirLinux"));
  _importMacDir->setText(_metrics->value("XMLDefaultDirMac"));
  _importWindowsDir->setText(_metrics->value("XMLDefaultDirWindows"));

  _atlasLinuxDir->setText(_metrics->value("CSVAtlasDefaultDirLinux"));
  _atlasMacDir->setText(_metrics->value("CSVAtlasDefaultDirMac"));
  _atlasWindowsDir->setText(_metrics->value("CSVAtlasDefaultDirWindows"));

  _importSuccessSelector->setCode(_metrics->value("XMLSuccessTreatment"));
  _importSuccessSelector->setDestdir(_metrics->value("XMLSuccessDir"));
  _importSuccessSelector->setSuffix(_metrics->value("XMLSuccessSuffix"));

  _importErrorSelector->setCode(_metrics->value("ImportFailureTreatment"));
  _importErrorSelector->setDestdir(_metrics->value("ImportFailureDir"));
  _importErrorSelector->setSuffix(_metrics->value("ImportFailureSuffix"));

  _importErrorXMLHandling->setChecked(_metrics->boolean("ImportXMLCreateErrorFile"));

  _exportLinuxDir->setText(_metrics->value("XMLExportDefaultDirLinux"));
  _exportMacDir->setText(_metrics->value("XMLExportDefaultDirMac"));
  _exportWindowsDir->setText(_metrics->value("XMLExportDefaultDirWindows"));

  sFillList();
}

void configureIE::sFillList()
{
  XSqlQuery xsltq("SELECT * FROM xsltmap ORDER BY xsltmap_name;");
  xsltq.exec();
  _map->populate(xsltq);
  if (xsltq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, xsltq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery atlasq("SELECT * FROM atlasmap ORDER BY atlasmap_name;");
  atlasq.exec();
  _atlasMap->populate(atlasq);
  if (atlasq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, atlasq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void configureIE::sDeleteAtlasMap()
{
  if (QMessageBox::question(this, tr("Delete Map?"),
                    tr("Are you sure you want to delete this CSV Atlas Map?"),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare("DELETE FROM atlasmap WHERE (atlasmap_id=:mapid);");
    delq.bindValue(":mapid", _atlasMap->id());
    delq.exec();
    if (delq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, delq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void configureIE::sDeleteMap()
{
  if (QMessageBox::question(this, tr("Delete Map?"),
                    tr("Are you sure you want to delete this XSLT Map?"),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare("DELETE FROM xsltmap WHERE (xsltmap_id=:mapid);");
    delq.bindValue(":mapid", _map->id());
    delq.exec();
    if (delq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, delq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void configureIE::sEditAtlasMap()
{
  ParameterList params;
  params.append("mode",        "edit");
  params.append("atlasmap_id", _atlasMap->id());
#if defined Q_WS_MACX
  params.append("defaultDir",  _atlasMacDir->text());
#elif defined Q_WS_WIN
  params.append("defaultDir",  _atlasWindowsDir->text());
#elif defined Q_WS_X11
  params.append("defaultDir",  _atlasLinuxDir->text());
#endif

  atlasMap newdlg(this);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void configureIE::sEditMap()
{
  ParameterList params;
  params.append("mode",       "edit");
  params.append("xsltmap_id", _map->id());
#if defined Q_WS_MACX
  params.append("defaultDir", _atlasMacDir->text());
#elif defined Q_WS_WIN
  params.append("defaultDir", _atlasWindowsDir->text());
#elif defined Q_WS_X11
  params.append("defaultDir", _atlasLinuxDir->text());
#endif

  xsltMap newdlg(this);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void configureIE::sNewAtlasMap()
{
  ParameterList params;
  params.append("mode",       "new");
#if defined Q_WS_MACX
  params.append("defaultDir", _atlasMacDir->text());
#elif defined Q_WS_WIN
  params.append("defaultDir", _atlasWindowsDir->text());
#elif defined Q_WS_X11
  params.append("defaultDir", _atlasLinuxDir->text());
#endif

  atlasMap newdlg(this);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void configureIE::sNewMap()
{
  ParameterList params;
  params.append("mode",       "new");
#if defined Q_WS_MACX
  params.append("defaultDir", _atlasMacDir->text());
#elif defined Q_WS_WIN
  params.append("defaultDir", _atlasWindowsDir->text());
#elif defined Q_WS_X11
  params.append("defaultDir", _atlasLinuxDir->text());
#endif

  xsltMap newdlg(this);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void configureIE::sPopulateMenu(QMenu* pMenu, QTreeWidgetItem* /* pItem */)
{

  QAction *newAct  = pMenu->addAction(tr("New..."));
  QAction *editAct = pMenu->addAction(tr("Edit..."));
  QAction *delAct  = pMenu->addAction(tr("Delete"));

  if (sender() == _map)
  {
    newAct->setEnabled(xsltMap::userHasPriv());
    editAct->setEnabled(xsltMap::userHasPriv());
    delAct->setEnabled(xsltMap::userHasPriv());

    connect(newAct,  SIGNAL(triggered()), this, SLOT(sNewMap()));
    connect(editAct, SIGNAL(triggered()), this, SLOT(sEditMap()));
    connect(delAct,  SIGNAL(triggered()), this, SLOT(sDeleteMap()));
  }
  else if (sender() == _atlasMap)
  {
    newAct->setEnabled(atlasMap::userHasPriv());
    editAct->setEnabled(atlasMap::userHasPriv());
    delAct->setEnabled(atlasMap::userHasPriv());

    connect(newAct,  SIGNAL(triggered()), this, SLOT(sNewAtlasMap()));
    connect(editAct, SIGNAL(triggered()), this, SLOT(sEditAtlasMap()));
    connect(delAct,  SIGNAL(triggered()), this, SLOT(sDeleteAtlasMap()));
  }
}
