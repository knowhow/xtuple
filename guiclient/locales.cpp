/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "locales.h"

#include <QVariant>
#include <QMessageBox>
#include <parameter.h>
#include "sysLocale.h"

locales::locales(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_locale, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_locale, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_locale, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_locale, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));

  _locale->addColumn(tr("Code"),        _itemColumn, Qt::AlignLeft, true, "locale_code" );
  _locale->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "locale_descrip" );
  
  sFillList();
}

locales::~locales()
{
  // no need to delete child widgets, Qt does it all for us
}

void locales::languageChange()
{
  retranslateUi(this);
}

void locales::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  sysLocale newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void locales::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("locale_id", _locale->id());

  sysLocale newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void locales::sCopy()
{
  ParameterList params;
  params.append("mode", "copy");
  params.append("locale_id", _locale->id());

  sysLocale newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void locales::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("locale_id", _locale->id());

  sysLocale newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void locales::sDelete()
{
  q.prepare( "SELECT usr_username "
             "FROM usr "
             "WHERE (usr_locale_id=:locale_id) "
             "LIMIT 1;" );
  q.bindValue(":locale_id", _locale->id());
  q.exec();
  if (q.first())
    QMessageBox::critical( this, tr("Cannot Delete Selected Locale"),
                           tr( "The selected Locale cannot be deleted as it has been assigned to one or more Users.\n"
                               "You must reassign the Locales for these Users before you may delete the selected Locale." ) );
  else
  {
    q.prepare( "DELETE FROM locale "
               "WHERE (locale_id=:locale_id);" );
    q.bindValue(":locale_id", _locale->id());
    q.exec();

    sFillList();
  }
}

void locales::sFillList()
{
  _locale->populate( "SELECT locale_id, locale_code, locale_descrip "
                     "FROM locale "
                     "ORDER BY locale_code" );
}

