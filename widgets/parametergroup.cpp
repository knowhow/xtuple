/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include <QRadioButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>

#include <xsqlquery.h>
#include <parameter.h>

#include "xcombobox.h"

#include "parametergroup.h"

ParameterGroup::ParameterGroup(QWidget *pParent, const char *pName) :
  QGroupBox(pParent)
{
  _fixed = true;
  
  if(pName)
    setObjectName(pName);
  
  _type = AdhocGroup;

  _selectedGroup = new QWidget(this);
  _patternGroup = new QWidget(this);
  
  _all= new QRadioButton(QObject::tr("All"), this);
  _all->setObjectName("_all");
  _all->setChecked(TRUE);

  _selected = new QRadioButton(tr("Selected:"), this);
  _selected->setObjectName("_selected");
  
  _items = new XComboBox(FALSE, _selectedGroup);
  _items->setObjectName("_items");
  
  _items->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  _items->setEnabled(FALSE);
  
  _usePattern = new QRadioButton(tr("Pattern:"), this);
  _usePattern->setObjectName("_usePattern");
  
  _pattern = new QLineEdit(this);
  _pattern->setObjectName("_pattern");
  _pattern->setEnabled(FALSE);
  
  _buttonGroup = new QButtonGroup(this);
  
  setFixedSize(false);

  connect(_selected, SIGNAL(toggled(bool)), _items, SLOT(setEnabled(bool)));
  connect(_usePattern, SIGNAL(toggled(bool)), _pattern, SLOT(setEnabled(bool)));
  connect(_buttonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(updated()));
  connect(_items, SIGNAL(newID(int)), this, SIGNAL(updated()));
  connect(_pattern, SIGNAL(lostFocus()), this, SIGNAL(updated()));

  setFocusProxy(_all);
}

void ParameterGroup::setFixedSize(bool pFixed)
{
  if (pFixed == _fixed)
    return;
  
  if (layout())
    delete layout();
    
  if (_selectedGroup->layout())
    delete _selectedGroup->layout();
      
  if (_patternGroup->layout())
    delete _patternGroup->layout();
  
  if (pFixed)
  {
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    QHBoxLayout * hLayout = new QHBoxLayout(_selectedGroup);
    hLayout->setSpacing(5);
    hLayout->setMargin(0);
    hLayout->addWidget(_selected);
    hLayout->addWidget(_items);
    _selectedGroup->setLayout(hLayout);

    hLayout = new QHBoxLayout(_patternGroup);
    hLayout->setSpacing(5);
    hLayout->setMargin(0);
    hLayout->addWidget(_usePattern);
    hLayout->addWidget(_pattern);
    _patternGroup->setLayout(hLayout);

    QVBoxLayout * vLayout = new QVBoxLayout(this);
    vLayout->setSpacing(0);
    vLayout->setMargin(5);
    vLayout->addWidget(_all);
    vLayout->addWidget(_selectedGroup);
    vLayout->addWidget(_patternGroup);
    setLayout(vLayout);
  }
  else
  {
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    QGridLayout * gridLayout = new QGridLayout(this);
    QGridLayout * gridLayout1 = new QGridLayout();
  #if defined Q_OS_MAC
    gridLayout1->setRowMinimumHeight(0,24);
  #endif
    
    gridLayout1->addWidget(_all, 0, 0, 1, 2);
    
    gridLayout1->addWidget(_selected, 1, 0, 1, 1);

    QHBoxLayout * hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(5);
    hboxLayout->addWidget(_items);
    QSpacerItem * spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hboxLayout->addItem(spacerItem);
    gridLayout1->addLayout(hboxLayout, 1, 1, 1, 1);

    gridLayout1->addWidget(_usePattern, 2, 0, 1, 1);

    QHBoxLayout * hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(5);
    hboxLayout1->addWidget(_pattern);
    QSpacerItem * spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hboxLayout1->addItem(spacerItem1);
    gridLayout1->addLayout(hboxLayout1, 2, 1, 1, 1);
    
    QVBoxLayout * vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setSpacing(5);
    vboxLayout1->addLayout(gridLayout1);
    QSpacerItem * spacerItem2 = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vboxLayout1->addItem(spacerItem2);

    gridLayout->addLayout(vboxLayout1, 0, 0, 1, 1);
  }
  _buttonGroup->addButton(_all);
  _buttonGroup->addButton(_selected);
  _buttonGroup->addButton(_usePattern);
  _fixed = pFixed;
}

