/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>

#include <xsqlquery.h>
#include <parameter.h>

#include "warehouseCluster.h"
#include "warehousegroup.h"

WarehouseGroup::WarehouseGroup(QWidget *pParent, const char *pName) :
  QGroupBox(pParent)
{
  _selectedOnly = false;
  if (_x_preferences)
    if (_x_preferences->boolean("selectedSites"))
      _selectedOnly=true;
  
  if(pName)
    setObjectName(pName);

  _fixed = true;
  _selectedGroup = new QWidget(this);
  QButtonGroup * buttonGroup = new QButtonGroup(this);
  
  _all = new QRadioButton(tr("All Sites"), this);
  _all->setObjectName("_all");
  _site = new QLabel(tr("Site:"),this);
  _site->setObjectName("_site");
  _selected = new QRadioButton(tr("Selected:"), _selectedGroup);
  _selected->setObjectName("_selected");
	 
  if (!_selectedOnly)
  {
    _all->setChecked(TRUE);
    buttonGroup->addButton(_all);
    buttonGroup->addButton(_selected);
  }
  
  _warehouses = new WComboBox(_selectedGroup, "_warehouses");

  if(_selectedOnly)
  {
    QHBoxLayout *hLayout = new QHBoxLayout(_selectedGroup);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(5);
    hLayout->addWidget(_site);
    hLayout->addWidget(_warehouses);
    hLayout->addStretch();
    _selectedGroup->setLayout(hLayout);
    
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(5, 5, 5, 5);
    vLayout->setSpacing(0);
    vLayout->addWidget(_selectedGroup);
    setLayout(vLayout);
    
    _all->hide();
    _selected->hide();
  }
  else
  {
    _site->hide();
    setFixedSize(false);
  }

  connect(_selected, SIGNAL(toggled(bool)), _warehouses, SLOT(setEnabled(bool)));
  connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(updated()));
  connect(_warehouses, SIGNAL(newID(int)), this, SIGNAL(updated()));

  if (((_x_preferences) ? _x_preferences->value("PreferredWarehouse").toInt() : -1) != -1)
    _selected->setChecked(TRUE);

  setTabOrder(_all, _selected);
  setTabOrder(_selected, _warehouses);
  setTabOrder(_warehouses, _all);
  setFocusProxy(_all);
  
  if (_x_metrics)
  {
    if (!_x_metrics->boolean("MultiWhs"))
    {
      this->hide();
      setAll();
    }
  }
}

void WarehouseGroup::setAll()
{
  _all->setChecked(TRUE);
}

void WarehouseGroup::setFixedSize(bool pFixed)
{
  if (pFixed == _fixed || _selectedOnly)
    return;
    
  if (layout())
    delete layout();
    
  if (_selectedGroup->layout())
    delete _selectedGroup->layout();
    
  if (pFixed)
  {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QHBoxLayout *hLayout = new QHBoxLayout(_selectedGroup);
    hLayout->setMargin(0);
    hLayout->setSpacing(5);
    hLayout->addWidget(_selected);
    hLayout->addWidget(_warehouses);
    _selectedGroup->setLayout(hLayout);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setMargin(5);
    vLayout->setSpacing(0);
    vLayout->addWidget(_all);
    vLayout->addWidget(_selectedGroup);
    setLayout(vLayout);
  }
  else
  {
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(_all,0,0);
    gridLayout->addWidget(_selected,1,0);
    gridLayout->addWidget(_warehouses,1,1);
#if defined Q_OS_MAC
    gridLayout->setRowMinimumHeight(0,24);
#endif
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addLayout(gridLayout);
    QSpacerItem * spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayout->addItem(spacerItem);
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayout);
    QSpacerItem * spacerItem1 = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vLayout->addItem(spacerItem1);
    QGridLayout *siteLayout = new QGridLayout(this);
    siteLayout->addLayout(vLayout,0,0);
  }
  _warehouses->setEnabled(FALSE);
  
  _fixed = pFixed;
}

void WarehouseGroup::setId(int pId)
{
  _selected->setChecked(TRUE);
  _warehouses->setId(pId);
}

void WarehouseGroup::appendValue(ParameterList &pParams)
{
  if (_selected->isChecked())
    pParams.append("warehous_id", _warehouses->id());
}

void WarehouseGroup::bindValue(XSqlQuery &pQuery)
{
  if (_selected->isChecked())
    pQuery.bindValue(":warehous_id", _warehouses->id());
}

void WarehouseGroup::findItemSites(int pItemid)
{
  _warehouses->findItemsites(pItemid);
}

int WarehouseGroup::id()
{
  if (_selected->isChecked())
    return _warehouses->id();
  else
    return -1;
}

bool WarehouseGroup::isAll() const
{
  return _all->isChecked();
}

bool WarehouseGroup::isSelected() const
{
  return _selected->isChecked();
}

