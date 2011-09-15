/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "copyBudget.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

/*
 *  Constructs a copyBudget as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
copyBudget::copyBudget(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _budgheadid = -1;

  // signals and slots connections
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
copyBudget::~copyBudget()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void copyBudget::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyBudget::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("budghead_id", &valid);
  if (valid)
  {
    _budgheadid = param.toInt();
    _copy->setEnabled(true);
  }

  return NoError;
}

void copyBudget::sCopy()
{
  if(_name->text().trimmed().isEmpty())
  {
    QMessageBox::warning(this, tr("Name Required"),
        tr("Budget Name is a required field."));
    _name->setFocus();
    return;
  }

  q.prepare("SELECT copyBudget(:budghead_id, :name, :descrip, :interval) AS result;");
  q.bindValue(":budghead_id", _budgheadid);
  q.bindValue(":name", _name->text());
  q.bindValue(":descrip", _descrip->text());
  q.bindValue(":interval", _interval->value());
  q.exec();

  if(q.first() && q.value("result").toInt() < 0)
  {
    QMessageBox::information( this, tr("Error Copying Budget"),
                              tr( "There was an error copying the budget. Make sure there are valid periods\n"
                                  "In the future to match the current periods of the budget being copied plus\n"
                                  "the period inteval." ));
    return;
  }
  else if(q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

