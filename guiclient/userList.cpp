/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "userList.h"

#include <QVariant>

/*
 *  Constructs a userList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
userList::userList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_usr, SIGNAL(valid(bool)), _select, SLOT(setEnabled(bool)));
  connect(_usr, SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));

  _usr->addColumn( tr("Username"),    _itemColumn, Qt::AlignLeft,  true,  "usr_username" );
  _usr->addColumn( tr("Proper Name"), -1,          Qt::AlignLeft,  true,  "usr_propername" );
  
  _usr->populate( "SELECT usr_id, usr_username, usr_propername "
                  "FROM usr "
                  "ORDER BY usr_username;" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
userList::~userList()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void userList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse userList::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("usr_id", &valid);
  if (valid)
    _usr->setId(param.toInt());
  
  return NoError;
}

void userList::sSelect()
{
  done(_usr->id());
}