void ParameterGroup::setType(enum ParameterGroupTypes pType)
{
  if (_type == pType)
    return;

  _type = pType;

  switch (_type)
  {
    case AdhocGroup:
      _all->setText(QObject::tr("All"));
      break;

    case ClassCode:
      _all->setText(QObject::tr("All Class Codes"));
      _items->setType(XComboBox::ClassCodes);
      break;

    case PlannerCode:
      _all->setText(QObject::tr("All Planner Codes"));
      _items->setType(XComboBox::PlannerCodes);
      break;

    case ProductCategory:
      _all->setText(QObject::tr("All Product Categories"));
      _items->setType(XComboBox::ProductCategories);
      break;

    case ItemGroup:
      _all->setText(QObject::tr("All Item Groups"));
      _items->setType(XComboBox::ItemGroups);
      break;

    case CostCategory:
      _all->setText(QObject::tr("All Cost Categories"));
      _items->setType(XComboBox::CostCategories);
      break;

    case CustomerType:
      _all->setText(QObject::tr("All Customer Types"));
      _items->setType(XComboBox::CustomerTypes);
      break;

    case CustomerGroup:
      _all->setText(QObject::tr("All Customer Groups"));
      _items->setType(XComboBox::CustomerGroups);
      break;

    case CurrencyNotBase:
      _all->setText(QObject::tr("All Foreign Currencies"));
      _items->setType(XComboBox::CurrenciesNotBase);
      if (_items->isHidden()) // if there's only one currency defined
        hide();
      break;

    case Currency:
      _all->setText(QObject::tr("All Currencies"));
      _items->setType(XComboBox::Currencies);
      if (_items->isHidden()) // if there's only one currency defined
        hide();
      break;

    case WorkCenter:
      _all->setText(QObject::tr("All Work Centers"));
      _items->setType(XComboBox::WorkCenters);
      break;

    case User:
      _all->setText(QObject::tr("All Users"));
      _items->setType(XComboBox::Users);
	  break;

    case ActiveUser:
      _all->setText(QObject::tr("All Users"));
      _items->setType(XComboBox::ActiveUsers);
	  break;

    case OpportunitySource:
      _all->setText(QObject::tr("All Opportunity Sources"));
      _items->setType(XComboBox::OpportunitySources);
      break;

    case OpportunityStage:
      _all->setText(QObject::tr("All Opportunity Stages"));
      _items->setType(XComboBox::OpportunityStages);
      break;

    case OpportunityType:
      _all->setText(QObject::tr("All Opportunity Types"));
      _items->setType(XComboBox::OpportunityTypes);
      break;
  }
}

void ParameterGroup::setId(int pId)
{
  _selected->setChecked(TRUE);
  _items->setId(pId);
}

void ParameterGroup::setPattern(const QString &pPattern)
{
  _usePattern->setChecked(TRUE);
  _pattern->setText(pPattern);
}

void ParameterGroup::setState(enum ParameterGroupStates pState)
{
  switch(pState)
  {
    case Selected:
      _selected->setChecked(true);
      break;
    case Pattern:
      _usePattern->setChecked(true);
      break;
    case All:
    default:
      _all->setChecked(true);
  }
}

