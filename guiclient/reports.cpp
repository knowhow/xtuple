/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reports.h"

#include <QMessageBox>
#include <QVariant>
#include <QWorkspace>

#include <metasql.h>
#include <mqlutil.h>
#include <openreports.h>
#include <reporthandler.h>

#include "errorReporter.h"

reports::reports(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_byPackage,  SIGNAL(toggled(bool)), this,    SLOT(sFillList()));
  connect(_close,          SIGNAL(clicked()), this,    SLOT(close()));
  connect(_delete,         SIGNAL(clicked()), this,    SLOT(sDelete()));
  connect(_edit,           SIGNAL(clicked()), this,    SLOT(sEdit()));
  connect(_new,            SIGNAL(clicked()), this,    SLOT(sNew()));
  connect(_print,          SIGNAL(clicked()), this,    SLOT(sPrint()));
  connect(_report, SIGNAL(itemSelected(int)), _edit,   SLOT(animateClick()));
  connect(_report,       SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_report,       SIGNAL(valid(bool)), _edit,   SLOT(setEnabled(bool)));

  _report->addColumn(tr("Name"),           200, Qt::AlignLeft, true, "report_name" );
  _report->addColumn(tr("Grade"),   _seqColumn, Qt::AlignRight,true, "report_grade");
  _report->addColumn(tr("Description"),     -1, Qt::AlignLeft, true, "report_descrip");
  _report->addColumn(tr("Package"),_itemColumn, Qt::AlignLeft, false,"pkgname");

  connect(omfgThis, SIGNAL(reportsChanged(int, bool)), this, SLOT(sFillList()));

  sFillList();
}

reports::~reports()
{
  // no need to delete child widgets, Qt does it all for us
}

void reports::languageChange()
{
  retranslateUi(this);
}

bool reports::setParams(ParameterList &params)
{
  if (_byPackage->isChecked())
    params.append("byPackage");

  return true;
}

void reports::sFillList()
{
  QString errmsg;
  bool    ok;
  MetaSQLQuery getm = MQLUtil::mqlLoad("reports", "detail", errmsg, &ok);
  if (! ok)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Getting Reports"),
                         errmsg, __FILE__, __LINE__);
    return;
  }

  ParameterList getp;
  if (! setParams(getp))
    return;

  XSqlQuery getq = getm.toQuery(getp);
  _report->populate(getq);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Reports"),
                           getq, __FILE__, __LINE__))
    return;
}

void reports::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("ReportsMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void reports::sNew()
{
  if (!omfgThis->_reportHandler)
  {
    omfgThis->_reportHandler = new ReportHandler(omfgThis, "report handler");
    if(_preferences->value("InterfaceWindowOption") != "TopLevel")
      omfgThis->_reportHandler->setParentWindow(omfgThis->workspace());
    omfgThis->_reportHandler->setAllowDBConnect(FALSE);
    omfgThis->_reportHandler->setPlaceMenusOnWindows(TRUE);
    omfgThis->_reportHandler->setPlaceToolbarsOnWindows(TRUE);
    connect(omfgThis->_reportHandler, SIGNAL(reportsChanged(int, bool)), omfgThis, SLOT(sReportsChanged(int, bool)));
  }

  omfgThis->_reportHandler->fileNew();
}

void reports::sEdit()
{
  if (!omfgThis->_reportHandler)
  {
    omfgThis->_reportHandler = new ReportHandler(omfgThis, "report handler");
    if(_preferences->value("InterfaceWindowOption") != "TopLevel")
      omfgThis->_reportHandler->setParentWindow(omfgThis->workspace());
    omfgThis->_reportHandler->setAllowDBConnect(FALSE);
    omfgThis->_reportHandler->setPlaceMenusOnWindows(TRUE);
    omfgThis->_reportHandler->setPlaceToolbarsOnWindows(TRUE);
    connect(omfgThis->_reportHandler, SIGNAL(reportsChanged(int, bool)), omfgThis, SLOT(sReportsChanged(int, bool)));
  }

  XSqlQuery getq;
  getq.prepare("SELECT report_name, report_grade, report_source "
               "  FROM report "
               " WHERE (report_id=:report_id);");
  getq.bindValue(":report_id", _report->id());
  getq.exec();
  if (getq.first())
  {
    QDomDocument doc;
    QString errorMessage;
    int errorLine;
    int errorColumn;

    if (doc.setContent(getq.value("report_source").toString(),
                       &errorMessage, &errorLine, &errorColumn))
      omfgThis->_reportHandler->fileOpen(doc,
                                         getq.value("report_name").toString(),
                                         getq.value("report_grade").toInt());
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Loading Report"),
                           errorMessage, getq.value("report_name").toString(),
                           errorLine);
      return;
    }
  }
}

void reports::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Report?"),
                            tr("<p>Are you sure that you want to completely "
                               "delete the selected Report?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare( "DELETE FROM report WHERE (report_id=:report_id);" );
    delq.bindValue(":report_id", _report->id());
    delq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Report"),
                             delq, __FILE__, __LINE__))
      return;

    sFillList();
  }
}
