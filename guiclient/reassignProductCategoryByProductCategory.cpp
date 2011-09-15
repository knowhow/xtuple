/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reassignProductCategoryByProductCategory.h"

#include <QMessageBox>
#include <qvariant.h>

/*
 *  Constructs a reassignProductCategoryByProductCategory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
reassignProductCategoryByProductCategory::reassignProductCategoryByProductCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_productCategoryPattern, SIGNAL(toggled(bool)), _productCategory, SLOT(setEnabled(bool)));
    connect(_selectedProductCategory, SIGNAL(toggled(bool)), _productCategories, SLOT(setEnabled(bool)));
    connect(_reassign, SIGNAL(clicked()), this, SLOT(sReassign()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
reassignProductCategoryByProductCategory::~reassignProductCategoryByProductCategory()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void reassignProductCategoryByProductCategory::languageChange()
{
    retranslateUi(this);
}


void reassignProductCategoryByProductCategory::init()
{
  _productCategories->setType(XComboBox::ProductCategories);
  _newProductCategory->setType(XComboBox::ProductCategories);
}

void reassignProductCategoryByProductCategory::sReassign()
{
  if ( (_productCategoryPattern->isChecked()) && (_productCategory->text().length() == 0) )
  {
    QMessageBox::warning(this, tr("Missing Product Category Pattern"),
                      tr("<p>You must enter a Product Category Pattern."));
    _productCategory->setFocus();
    return;
  }
  
  QString sql( "UPDATE item "
               "SET item_prodcat_id=:new_prodcat_id " );

  if (_selectedProductCategory->isChecked())
    sql += "WHERE (item_prodcat_id=:old_prodcat_id);";
  else if (_productCategoryPattern->isChecked())
    sql += "WHERE (item_prodcat_id IN (SELECT prodcat_id FROM prodcat WHERE (prodcat_code ~ :old_prodcat_code)));";

  XSqlQuery reassign;
  reassign.prepare(sql);
  reassign.bindValue(":new_prodcat_id", _newProductCategory->id());
  reassign.bindValue(":old_prodcat_id", _productCategories->id());
  reassign.bindValue(":old_prodcat_code", _productCategory->text());
  reassign.exec();

  accept();
}