void ParameterGroup::appendValue(ParameterList &pParams)
{
  if (_selected->isChecked())
  {
    if (_type == ClassCode)
      pParams.append("classcode_id", _items->id());
    else if (_type == PlannerCode)
      pParams.append("plancode_id", _items->id());
    else if (_type == ProductCategory)
      pParams.append("prodcat_id", _items->id());
    else if (_type == ItemGroup)
      pParams.append("itemgrp_id", _items->id());
    else if (_type == CostCategory)
      pParams.append("costcat_id", _items->id());
    else if (_type == CustomerType)
      pParams.append("custtype_id", _items->id());
    else if (_type == CustomerGroup)
      pParams.append("custgrp_id", _items->id());
    else if (_type == CurrencyNotBase || _type == Currency)
      pParams.append("curr_id", _items->id());
    else if (_type == WorkCenter)
      pParams.append("wrkcnt_id", _items->id());
    else if (_type == OpportunitySource)
      pParams.append("opsource_id", _items->id());
    else if (_type == OpportunityStage)
      pParams.append("opstage_id", _items->id());
    else if (_type == OpportunityType)
      pParams.append("optype_id", _items->id());
    else if (_type == User || _type == ActiveUser )
    {
      pParams.append("usr_id", _items->id());
      pParams.append("username", _items->itemText(_items->currentIndex()));
    }
  }
  else if (_usePattern->isChecked() && !_pattern->text().isEmpty())
  {
    if (_type == ClassCode)
      pParams.append("classcode_pattern", _pattern->text());
    else if (_type == PlannerCode)
      pParams.append("plancode_pattern", _pattern->text());
    else if (_type == ProductCategory)
      pParams.append("prodcat_pattern", _pattern->text());
    else if (_type == ItemGroup)
      pParams.append("itemgrp_pattern", _pattern->text());
    else if (_type == CostCategory)
      pParams.append("costcat_pattern", _pattern->text());
    else if (_type == CustomerType)
      pParams.append("custtype_pattern", _pattern->text());
    else if (_type == CustomerGroup)
      pParams.append("custgrp_pattern", _pattern->text());
    else if (_type == CurrencyNotBase || _type == Currency)
      pParams.append("currConcat_pattern", _pattern->text());
    else if (_type == WorkCenter)
      pParams.append("wrkcnt_pattern", _pattern->text());
    else if (_type == OpportunitySource)
      pParams.append("opsource_pattern", _pattern->text());
    else if (_type == OpportunityStage)
      pParams.append("opstage_pattern", _pattern->text());
    else if (_type == OpportunityType)
      pParams.append("optype_pattern", _pattern->text());
    else if (_type == User || _type == ActiveUser)
      pParams.append("usr_pattern", _pattern->text());
  }
}

void ParameterGroup::repopulateSelected()
{
    _items->populate();
}

void ParameterGroup::bindValue(XSqlQuery &pQuery)
{
  if (_selected->isChecked())
  {
    QString name;

    switch (_type)
    {
      case ClassCode:
        name = ":classcode_id";   break;

      case PlannerCode:
        name = ":plancode_id";    break;

      case ProductCategory:
        name = ":prodcat_id";     break;

      case ItemGroup:
        name = ":itemgrp_id";     break;

      case CostCategory:
        name = ":costcat_id";     break;

      case CustomerType:
        name = ":custtype_id";    break;

      case CustomerGroup:
        name = ":custgrp_id";     break;

      case CurrencyNotBase:
      case Currency:
        name = ":curr_id";        break;

      case WorkCenter:
        name = ":wrkcnt_id";      break;

      case OpportunitySource:
        name = ":opsource_id";      break;

      case OpportunityStage:
        name = ":opstage_id";      break;

      case OpportunityType:
        name = ":optype_id";      break;

      case User:
        name = ":usr_id";         break;

      case AdhocGroup:
        name = ":id";             break;

      default:
        break;
    }

    pQuery.bindValue(name, _items->id());
  }
  else if (_usePattern->isChecked() && !_pattern->text().isEmpty())
  {
    QString name;

    switch (_type)
    {
      case ClassCode:
        name = ":classcode_pattern";   break;

      case PlannerCode:
        name = ":plancode_pattern";    break;

      case ProductCategory:
        name = ":prodcat_pattern";     break;

      case ItemGroup:
        name = ":itemgrp_pattern";     break;

      case CostCategory:
        name = ":costcat_pattern";     break;

      case CustomerType:
        name = ":custtype_pattern";    break;

      case CustomerGroup:
        name = ":custgrp_pattern";     break;

      case CurrencyNotBase:
      case Currency:
        name = ":currConcat_pattern";  break;

      case WorkCenter:
        name = ":wrkcnt_pattern";      break;

      case OpportunitySource:
        name = ":opsource_pattern";      break;

      case OpportunityStage:
        name = ":opstage_pattern";      break;

      case OpportunityType:
        name = ":optype_pattern";      break;

      case User:
        name = ":usr_pattern";         break;

      case AdhocGroup:
        name = ":pattern";             break;

      default:
        break;
    }

    pQuery.bindValue(name, _pattern->text());
  }
}

enum ParameterGroup::ParameterGroupStates ParameterGroup::state()
{
  if (_all->isChecked())
    return All;
  else if (_selected->isChecked())
    return Selected;
  else
    return Pattern;
}

int ParameterGroup::id()
{
  if (_selected->isChecked())
    return _items->id();
  else
    return -1;
}

QString ParameterGroup::pattern()
{
  if (_usePattern->isChecked())
    return _pattern->text();
  else
    return QString::null;
}

QString ParameterGroup::selected()
{
  if (_selected->isChecked())
    return _items->code();
  else
    return QString::null;
}

void ParameterGroup::setSelected(const QString &p)
{
  _selected->setChecked(TRUE);
  _items->setCode(p);
}
