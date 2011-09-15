/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "fixACL.h"

#include <QApplication>
#include <QCursor>
#include <QSqlError>
//#include <QStatusBar>

#include "storedProcErrorLookup.h"

bool fixACL::userHasPriv(const int /* ignored */)
{
  return _privileges->check("fixACL");
}

fixACL::fixACL(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_fix,		SIGNAL(clicked()),	this, SLOT(sFix()));
//  statusBar()->showMessage("");
}

fixACL::~fixACL()
{
  // no need to delete child widgets, Qt does it all for us
}

void fixACL::languageChange()
{
  retranslateUi(this);
}

void fixACL::sFix()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  q.prepare("SELECT fixACL() AS result;");
  q.exec();
  QApplication::restoreOverrideCursor();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("fixACL", result),
                  __FILE__, __LINE__);
      return;
    }
//    statusBar()->showMessage(tr("Done. %1 entities examined.").arg(result));
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
