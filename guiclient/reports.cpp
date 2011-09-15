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

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <QWorkspace>
#include <openreports.h>
#include <reporthandler.h>

/*
 *  Constructs a reports as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
reports::reports(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_report, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_report, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_report, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

//  statusBar()->hide();

  _report->addColumn( tr("Name"),       200, Qt::AlignLeft,  true, "report_name" );
  _report->addColumn( tr("Grade"),       50, Qt::AlignRight, true, "report_grade");
  _report->addColumn( tr("Description"), -1, Qt::AlignLeft | Qt::AlignTop, true, "report_descrip");

  connect(omfgThis, SIGNAL(reportsChanged(int, bool)), this, SLOT(sFillList()));

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
reports::~reports()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void reports::languageChange()
{
    retranslateUi(this);
}

void reports::sFillList()
{
  _report->populate( "SELECT report_id, report_name, report_grade, report_descrip "
                     "FROM report "
                     "ORDER BY report_name, report_grade" );
}

void reports::sPrint()
{
  orReport report("ReportsMasterList");
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

  q.prepare( "SELECT report_name, report_grade, report_source "
             "  FROM report "
             " WHERE (report_id=:report_id); " );
  q.bindValue(":report_id", _report->id());
  q.exec();
  if (q.first())
  {
    QDomDocument doc;
    QString errorMessage;
    int errorLine;
    int errorColumn;

    if (doc.setContent(q.value("report_source").toString(), &errorMessage, &errorLine, &errorColumn))
      omfgThis->_reportHandler->fileOpen(doc, q.value("report_name").toString(), q.value("report_grade").toInt());
    else
      QMessageBox::warning( this, tr("Error Loading Report"),
                            tr( "ERROR parsing content:\n"
                                "\t%1 (Line %2 Column %3)" )
                            .arg(errorMessage)
                            .arg(errorLine)
                            .arg(errorColumn) );
  }
}

void reports::sDelete()
{
  if ( QMessageBox::warning(this, tr("Delete Report?"),
                            tr("<p>Are you sure that you want to completely "
                               "delete the selected Report?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare( "DELETE FROM report "
               "WHERE (report_id=:report_id);" );
    q.bindValue(":report_id", _report->id());
    q.exec();

    sFillList();
  }
}
