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

#include "vendorgroup.h"

VendorGroup::VendorGroup(QWidget *pParent, const char *pName) : QWidget(pParent)
{
  setObjectName(pName);

  setupUi(this);

  setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

  _select->setCurrentIndex(All);
  _vend->setId(-1);
  _vendorTypes->setId(-1);
  _vendorType->setText("");

  connect(_select,SIGNAL(currentIndexChanged(int)), this, SIGNAL(updated()));
  connect(_select,SIGNAL(currentIndexChanged(int)), this, SIGNAL(newState(int)));
  connect(_vend,                SIGNAL(newId(int)), this, SIGNAL(updated()));
  connect(_vend,                SIGNAL(newId(int)), this, SIGNAL(newVendId(int)));
  connect(_vendorTypes,         SIGNAL(newID(int)), this, SIGNAL(updated()));
  connect(_vendorTypes,         SIGNAL(newID(int)), this, SIGNAL(newVendTypeId(int)));
  connect(_vendorType,   SIGNAL(editingFinished()), this, SIGNAL(updated()));
  connect(_vendorType,   SIGNAL(editingFinished()), this, SLOT(sTypePatternFinished()));
  connect(_vendorType,SIGNAL(textChanged(QString)), this, SIGNAL(newTypePattern(QString)));

  setFocusProxy(_select);
}

VendorGroup::~VendorGroup()
{
}

void VendorGroup::languageChange()
{
  retranslateUi(this);
}

void VendorGroup::appendValue(ParameterList &pParams)
{
  switch (_select->currentIndex())
  {
    case Selected:
      pParams.append("vend_id", _vend->id());
      break;
    case SelectedType:
      pParams.append("vendtype_id", _vendorTypes->id());
      break;
    case TypePattern:
      pParams.append("vendtype_pattern", _vendorType->text());
      break;
    case All:
    default:
      ; // nothing
  }
}

void VendorGroup::bindValue(XSqlQuery &pQuery)
{
  switch (_select->currentIndex())
  {
    case Selected:
      pQuery.bindValue(":vend_id", _vend->id());
      break;
    case SelectedType:
      pQuery.bindValue(":vendtype_id", _vendorTypes->id());
      break;
    case TypePattern:
      pQuery.bindValue(":vendtype_pattern", _vendorType->text());
      break;
    case All:
    default:
      ; // nothing
  }
}

bool VendorGroup::isValid()
{
  switch (_select->currentIndex())
  {
    case Selected:
      return _vend->isValid();
      break;
    case SelectedType:
      return _vendorTypes->isValid();
      break;
    case TypePattern:
      return ! _vendorType->text().trimmed().isEmpty();
      break;
    case All:
      return true;
      break;
  }

  return false;
}

void VendorGroup::setVendId(int p)
{
  _vend->setId(p);
  setState(Selected);
}

void VendorGroup::setVendTypeId(int p)
{
  _vendorTypes->setId(p);
  setState(SelectedType);
}

void VendorGroup::setTypePattern(const QString &p)
{
  _vendorType->setText(p);
  sTypePatternFinished();
  setState(TypePattern);
}

void VendorGroup::setState(enum VendorGroupState p)
{
  _select->setCurrentIndex(p);
}

void VendorGroup::sTypePatternFinished()
{
  emit newTypePattern(_vendorType->text());
  emit updated();
}

/*
   keep the contents of another vendor group (p) synchronized with this in a
   manner which allows any changes on this to propagate to p and any objects
   listening for signals from p will respond to changes to this.
   hide p because it's superfluous.
*/
void VendorGroup::synchronize(VendorGroup *p)
{
  connect(this, SIGNAL(newTypePattern(QString)), p, SLOT(setTypePattern(QString)));
  connect(this, SIGNAL(newState(int)),           p, SLOT(setState(int)));
  connect(this, SIGNAL(newVendId(int)),          p, SLOT(setVendId(int)));
  connect(this, SIGNAL(newVendTypeId(int)),      p, SLOT(setVendTypeId(int)));
  p->hide();
}
