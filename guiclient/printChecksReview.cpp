/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printChecksReview.h"

#include <QSqlError>
#include <QVariant>

#include "guiclient.h"
#include "storedProcErrorLookup.h"

printChecksReview::printChecksReview(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_complete,  SIGNAL(clicked()), this, SLOT(sComplete()));
  connect(_printed,   SIGNAL(clicked()), this, SLOT(sMarkPrinted()));
  connect(_replace,   SIGNAL(clicked()), this, SLOT(sMarkReplaced()));
  connect(_selectAll, SIGNAL(clicked()), this, SLOT(sSelectAll()));
  connect(_unmark,    SIGNAL(clicked()), this, SLOT(sUnmark()));
  connect(_voided,    SIGNAL(clicked()), this, SLOT(sMarkVoided()));

  _checks->addColumn(tr("Check Number"),          -1, Qt::AlignLeft );
  _checks->addColumn(tr("Action"),       _itemColumn, Qt::AlignLeft );
}

printChecksReview::~printChecksReview()
{
  // no need to delete child widgets, Qt does it all for us
}

void printChecksReview::languageChange()
{
  retranslateUi(this);
}

static const int ActionUnmark = -1;
static const int ActionPrinted = 1;
static const int ActionVoided = 2;
static const int ActionReplaced = 3;

/*
  TODO: refactor printChecks/printChecksReview so that printChecks
	hands printChecksReview a list of checks to be reviewed
	instead of populating printChecksReview directly.  Then
	printChecksReview could populate its GUI from this list.
	This will allow printChecksReview to have real error reporting
	and recovery: requery the db for the entire passed-in list
	and show the current state of each check so the user can
	decide what to reprocess if there were errors.
*/
void printChecksReview::sComplete()
{
  XSqlQuery checkPrint;
  checkPrint.prepare( "SELECT markCheckAsPrinted(:check_id) AS result;");

  XSqlQuery checkVoid;
  checkVoid.prepare( "SELECT voidCheck(:check_id) AS result;" );

  XSqlQuery checkReplace;
  checkReplace.prepare( "SELECT replaceVoidedCheck(:check_id) AS result;" );

  // no returns in the loop: process as much as possible, regardless of errors
  for (int i = 0; i < _checks->topLevelItemCount(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)(_checks->topLevelItem(i));

    switch(cursor->altId())
    {
      case ActionPrinted:
        checkPrint.bindValue(":check_id", cursor->id());
        checkPrint.exec();
	if (checkPrint.first())
	{
	  int result = checkPrint.value("result").toInt();
	  if (result < 0)
	    systemError(this, storedProcErrorLookup("markCheckPrinted", result), __FILE__, __LINE__);
	}
	else if (checkPrint.lastError().type() != QSqlError::NoError)
	  systemError(this, checkPrint.lastError().databaseText(), __FILE__, __LINE__);
        break;
      case ActionVoided:
        checkVoid.bindValue(":check_id", cursor->id());
        checkVoid.exec();
	if (checkVoid.first())
	{
	  int result = checkVoid.value("result").toInt();
	  if (result < 0)
	    systemError(this, storedProcErrorLookup("voidCheck", result), __FILE__, __LINE__);
	}
	else if (checkVoid.lastError().type() != QSqlError::NoError)
	  systemError(this, checkVoid.lastError().databaseText(), __FILE__, __LINE__);
        break;
      case ActionReplaced:
        checkVoid.bindValue(":check_id", cursor->id());
        checkVoid.exec();
	if (checkVoid.first())
	{
	  int result = checkVoid.value("result").toInt();
	  if (result < 0)
	    systemError(this, storedProcErrorLookup("voidCheck", result), __FILE__, __LINE__);
	}
	else if (checkVoid.lastError().type() != QSqlError::NoError)
	  systemError(this, checkVoid.lastError().databaseText(), __FILE__, __LINE__);
        checkReplace.bindValue(":check_id", cursor->id());
        checkReplace.exec();
	if (checkReplace.first())
	{
	  int result = checkReplace.value("result").toInt();
	  if (result < 0)
	    systemError(this, storedProcErrorLookup("replaceVoidedCheck", result), __FILE__, __LINE__);
	}
	else if (checkReplace.lastError().type() != QSqlError::NoError)
	  systemError(this, checkReplace.lastError().databaseText(), __FILE__, __LINE__);
        break;
    }
  }
  // TODO: after refactoring, handle any errors in the loop here and *return*

  close();
}

void printChecksReview::sUnmark()
{
  markSelected(ActionUnmark);
}

void printChecksReview::sMarkPrinted()
{
  markSelected(ActionPrinted);
}

void printChecksReview::sMarkVoided()
{
  markSelected(ActionVoided);
}

void printChecksReview::sMarkReplaced()
{
  markSelected(ActionReplaced);
}

void printChecksReview::sSelectAll()
{
  _checks->selectAll();
}

void printChecksReview::markSelected( int actionId )
{
  QString action;
  switch(actionId)
  {
    case ActionPrinted:
      action = tr("Printed");
      break;
    case ActionVoided:
      action = tr("Voided");
      break;
    case ActionReplaced:
      action = tr("Replace");
      break;
    case ActionUnmark:
    default:
      actionId = -1;
      action = "";
  };

  QList<XTreeWidgetItem*> selected = _checks->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];
    cursor->setText(1, action);
    cursor->setAltId(actionId);
  }
}
