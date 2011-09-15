/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "hotkey.h"

#include <QVariant>
#include <QMessageBox>

hotkey::hotkey(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  int i = 0;
  _hotkey->append(i++, "F1");
  _hotkey->append(i++, "F2");
  _hotkey->append(i++, "F3");
  _hotkey->append(i++, "F4");
  _hotkey->append(i++, "F5");
  _hotkey->append(i++, "F6");
  _hotkey->append(i++, "F7");
  _hotkey->append(i++, "F8");
  _hotkey->append(i++, "F9");
  _hotkey->append(i++, "Ctrl+B");
  _hotkey->append(i++, "Ctrl+C");
  _hotkey->append(i++, "Ctrl+D");
  _hotkey->append(i++, "Ctrl+E");
  _hotkey->append(i++, "Ctrl+F");
  _hotkey->append(i++, "Ctrl+G");
  _hotkey->append(i++, "Ctrl+H");
  _hotkey->append(i++, "Ctrl+I");
  _hotkey->append(i++, "Ctrl+J");
  _hotkey->append(i++, "Ctrl+K");
  _hotkey->append(i++, "Ctrl+M");
  _hotkey->append(i++, "Ctrl+N");
  _hotkey->append(i++, "Ctrl+O");
  _hotkey->append(i++, "Ctrl+P");
  _hotkey->append(i++, "Ctrl+Q");
  _hotkey->append(i++, "Ctrl+R");
  _hotkey->append(i++, "Ctrl+T");
  _hotkey->append(i++, "Ctrl+U");
  _hotkey->append(i++, "Ctrl+V");
  _hotkey->append(i++, "Ctrl+W");
  _hotkey->append(i++, "Ctrl+X");
  _hotkey->append(i++, "Ctrl+Y");
  _hotkey->append(i++, "Ctrl+Z");
  _hotkey->append(i++, "Ctrl+0");
  _hotkey->append(i++, "Ctrl+1");
  _hotkey->append(i++, "Ctrl+2");
  _hotkey->append(i++, "Ctrl+3");
  _hotkey->append(i++, "Ctrl+4");
  _hotkey->append(i++, "Ctrl+5");
  _hotkey->append(i++, "Ctrl+6");
  _hotkey->append(i++, "Ctrl+7");
  _hotkey->append(i++, "Ctrl+8");
  _hotkey->append(i++, "Ctrl+9");

  _action->addColumn( tr("Action Name"),  200, Qt::AlignLeft );
  _action->addColumn( tr("Display Name"), -1,  Qt::AlignLeft );

  QStringList addedactions;
  XTreeWidgetItem *last = 0;
  QList<QMenu*> menulist = omfgThis->findChildren<QMenu*>();
  for(int m = 0; m < menulist.size(); ++m)
  {
    QList<QAction*> actionlist = menulist.at(m)->actions();
    for(int i = 0; i < actionlist.size(); ++i)
    {
      QAction* act = actionlist.at(i);
      if(!act->objectName().isEmpty())
      {
        if(!addedactions.contains(act->objectName()))
        {
          addedactions.append(act->objectName());
          last = new XTreeWidgetItem(_action, last, -1,
				     QVariant(act->objectName()),
				     act->text().remove("&")); 
        }
      }
    }
  }
  _action->sortItems(0,Qt::AscendingOrder);
}

hotkey::~hotkey()
{
  // no need to delete child widgets, Qt does it all for us
}

void hotkey::languageChange()
{
  retranslateUi(this);
}

enum SetResponse hotkey::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _hotkey->setEnabled(FALSE);
    }
  }

  param = pParams.value("username", &valid);
  if (valid)
  {
    _username = param.toString();
    _currentUser = FALSE;
  }

  param = pParams.value("currentUser", &valid);
  if (valid)
    _currentUser = TRUE;

  param = pParams.value("hotkey", &valid);
  if (valid)
  {
    QString value;

    if (_currentUser)
      value = _preferences->value(param.toString());
    else
    {
      q.prepare( "SELECT usrpref_value "
                 "FROM usrpref "
                 "WHERE ( (usrpref_username=:username)"
                 " AND (usrpref_name=:name) );" );
      q.bindValue(":username", _username);
      q.bindValue(":name", param.toString());
      q.exec();
      if (q.first())
        value = q.value("usrpref_value").toString();
//  ToDo
    }

    if (!value.isNull())
    {
      for (int i = 0; i < _action->topLevelItemCount(); i++)
      {
	XTreeWidgetItem *cursor = _action->topLevelItem(i);
        if (param.toString().left(1) == "F")
          _hotkey->setText(QString("F%1").arg(param.toString().right(1)));
        else if (param.toString().left(1) == "C")
          _hotkey->setText(QString("Ctrl+%1").arg(param.toString().right(1)));

        if (cursor->text(0) == value)
        {
          _action->setCurrentItem(cursor);
          _action->scrollToItem(cursor);
          break;
        }
      }
    }
  }

  return NoError;
}

void hotkey::sSave()
{
  QString keyValue;

  if(_action->currentItem() == 0)
  { 
    QMessageBox::information( this, tr("No Action Selected"),
      tr("You must select an Action before saving this Hotkey.") );
    return;
  }

  if (_hotkey->currentText().left(1) == tr("F"))
    keyValue = QString("F%1").arg(_hotkey->currentText().right(1));
  else if (_hotkey->currentText().left(5) == "Ctrl+")
    keyValue = QString("C%1").arg(_hotkey->currentText().right(1));

  if (_currentUser)
  {
    _preferences->set(keyValue, _action->currentItem()->text(0));
    _preferences->load();
  }
  else
  {
    q.prepare("SELECT setUserPreference(:username, :name, :value);");
    q.bindValue(":username", _username);
    q.bindValue(":name", keyValue);
    q.bindValue(":value", _action->currentItem()->text(0));
    q.exec();
  }

  accept();
}

