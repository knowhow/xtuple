/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "zeroUncountedCountTagsByWarehouse.h"

#include <qvariant.h>
#include "guiclient.h"
/*
 *  Constructs a zeroUncountedCountTagsByWarehouse as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
zeroUncountedCountTagsByWarehouse::zeroUncountedCountTagsByWarehouse(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_zero, SIGNAL(clicked()), this, SLOT(sZero()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
zeroUncountedCountTagsByWarehouse::~zeroUncountedCountTagsByWarehouse()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void zeroUncountedCountTagsByWarehouse::languageChange()
{
    retranslateUi(this);
}


void zeroUncountedCountTagsByWarehouse::init()
{
}

void zeroUncountedCountTagsByWarehouse::sZero()
{
  if (_warehouse->isSelected())
  {
    q.prepare( "SELECT enterCount(invcnt_id, 0, :comments) "
               "FROM invcnt, itemsite "
               "WHERE ( (invcnt_itemsite_id=itemsite_id)"
               " AND (invcnt_qoh_after IS NULL)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    q.bindValue(":warehous_id", _warehouse->id());
  }
  else
    q.prepare( "SELECT enterCount(invcnt_id, 0, :comments) "
               "FROM invcnt "
               "WHERE (invcnt_qoh_after IS NULL);" );

  q.bindValue(":comments", _comments->toPlainText());
  q.exec();

  accept();
}
