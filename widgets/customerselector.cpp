/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <xsqlquery.h>
#include <parameter.h>

#include "customerselector.h"

CustomerSelector::CustomerSelector(QWidget *pParent, const char *pName) : QWidget(pParent)
{
  setupUi(this);

  if (pName)
    setObjectName(pName);

  setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

  _select->setCurrentIndex(All);
  _cust->setId(-1);
  _customerTypes->setId(-1);
  _customerGroup->setId(-1);
  _customerType->setText("");

  connect(_select,SIGNAL(currentIndexChanged(int)), this, SIGNAL(updated()));
  connect(_select,SIGNAL(currentIndexChanged(int)), this, SIGNAL(newState(int)));
  connect(_cust,                SIGNAL(newId(int)), this, SIGNAL(updated()));
  connect(_cust,                SIGNAL(newId(int)), this, SIGNAL(newCustId(int)));
  connect(_customerTypes,         SIGNAL(newID(int)), this, SIGNAL(updated()));
  connect(_customerTypes,         SIGNAL(newID(int)), this, SIGNAL(newCustTypeId(int)));
  connect(_customerType,   SIGNAL(editingFinished()), this, SIGNAL(updated()));
  connect(_customerType,   SIGNAL(editingFinished()), this, SLOT(sTypePatternFinished()));
  connect(_customerType,SIGNAL(textChanged(QString)), this, SIGNAL(newTypePattern(QString)));
  connect(_customerGroup,         SIGNAL(newID(int)), this, SIGNAL(updated()));
  connect(_customerGroup,         SIGNAL(newID(int)), this, SIGNAL(newCustGroupId(int)));

  setFocusProxy(_select);
}

CustomerSelector::~CustomerSelector()
{
}

void CustomerSelector::languageChange()
{
  retranslateUi(this);
}

void CustomerSelector::appendValue(ParameterList &pParams)
{
  switch (_select->currentIndex())
  {
    case Selected:
      pParams.append("cust_id", _cust->id());
      break;
    case SelectedGroup:
    pParams.append("custgrp_id", _customerGroup->id());
      break;
    case SelectedType:
      pParams.append("custtype_id", _customerTypes->id());
      break;
    case TypePattern:
      pParams.append("custtype_pattern", _customerType->text());
      break;
    case All:
    default:
      ; // nothing
  }
}

void CustomerSelector::bindValue(XSqlQuery &pQuery)
{
  switch (_select->currentIndex())
  {
    case Selected:
      pQuery.bindValue(":cust_id", _cust->id());
      break;
    case SelectedGroup:
      pQuery.bindValue(":custgrp_id", _customerGroup->id());
      break;
    case SelectedType:
      pQuery.bindValue(":custtype_id", _customerTypes->id());
      break;
    case TypePattern:
      pQuery.bindValue(":custtype_pattern", _customerType->text());
      break;
    case All:
    default:
      ; // nothing
  }
}

bool CustomerSelector::isValid()
{
  switch (_select->currentIndex())
  {
    case Selected:
      return _cust->isValid();
      break;
    case SelectedGroup:
      return _customerGroup->isValid();
      break;
    case SelectedType:
      return _customerTypes->isValid();
      break;
    case TypePattern:
      return ! _customerType->text().trimmed().isEmpty();
      break;
    case All:
      return true;
      break;
  }

  return false;
}

void CustomerSelector::setCustId(int p)
{
  _cust->setId(p);
  setState(Selected);
}

void CustomerSelector::setCustTypeId(int p)
{
  _customerTypes->setId(p);
  setState(SelectedType);
}

void CustomerSelector::setTypePattern(const QString &p)
{
  _customerType->setText(p);
  sTypePatternFinished();
  setState(TypePattern);
}

void CustomerSelector::setCustGroupId(int p)
{
  _customerGroup->setId(p);
  setState(SelectedGroup);
}

void CustomerSelector::setState(enum CustomerSelectorState p)
{
  _select->setCurrentIndex(p);
}

void CustomerSelector::sTypePatternFinished()
{
  emit newTypePattern(_customerType->text());
  emit updated();
}

/*
   keep the contents of another customer group (p) synchronized with this in a
   manner which allows any changes on this to propagate to p and any objects
   listening for signals from p will respond to changes to this.
   hide p because it's superfluous.
*/
void CustomerSelector::synchronize(CustomerSelector *p)
{
  connect(this, SIGNAL(newTypePattern(QString)), p, SLOT(setTypePattern(QString)));
  connect(this, SIGNAL(newState(int)),           p, SLOT(setState(int)));
  connect(this, SIGNAL(newCustId(int)),          p, SLOT(setCustId(int)));
  connect(this, SIGNAL(newCustTypeId(int)),      p, SLOT(setCustTypeId(int)));
  connect(this, SIGNAL(newCustGroupId(int)),     p, SLOT(setCustGroupId(int)));

  p->hide();
}
