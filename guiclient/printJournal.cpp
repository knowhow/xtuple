/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printJournal.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>

/*
 *  Constructs a printJournal as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printJournal::printJournal(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
printJournal::~printJournal()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void printJournal::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printJournal::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("type", &valid);
  if (valid)
  {
    switch ((_type = param.toInt()))
    {
      case SalesJournal:
        setWindowTitle(tr("Print Sales Journal"));
        _journalNumber->populate( "SELECT jrnluse_number, (jrnluse_use || TEXT(jrnluse_number) || ':' || formatDate(jrnluse_date)) "
                                  "FROM jrnluse "
                                  "WHERE (jrnluse_use='AR-IN') "
                                  "ORDER BY jrnluse_date;" );
        break;

      case CreditMemoJournal:
        setWindowTitle(tr("Print Credit Memo Journal"));
        _journalNumber->populate( "SELECT jrnluse_number, (jrnluse_use || TEXT(jrnluse_number) || ':' || formatDate(jrnluse_date)) "
                                  "FROM jrnluse "
                                  "WHERE (jrnluse_use='AR-CM') "
                                  "ORDER BY jrnluse_date;" );
          break;

      case PayablesJournal:
        setWindowTitle(tr("Print Payables Journal"));
        _journalNumber->populate( "SELECT jrnluse_number, (jrnluse_use || TEXT(jrnluse_number) || ':' || formatDate(jrnluse_date)) "
                                  "FROM jrnluse "
                                  "WHERE (jrnluse_use='AP-VO') "
                                  "ORDER BY jrnluse_date;" );
          break;

      case CheckJournal:
        setWindowTitle(tr("Print Check Journal"));
        _journalNumber->populate( "SELECT jrnluse_number, (jrnluse_use || TEXT(jrnluse_number) || ':' || formatDate(jrnluse_date)) "
                                  "FROM jrnluse "
                                  "WHERE (jrnluse_use='AP-CK') "
                                  "ORDER BY jrnluse_date;" );
          break;

    }
  }

  param = pParams.value("journalNumber", &valid);
  if (valid)
    _journalNumber->setId(param.toInt());

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void printJournal::sPrint()
{
  QString reportName;

  ParameterList params;
  params.append("journalNumber", _journalNumber->id());

  switch (_type)
  {
    case SalesJournal:
      reportName = "SalesJournal";
      break;

    case CreditMemoJournal:
      reportName = "CreditMemoJournal";
      break;

    case PayablesJournal:
      reportName = "PayablesJournal";
      break;

    case CheckJournal:
      reportName = "CheckJournal";
      break;
  }

  orReport report(reportName, params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);

  accept();
}

