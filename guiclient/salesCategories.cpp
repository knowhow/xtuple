/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesCategories.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include "salesCategory.h"

salesCategories::salesCategories(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_salescat, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

  _salescat->addColumn(tr("Category"),    _itemColumn, Qt::AlignLeft, true, "salescat_name"   );
  _salescat->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "salescat_descrip"  );

  if (_privileges->check("MaintainSalesCategories"))
  {
    connect(_salescat, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_salescat, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_salescat, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_salescat, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_salescat, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

salesCategories::~salesCategories()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesCategories::languageChange()
{
  retranslateUi(this);
}

void salesCategories::sDelete()
{
  q.prepare("SELECT deleteSalesCategory(:salescat_id) AS result;");
  q.bindValue(":salescat_id", _salescat->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    switch (result)
    {
      case -1:
        QMessageBox::warning( this, tr("Cannot Delete Sales Category"),
                              tr( "The selected Sales Category cannot be deleted as there are unposted Invoice Lines assigned to it.\n"
                                  "You must reassign these Invoice Lines before you may delete the selected Sales Category." ) );
        break;

      case -2:
        result = QMessageBox::warning( this, tr("Cannot Delete Sales Category"),
                                       tr( "The selected Sales Category cannot be deleted as there are closed Invoice Lines assigned to it.\n"
                                           "Would you like to mark the selected Sales Category as inactive instead?" ),
                                       tr("&Yes"), tr("&No"), QString::null );
        if (result == 0)
        {
          q.prepare( "UPDATE salescat "
                     "SET salescat_active=FALSE "
                     "WHERE (salescat_id=:salescat_id);" );
          q.bindValue(":salescat_id", _salescat->id());
          q.exec();
          sFillList();
        }
        break;

      case -3:
        result = QMessageBox::warning( this, tr("Cannot Delete Sales Category"),
                                       tr( "The selected Sales Category cannot be deleted as there are A/R Open Items assigned to it.\n"
                                           "Would you like to mark the selected Sales Category as inactive instead?" ),
                                       tr("&Yes"), tr("&No"), QString::null );
        if (result == 0)
        {
          q.prepare( "UPDATE salescat "
                     "SET salescat_active=FALSE "
                     "WHERE (salescat_id=:salescat_id);" );
          q.bindValue(":salescat_id", _salescat->id());
          q.exec();
          sFillList();
        }
        break;

      default:
        sFillList();
        break;
    }
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

void salesCategories::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  salesCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesCategories::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("salescat_id", _salescat->id());

  salesCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesCategories::sCopy()
{
  ParameterList params;
  params.append("mode", "copy");
  params.append("salescat_id", _salescat->id());

  salesCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesCategories::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("salescat_id", _salescat->id());

  salesCategory newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void salesCategories::sPrint()
{
  orReport report("SalesCategoriesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void salesCategories::sFillList()
{
  _salescat->populate( "SELECT salescat_id, salescat_name, salescat_descrip "
                       "FROM salescat "
                       "ORDER BY salescat_name;" );
}

