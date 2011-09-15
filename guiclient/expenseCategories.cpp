/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "expenseCategories.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <openreports.h>
#include "expenseCategory.h"

/*
 *  Constructs a expenseCategories as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
expenseCategories::expenseCategories(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_expcat, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

//  statusBar()->hide();
  
  _expcat->addColumn(tr("Category"),    _itemColumn, Qt::AlignLeft, true, "expcat_code"   );
  _expcat->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "expcat_descrip"   );

  if (_privileges->check("MaintainExpenseCategories"))
  {
    connect(_expcat, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_expcat, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_expcat, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_expcat, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_expcat, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

expenseCategories::~expenseCategories()
{
  // no need to delete child widgets, Qt does it all for us
}

void expenseCategories::languageChange()
{
  retranslateUi(this);
}

void expenseCategories::sDelete()
{
  q.prepare( "SELECT poitem_status, COUNT(*) "
             "FROM poitem "
             "WHERE (poitem_expcat_id=:expcat_id) "
             "GROUP BY poitem_status;" );
  q.bindValue(":expcat_id", _expcat->id());
  q.exec();
  if (q.first())
  {
    if (q.findFirst("poitem_status", "U") != -1)
      QMessageBox::warning( this, tr("Cannot Delete Expense Category"),
                            tr( "<p>The selected Expense Category cannot be deleted as there are unposted P/O Lines assigned to it. "
                                "You must reassign these P/O Lines before you may delete the selected Expense Category.</p>" ) );
    else if (q.findFirst("poitem_status", "O") != -1)
      QMessageBox::warning( this, tr("Cannot Delete Expense Category"),
                            tr( "<p>The selected Expense Category cannot be deleted as there are open P/O Lines assigned to it. "
                                "You must close these P/O Lines before you may delete the selected Expense Category.</p>" ) );
    else if (q.findFirst("poitem_status", "C") != -1)
    {
      int result = QMessageBox::warning( this, tr("Cannot Delete Expense Category"),
                                         tr( "<p>The selected Expense Category cannot be deleted as there are closed P/O Lines assigned to it. "
                                             "Would you like to mark the selected Expense Category as inactive instead?</p>" ),
                                         tr("&Yes"), tr("&No"), QString::null );
      if (result == 0)
      {
        q.prepare( "UPDATE expcat "
                   "SET expcat_active=FALSE "
                   "WHERE (expcat_id=:expcat_id);" );
        q.bindValue(":expcat_id", _expcat->id());
        q.exec();
	sFillList();
      }
    }
  }
  else
  {
    q.prepare( "SELECT checkhead_id AS id"
               "  FROM checkhead"
               " WHERE(checkhead_expcat_id=:expcat_id)"
               " UNION "
               "SELECT vodist_id AS id"
               "  FROM vodist"
               " WHERE(vodist_expcat_id=:expcat_id)");
    q.bindValue(":expcat_id", _expcat->id());
    q.exec();
    if(q.first())
    {
      QMessageBox::warning( this, tr("Cannot Delete Expense Category"),
        tr( "<p>The selected Expense Category cannot be deleted as there are Checks and Voucher Distributions assigned to it. "
            "You must change these before you may delete the selected Expense Category.</p>" ) );
    }
    else
    {
      q.prepare( "DELETE FROM expcat "
                 "WHERE (expcat_id=:expcat_id); ");
      q.bindValue(":expcat_id", _expcat->id());
      q.exec();
      sFillList();
    }
  }
}

void expenseCategories::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  expenseCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void expenseCategories::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("expcat_id", _expcat->id());

  expenseCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void expenseCategories::sCopy()
{
  ParameterList params;
  params.append("mode", "copy");
  params.append("expcat_id", _expcat->id());

  expenseCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void expenseCategories::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("expcat_id", _expcat->id());

  expenseCategory newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void expenseCategories::sPrint()
{
  orReport report("ExpenseCategoriesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void expenseCategories::sFillList()
{
  _expcat->populate( "SELECT expcat_id, expcat_code, expcat_descrip "
                     "FROM expcat "
                    "ORDER BY expcat_code;" );
}

