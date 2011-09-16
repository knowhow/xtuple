/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "importData.h"

#include <QDirIterator>
#include <QMessageBox>
#include <QVariant>

#include "configureIE.h"
#include "importhelper.h"
#include "storedProcErrorLookup.h"

#define DEBUG false

enum ImportFileType { Unknown = -1, Csv, Xml };

bool importData::userHasPriv()
{
  return _privileges->check("ImportXML");
}

void importData::setVisible(bool visible)
{
  if (! visible)
    XWidget::setVisible(false);

  else if (! userHasPriv())
  {
    systemError(this,
                tr("You do not have sufficient privilege to view this window"),
                __FILE__, __LINE__);
    close();
  }
  else if (_metrics->value("XMLSuccessTreatment").isEmpty() ||
           _metrics->value("XSLTLibrary").isEmpty()) // not configured properly
  {
    QString msg = tr("The application is not set up to import data. "
                     "Have an administrator configure Data Import before "
                     "trying to import data.");
    if (configureIE::userHasPriv())
      tr("<p>You must first set up the application to import data.");

    systemError(this, msg, __FILE__, __LINE__);
    deleteLater();
  }
  else
    XWidget::setVisible(true);
}

importData::importData(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_add,            SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_autoUpdate,  SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_clearStatus,    SIGNAL(clicked()), this, SLOT(sClearStatus()));
  connect(_delete,         SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_file, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_importAll,      SIGNAL(clicked()), this, SLOT(sImportAll()));
  connect(_importSelected, SIGNAL(clicked()), this, SLOT(sImportSelected()));
  connect(_resetList,      SIGNAL(clicked()), this, SLOT(sFillList()));

  _file->addColumn(tr("Type"),          -1, Qt::AlignLeft,  false, "type");
  _file->addColumn(tr("File Name"),     -1, Qt::AlignLeft,  true,  "filename");
  _file->addColumn(tr("Status"), _ynColumn, Qt::AlignCenter,true,  "status");

  _defaultDir = _metrics->value(
#if defined Q_WS_MACX
                                      "XMLDefaultDirMac"
#elif defined Q_WS_WIN
                                      "XMLDefaultDirWindows"
#elif defined Q_WS_X11
                                      "XMLDefaultDirLinux"
#endif
      );
  if (_defaultDir.isEmpty())
    _defaultDir = ".";

  sFillList();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

importData::~importData()
{
  // no need to delete child widgets, Qt does it all for us
}

void importData::languageChange()
{
  retranslateUi(this);
}

void importData::sFillList()
{
  _file->clear();
  if (! _defaultDir.isEmpty())
  {
    QStringList filters;
    filters << "*.xml" << "*.XML";
    if (ImportHelper::getCSVImpPlugin(omfgThis))
      filters << "*.csv" << "*.CSV";

    QDirIterator iterator(_defaultDir, filters);
    XTreeWidgetItem *last = 0;
    for (int i = 0; iterator.hasNext(); i++)
    {
      QString filename = iterator.next();
      ImportFileType type = Unknown;
      QString suffix = QFileInfo(filename).suffix().toUpper();
      if (suffix == "XML")
        type = Xml;
      else if (suffix == "CSV")
        type = Csv;
      last = new XTreeWidgetItem(_file, last, i, QVariant(suffix), QVariant(filename));
    }
  }
}

void importData::sAdd()
{
  QFileDialog newdlg(this, tr("Select File(s) to Import"), _defaultDir);
  newdlg.setFileMode(QFileDialog::ExistingFiles);
  QStringList filters;
  if (ImportHelper::getCSVImpPlugin(omfgThis))
    filters << tr("Data files (*.xml *.csv)")
            << tr("CSV files (*.csv)");
  filters << tr("XML files (*.xml)")
          << tr("Any Files (*)");
  newdlg.setNameFilters(filters);
  if (newdlg.exec())
  {
    QStringList files = newdlg.selectedFiles();
    XTreeWidgetItem *last = 0;
    for (int i = 0; i < files.size(); i++)
    {
      ImportFileType type = Unknown;
      QString suffix = QFileInfo(files[i]).suffix().toUpper();
      if (suffix == "XML")
        type = Xml;
      else if (suffix == "CSV")
        type = Csv;

      last = new XTreeWidgetItem(_file, last, i, type, QVariant(suffix), QVariant(files[i]));
    }
  }
}

void importData::sClearStatus()
{
  QList<XTreeWidgetItem*> selected = _file->selectedItems();
  for (int i = selected.size() - 1; i >= 0; i--)
    selected[i]->setText(_file->column("status"), "");
}

void importData::sDelete()
{
  QList<XTreeWidgetItem*> selected = _file->selectedItems();
  for (int i = selected.size() - 1; i >= 0; i--)
    _file->takeTopLevelItem(_file->indexOfTopLevelItem(selected[i]));
}

void importData::sPopulateMenu(QMenu* pMenu, QTreeWidgetItem* /* pItem */)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Import Selected"),  this, SLOT(sImportSelected()));
  menuItem = pMenu->addAction(tr("Clear Status"),     this, SLOT(sClearStatus()));
  menuItem = pMenu->addAction(tr("Delete From List"), this, SLOT(sDelete()));
}

void importData::sImportAll()
{
  bool oldAutoUpdate = _autoUpdate->isChecked();
  sHandleAutoUpdate(false);
  for (int i = 0; i < _file->topLevelItemCount(); i++)
  {
    XTreeWidgetItem* pItem = _file->topLevelItem(i);
    if (pItem->text("status").isEmpty())
    {
      if (importOne(pItem->text("filename"), pItem->altId()))
        pItem->setText(_file->column("status"), tr("Done"));
      else
        pItem->setText(_file->column("status"), tr("Error"));
    }
  }
  if (oldAutoUpdate)
    sHandleAutoUpdate(true);
}

void importData::sImportSelected()
{
  bool oldAutoUpdate = _autoUpdate->isChecked();
  sHandleAutoUpdate(false);
  QList<XTreeWidgetItem*> selected = _file->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    if (selected[i]->text("status").isEmpty())
    {
      if (importOne(selected[i]->text("filename"), selected[i]->altId()))
        selected[i]->setText(_file->column("status"), tr("Done"));
      else
        selected[i]->setText(_file->column("status"), tr("Error"));
    }
  }
  if (oldAutoUpdate)
    sHandleAutoUpdate(true);
}

bool importData::importOne(const QString &pFileName, int pType)
{
  if (DEBUG)
    qDebug("importData::importOne(%s, %d)", qPrintable(pFileName), pType);

  QString errmsg;
  QString warnmsg;
  if (pType == Xml || QFileInfo(pFileName).suffix().toUpper() == "XML")
  {
    if (! ImportHelper::importXML(pFileName, errmsg, warnmsg))
    {
      systemError(this, errmsg);
      return false;
    }
    else if (! warnmsg.isEmpty())
    {
      QMessageBox::warning(this, tr("XML Import Warnings"), warnmsg);
      return true;
    }
  }
  else if (pType == Csv || QFileInfo(pFileName).suffix().toUpper() == "CSV")
  {
    if (! ImportHelper::importCSV(pFileName, errmsg))
    {
      systemError(this, errmsg);
      return false;
    }
  }
  else
  {
    systemError(this, tr("Don't know how to import %1").arg(pFileName));
    return false;
  }

  return true;
}

void importData::sHandleAutoUpdate(const bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}
