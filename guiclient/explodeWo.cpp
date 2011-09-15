/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "explodeWo.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "storedProcErrorLookup.h"

explodeWo::explodeWo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  QButtonGroup* _levelGroupInt = new QButtonGroup(this);
  _levelGroupInt->addButton(_singleLevel);
  _levelGroupInt->addButton(_multipleLevel);

  connect(_explode, SIGNAL(clicked()), this, SLOT(sExplode()));

  _captive = FALSE;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _multipleLevel->setChecked(_metrics->value("WOExplosionLevel") == "M");
  _wo->setType(cWoOpen);

  _wo->setFocus();
}

explodeWo::~explodeWo()
{
  // no need to delete child widgets, Qt does it all for us
}

void explodeWo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse explodeWo::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _explode->setFocus();
  }

  return NoError;
}

void explodeWo::sExplode()
{
  if (_wo->isValid())
  {
    q.prepare("SELECT explodeWo(:wo_id, :explodeChildren) AS result;");
    q.bindValue(":wo_id", _wo->id());
    q.bindValue(":explodeChildren", QVariant(_multipleLevel->isChecked()));
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this,
		    storedProcErrorLookup("explodeWo", result).arg(_wo->woNumber()),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);

    if (_captive)
      accept();
    else
    {
      _wo->setId(-1);
      _close->setText(tr("&Close"));
      _wo->setFocus();
    }
  }
}
