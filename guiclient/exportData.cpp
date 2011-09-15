/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "exportData.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <mqlutil.h>
#include <parameteredit.h>
#include <selectmql.h>

#include "exporthelper.h"
#include "storedProcErrorLookup.h"

#define DEBUG true

QString exportData::exportFileDir = QString::null;

exportData::exportData(QWidget *parent, const char *name, Qt::WindowFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  _paramedit = new ParameterEdit(this);
  QGridLayout *lyt = qobject_cast<QGridLayout*>(layout());
  if (lyt)
    lyt->addWidget(_paramedit, 4, 0, 1, -1);

  connect(_otherXML,    SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_deleteQuerySet,SIGNAL(clicked()),   this, SLOT(sDeleteQuerySet()));
  connect(_editQuerySet,SIGNAL(clicked()),     this, SLOT(sEditQuerySet()));
  connect(_export,      SIGNAL(clicked()),     this, SLOT(sExport()));
  connect(_exportList,  SIGNAL(newId(int)),    this, SLOT(sHandleButtons()));
  connect(_newQuerySet, SIGNAL(clicked()),     this, SLOT(sNewQuerySet()));
  connect(_qrySetList,  SIGNAL(newId(int)),    this, SLOT(sHandleButtons()));
  connect(_xtupleXML,   SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));

  _exportList->addColumn(tr("Name"),             -1, Qt::AlignLeft, true, "xsltmap_name");
  _exportList->addColumn(tr("Document Type"),    -1, Qt::AlignLeft, true, "xsltmap_doctype");
  _exportList->addColumn(tr("System Identifier"),-1, Qt::AlignLeft, false,"xsltmap_system");
  _exportList->addColumn(tr("Export XSLT File"), -1, Qt::AlignLeft, false,"xsltmap_export");

  _qrySetList->addColumn(tr("Name"),        -1, Qt::AlignLeft, true, "qryhead_name");
  _qrySetList->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "qryhead_descrip");

  _exportList->populate("SELECT xsltmap_id, xsltmap_name, xsltmap_doctype,"
                        "       xsltmap_system, xsltmap_export"
                        "  FROM xsltmap"
                        " WHERE (LENGTH(xsltmap_export) > 0)"
                        " ORDER BY xsltmap_name;");
  sFillList();
  sHandleButtons();

  _queryset = new QuerySet(0, Qt::Dialog);
  if (_queryset)
  {
    _queryset->setWindowModality(Qt::WindowModal);
    connect(_queryset, SIGNAL(saved(int)), this, SLOT(sFillList()));
  }
}

exportData::~exportData()
{
  // no need to delete child widgets, Qt does it all for us
  if (_queryset)
    _queryset->deleteLater();
  _queryset = 0;
}

void exportData::languageChange()
{
  retranslateUi(this);
}

ParameterEdit *exportData::getParameterEdit() const
{
  return _paramedit;
}

void exportData::sDeleteQuerySet()
{
  if (QMessageBox::question(this, tr("Delete Query Set?"),
                            tr("<p>Are you sure you want to delete this "
                               "Query Set?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare("SELECT deleteQryhead(:id) AS result;");
    delq.bindValue(":id", _qrySetList->id());
    if (delq.exec())
    {
      int result = delq.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("deleteQryhead", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    if (delq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, delq.lastError().text(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void exportData::sEditQuerySet()
{
  if (_queryset)
  {
    _queryset->setId(_qrySetList->id());
    _queryset->show();
  }
}

void exportData::sExport()
{
  if (DEBUG)
    qDebug("exportData::sExport() entered with exportFileDir '%s'",
           qPrintable(exportFileDir));
  if (exportFileDir.isEmpty())
  {
#if defined Q_WS_MACX
    exportFileDir = _metrics->value("XMLExportDefaultDirMac");
#elif defined Q_WS_WIN
    exportFileDir = _metrics->value("XMLExportDefaultDirWindows");
#elif defined Q_WS_X11
    exportFileDir = _metrics->value("XMLExportDefaultDirLinux");
#endif
  }

  QString filename = QFileDialog::getSaveFileName(0, tr("Export Output File"),
              exportFileDir + QDir::separator()
              + _qrySetList->currentItem()->rawValue("qryhead_name").toString()
              + ".xml",
              "XML files (*.xml *.txt)");
  if (filename.isEmpty())
    return;
  QString tmpfilename = QFileInfo(filename).dir().absolutePath();

  exportFileDir = tmpfilename;
  QString errmsg;

  ParameterList params = _paramedit->getParameterList();
  bool success = ExportHelper::exportXML(_qrySetList->id(), params,
                                         filename,          errmsg,
                                         (_otherXML->isChecked() ?
                                                    _exportList->id() : -1));
  if (success)
    QMessageBox::information(this, tr("Processing Complete"),
                             tr("The export to %1 is complete").arg(filename));
  else
    QMessageBox::critical(this, tr("Processing Error"), errmsg);
}

void exportData::sFillList()
{
  _qrySetList->populate("SELECT qryhead_id, qryhead_name, qryhead_descrip"
                        "  FROM qryhead"
                        " ORDER BY qryhead_name;");
}

void exportData::sHandleButtons()
{
  static int prevqrysetid = -1;

  _export->setEnabled(_qrySetList->id() > 0 &&
                        (_xtupleXML->isChecked() ||
                         (_otherXML->isChecked() && _exportList->id() > 0)));

  // populate _paramedit with parameters from all of the query items in the set
  if (_qrySetList->id() != prevqrysetid)
  {
    _paramedit->clear();

    if (_qrySetList->id() < 0)
      prevqrysetid = -1;
    else
    {
      QString xml = "<report>";

      QStringList paramlist;
      XSqlQuery qiq;
      qiq.prepare("SELECT qryitem_src, qryitem_group, qryitem_detail"
                  "  FROM qryitem"
                  " WHERE ((qryitem_qryhead_id=:id)"
                  "    AND (qryitem_src!='REL'))"
                  " ORDER BY qryitem_order;");
      qiq.bindValue(":id", _qrySetList->id());
      qiq.exec();
      while (qiq.next())
      {
        QString qryitem_src = qiq.value("qryitem_src").toString();
        if (qryitem_src == "MQL")
        {
          bool valid = false;
          QString errmsg;
          QString tmpqry;
          tmpqry = MQLUtil::mqlLoad(qiq.value("qryitem_group").toString(),
                                    qiq.value("qryitem_detail").toString(),
                                    errmsg, &valid);
          if (valid)
            paramlist.append(MQLEdit::getParamsFromMetaSQLText(tmpqry));
        }
        else if (qryitem_src == "CUSTOM")
          paramlist.append(MQLEdit::getParamsFromMetaSQLText(qiq.value("qryitem_detail").toString()));
      }
      if (qiq.lastError().type() != QSqlError::NoError)
      {
        systemError(this, qiq.lastError().text(), __FILE__, __LINE__);
        return;
      }
      paramlist.sort();
      for (int i = 0; i < paramlist.size(); i++)
        xml += "\n <parameter name='" + paramlist.at(i) + "'/>";

      xml += "\n</report>";
      QDomDocument doc;
      doc.setContent(xml);
      _paramedit->setDocument(doc);
      prevqrysetid = _qrySetList->id();
    }
  }
}

void exportData::sNewQuerySet()
{
  if (_queryset)
  {
    _queryset->setId(-1);
    _queryset->show();
  }
}
