/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reassignCustomerTypeByCustomerType.h"

#include <QMessageBox>
#include <qvariant.h>

/*
 *  Constructs a reassignCustomerTypeByCustomerType as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
reassignCustomerTypeByCustomerType::reassignCustomerTypeByCustomerType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_customerTypePattern, SIGNAL(toggled(bool)), _customerType, SLOT(setEnabled(bool)));
    connect(_selectedCustomerType, SIGNAL(toggled(bool)), _customerTypes, SLOT(setEnabled(bool)));
    connect(_reassign, SIGNAL(clicked()), this, SLOT(sReassign()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
reassignCustomerTypeByCustomerType::~reassignCustomerTypeByCustomerType()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void reassignCustomerTypeByCustomerType::languageChange()
{
    retranslateUi(this);
}


void reassignCustomerTypeByCustomerType::init()
{
  _customerTypes->setType(XComboBox::CustomerTypes);
  _newCustomerType->setType(XComboBox::CustomerTypes);
}

void reassignCustomerTypeByCustomerType::sReassign()
{
  if ( (_customerTypePattern->isChecked()) && (_customerType->text().length() == 0) )
  {
    QMessageBox::warning(this, tr("Missing Customer Type Pattern"),
                      tr("<p>You must enter a Customer Type Pattern."));
    _customerType->setFocus();
    return;
  }
  
  QString sql( "UPDATE custinfo "
               "SET cust_custtype_id=:new_custtype_id " );

  if (_selectedCustomerType->isChecked())
    sql += "WHERE (cust_custtype_id=:old_custtype_id);";
  else if (_customerTypePattern->isChecked())
    sql += "WHERE (cust_custtype_id IN (SELECT custtype_id FROM custtype WHERE (custtype_code ~ :old_custtype_code)));";

  q.prepare(sql);
  q.bindValue(":new_custtype_id", _newCustomerType->id());
  q.bindValue(":old_custtype_id", _customerTypes->id());
  q.bindValue(":old_custtype_code", _customerType->text());
  q.exec();

  accept();
}
