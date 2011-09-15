/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "hotkeys.h"

#include <QVariant>
#include "hotkey.h"

/*
 *  Constructs a hotkeys as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
hotkeys::hotkeys(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_hotkey, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_hotkey, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_hotkey, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

  _dirty = FALSE;

  _hotkey->addColumn(tr("Keystroke"), _itemColumn, Qt::AlignLeft );
  _hotkey->addColumn(tr("Action"),    -1,          Qt::AlignLeft );
  _hotkey->addColumn("key",           0,           Qt::AlignLeft );

  sFillList();
}

hotkeys::~hotkeys()
{
  // no need to delete child widgets, Qt does it all for us
}

void hotkeys::languageChange()
{
    retranslateUi(this);
}

enum SetResponse hotkeys::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("username", &valid);
  if (valid)
  {
    _username->setText(param.toString());
    _currentUser = FALSE;
  }

  param = pParams.value("currentUser", &valid);
  if (valid)
  {
    _username->setText(omfgThis->username());
    _currentUser = TRUE;
  }

  sFillList();

  return NoError;
}

void hotkeys::sFillList()
{
  _hotkey->clear();

  QString hotkey;
  QString action;
  char    key;

  XTreeWidgetItem *last = 0;
  if (_currentUser)
  {
    for (key = '1'; key <= '9'; key++)
    {
      hotkey = QString("F%1").arg(key);
      action = _preferences->value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(tr("F%1").arg(key)), action, hotkey);
    }

    for (key = 'A'; key <= 'Z'; key++)
    {
      hotkey = QString("C%1").arg(key);
      action = _preferences->value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(QString("Ctrl+%1").arg(key)), action, hotkey);
    }

    for (key = '0'; key <= '9'; key++)
    {
      hotkey = QString("C%1").arg(key);
      action = _preferences->value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(QString("Ctrl+%1").arg(key)), action, hotkey);
    }
  }
  else
  {
    Preferences pref(_username->text());

    for (key = '1'; key <= '9'; key++)
    {
      hotkey = QString("F%1").arg(key);
      action = pref.value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(tr("F%1").arg(key)), action, hotkey);
    }

    for (key = 'A'; key <= 'Z'; key++)
    {
      hotkey = QString("C%1").arg(key);
      action = pref.value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1,QVariant( QString("Ctrl+%1").arg(key)), action, hotkey);
    }

    for (key = '0'; key <= '9'; key++)
    {
      hotkey = QString("C%1").arg(key);
      action = pref.value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(QString("Ctrl+%1").arg(key)), action, hotkey);
    }
  }
}

void hotkeys::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  if (_currentUser)
    params.append("currentUser");
  else
    params.append("username", _username->text());

  hotkey newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
  {
    _dirty = TRUE;
    sFillList();
  }
}

void hotkeys::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("hotkey", _hotkey->currentItem()->text(2));

  if (_currentUser)
    params.append("currentUser");
  else
    params.append("username", _username->text());

  hotkey newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
  {
    _dirty = TRUE;
    sFillList();
  }
}

void hotkeys::sDelete()
{
  if (_currentUser)
  {
    _preferences->remove(_hotkey->currentItem()->text(2));
    _preferences->load();
  }
  else
  {
    q.prepare("SELECT deleteUserPreference(:username, :name) AS _result;");
    q.bindValue(":username", _username->text());
    q.bindValue(":name", _hotkey->currentItem()->text(2));
    q.exec();
  }

  _dirty = TRUE;
  sFillList();
}

void hotkeys::sClose()
{
  if (_dirty)
    accept();
  else
    reject();
}

