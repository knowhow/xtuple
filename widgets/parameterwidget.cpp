/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <parameter.h>
#include <xsqlquery.h>

#include <QCheckBox>
#include <QtScript>
#include <QMessageBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDebug>

#ifdef Q_WS_MAC
#include <QFont>
#endif

#include "parameterwidget.h"
#include "widgets.h"
#include "xcombobox.h"
#include "usernamecluster.h"
#include "datecluster.h"
#include "crmacctcluster.h"
#include "filterManager.h"
#include "contactcluster.h"
#include "filtersave.h"
#include "glcluster.h"
#include "projectcluster.h"
#include "custcluster.h"
#include "warehouseCluster.h"
#include "vendorcluster.h"
#include "itemcluster.h"
#include "empcluster.h"
#include "shiptocluster.h"
#include "ordercluster.h"
#include "wocluster.h"

#define DEBUG false

/* TODO: - add Flag to the set of types to handle parameters that are either
           passed or not (this is different from boolean parameters, that can
           be either true or false).
         - add XComboBoxCode to the set of types to handle parameters that
           require the code value from an xcombobox instead of the id.
         - store the text value of the enum in the filter definition, not the
           integer. otherwise the db contents will prevent reorganizing the
           order of the symbolic values of the type enumeration.
         - rename methods to align better with qt naming conventions
         - find a solution to the problem that multiselect's use of
           qtablewidget violates the one-unit-high rule
 */

/** \ingroup widgets

    \class ParameterWidget

    \brief The ParameterWidget provides a consistent user and
           programming interface for managing query criteria.

    The ParameterWidget can be used to set filtering criteria to
    apply to MetaSQL queries. The application developer defines
    what criteria can be used (e.g. filter by a date range), what
    the default values should be, if any, and which criteria are
    mandatory.  The user then chooses which criteria to actually
    use at runtime and supplies values (e.g. 'January 1, 2010'
    through 'December 31, 2010').

    It is the application developer's responsibility to ensure that
    the user's selections are honored by the query.

    Specific sets of filtering criteria can be saved, reused, and
    shared between users. The details of the filter can be hidden
    from view or made visible, allowing for efficient use of screen
    real estate.

    If application preferences are available, the most recently
    selected filter will saved when the parent window closes and
    will be reselected when the window is next opened.

 */

/** \brief Create a new Parameter Widget.
    \param pParent The parent of this widget
    \param pName   The object name to assign to this widget
  */
ParameterWidget::ParameterWidget(QWidget *pParent, const char *pName)  :
    QWidget(pParent)
{
  setupUi(this);


  if(pName)
    setObjectName(pName);

  _initialized = false;
  _shared = false;
  setSavedFilters(-1);
  _filterSignalMapper = new QSignalMapper(this);
  _saveButton->setDisabled(true);

  _filterGroup->setVisible(false);
  _filterButton->setChecked(false);

  connect(_addFilterRow, SIGNAL(clicked()), this, SLOT( addParam() ) );
  connect(_filterSignalMapper, SIGNAL(mapped(int)), this, SLOT( removeParam(int) ));
  connect(_saveButton, SIGNAL(clicked()), this, SLOT( save() ) );
  connect(_manageButton, SIGNAL(clicked()), this, SLOT( sManageFilters() ) );
  connect(_filterList, SIGNAL(currentIndexChanged(int)), this, SLOT( applySaved(int) ) );
  connect(_filterList, SIGNAL(currentIndexChanged(int)), this, SLOT( setFiltersDefault() ) );
  connect(this, SIGNAL(updated()), this, SLOT( toggleSave() ) );
  connect(_filterButton, SIGNAL(toggled(bool)), this, SLOT(setFiltersVisible(bool)));

  _saveButton->setShortcut(QKeySequence::Save);
  _saveButton->setToolTip(_saveButton->text()
                          .remove("&")
                          .append(" ")
                          .append(_saveButton->shortcut().toString(QKeySequence::NativeText)));
}

void ParameterWidget::showEvent(QShowEvent * event)
{

  if(!_initialized)
  {
    QString pname;
    if(window())
      pname = window()->objectName() + "/";
    _settingsName = pname + objectName();
    if(_x_preferences)
      setFiltersVisible(_x_preferences->boolean(_settingsName + "/checked"));
    _initialized = true;
  }
  QWidget::showEvent(event);

}

/** \brief Add name/value pairs to the input ParameterList
	   to reflect the current user selections.

    The ParameterList passed in to this method is modified to reflect
    the current selections. Parameters already in the list have
    their associated values changed and parameters not in the list
    are added. No name/value pairs are removed from the list.

    This should be used carefully, as cached ParameterLists may have
    name/value pairs that the caller does not expect.

    \param[in,out] pParams The ParameterList to modify

*/
void ParameterWidget::appendValue(ParameterList &pParams)
{
  QMapIterator<int, QPair<QString, QVariant> > i(_filterValues);
  while (i.hasNext())
  {
    i.next();
    QPair<QString, QVariant> tempPair = i.value();
    if (pParams.inList(tempPair.first))
    {
      pParams.remove(tempPair.first);
    }

    pParams.append(tempPair.first, tempPair.second);
  }
}

/** \brief Find the default filter set for the ParameterWidget with
           the current parent and apply it.

    This method searches for the default filter set based on the
    QObject::objectName of the parent and the QObject::objectName
    of this ParameterWidget instance. If it is found, then this
    filter set is applied.

*/

void ParameterWidget::applyDefaultFilterSet()
{
  XSqlQuery qry;
  QString filter_name;
  int filter_id = -1;
  QString pname;

  if(window())
    pname = window()->objectName() + "/";
  _settingsName2 = pname + this->objectName();

  if(_x_preferences)
  {
    _settingsName2 += "/filter_default";
    filter_id = _x_preferences->value(_settingsName2).toInt();
  }

  QString query = "SELECT filter_name "
                  "FROM filter "
                  "WHERE filter_id=:id ";

  if (parent())
  {
    QString classname(parent()->objectName());
    if (classname.isEmpty())
      classname = parent()->metaObject()->className();

    qry.prepare(query);
    qry.bindValue(":id", filter_id);
    qry.exec();

    if (qry.first())
    {
      filter_name = qry.value("filter_name").toString();
      setSavedFiltersIndex(filter_name);
      applySaved(0, filter_id);
    }
    else
    {
      addParam();
      applySaved(0, 0);
    }
  }
}

void ParameterWidget::addParam()
{
  XComboBox *xcomboBox = new XComboBox(_filterGroup);
  QToolButton *toolButton = new QToolButton(_filterGroup);
  QLineEdit *lineEdit = new QLineEdit(_filterGroup);
  QGridLayout *gridLayout = new QGridLayout();
  QVBoxLayout *xcomboLayout = new QVBoxLayout();
  QHBoxLayout *widgetLayout1 = new QHBoxLayout();
  QVBoxLayout *widgetLayout2 = new QVBoxLayout();
  QVBoxLayout *buttonLayout = new QVBoxLayout();

  int nextRow = _filtersLayout->rowCount();
  QString currRow = QString().setNum(nextRow);

  // Set up objects
  gridLayout->setObjectName("topLayout" + currRow);

  xcomboLayout->setObjectName("xcomboLayout" + currRow);
  xcomboLayout->setContentsMargins(0, 0, 0, 0);

  xcomboBox->setObjectName("xcomboBox" + currRow);
  xcomboBox->addItem("", currRow + ":" + "2");

  widgetLayout1->setObjectName("widgetLayout1" + currRow);

  widgetLayout2->setObjectName("widgetLayout2" + currRow);
  widgetLayout2->setContentsMargins(0, 0, 0, 0);

  lineEdit->setObjectName("widget" + currRow);
  lineEdit->setDisabled(true);

  buttonLayout->setObjectName("buttonLayout" + currRow);
  buttonLayout->setContentsMargins(0, 0, 0, 0);

  toolButton->setObjectName("button" + currRow);
  toolButton->setText(tr("-"));
#ifdef Q_WS_MAC
  toolButton->setMinimumWidth(25);
#else
  toolButton->setMinimumWidth(21);
#endif

  //grab the items provided by other widgets to populate xcombobox with
  QMapIterator<int, ParamProps* > i(_params);
  while (i.hasNext())
  {
    i.next();
    ParamProps* pp = i.value();
    QString value = QString().setNum(nextRow) + ":" + QString().setNum(pp->paramType);
    if ( _usedTypes.isEmpty() || !containsUsedType(pp->name) )
      xcomboBox->addItem(pp->name, value );
  }

  xcomboLayout->addWidget(xcomboBox);
  xcomboLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

  // Place the default line edit/button combo
  widgetLayout1->addWidget(lineEdit);
  widgetLayout1->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed));

  widgetLayout2->addLayout(widgetLayout1);
  widgetLayout2->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

  gridLayout->addLayout(widgetLayout2, 0, 0, 1, 1);

  // Place Button
  buttonLayout->addWidget(toolButton);
  buttonLayout->addItem(new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

  gridLayout->addLayout(buttonLayout, 0, 1, 1, 1);

  _filtersLayout->addLayout(gridLayout, nextRow, 1, 1, 1);
  _filtersLayout->addLayout(xcomboLayout, nextRow, 0, 1, 1);


  // Hook up connections
  connect(toolButton, SIGNAL(clicked()), _filterSignalMapper, SLOT(map()));
  connect(toolButton, SIGNAL(clicked()), gridLayout, SLOT( deleteLater() ) );
  connect(toolButton, SIGNAL(clicked()), xcomboBox, SLOT( deleteLater() ) );
  connect(toolButton, SIGNAL(clicked()), lineEdit, SLOT( deleteLater() ) );
  connect(toolButton, SIGNAL(clicked()), toolButton, SLOT( deleteLater() ) );
  connect(xcomboBox, SIGNAL(currentIndexChanged(int)), this, SLOT( addUsedType() ) );
  connect(xcomboBox, SIGNAL(currentIndexChanged(int)), this, SLOT( changeFilterObject(int)) );
  connect(xcomboBox, SIGNAL(currentIndexChanged(int)), this, SLOT( repopulateComboboxes() ) );
  connect(lineEdit, SIGNAL(editingFinished()), this, SLOT( storeFilterValue() ) );

  _filterSignalMapper->setMapping(toolButton, nextRow);

  _addFilterRow->setDisabled(true);

}

/** \brief Add a new filter to the set of available filters.

  \param pName      The user-visible name for this filter to distinguish
                    it from other filters in the same set
  \param pParam     The name to use for the parameter in the ParameterList
  \param pType      The type of widget for the user to enter a criterion in
  \param pDefault   The default value to use for this parameter
  \param pRequired  Required parameters cannot be removed from the filter set
  \param pExtraInfo A query string to use if pType indicates a combo box or multiselect
*/
void ParameterWidget::append(QString pName, QString pParam, ParameterWidgetTypes pType, QVariant pDefault, bool pRequired, QString pExtraInfo)
{
  if (DEBUG)
    qDebug("%s::append(%s, %s, %d, %s, %d, %s) entered",
           qPrintable(objectName()), qPrintable(pName), qPrintable(pParam),
           pType, qPrintable(pDefault.toString()), pRequired,
           qPrintable(pExtraInfo.left(50)));
  ParamProps *pp = _params.value(_params.count(), 0);

  if (! pp)
  {
    if (DEBUG)
      qDebug("%s::append() instantiating a new ParamProps",
             qPrintable(objectName()));
    pp = new ParamProps();
    pp->name = pName;
    pp->param = pParam;
    pp->paramType = pType;
    pp->defaultValue = pDefault;
    pp->required = pRequired;
    pp->comboType = XComboBox::Adhoc;
    pp->query = pExtraInfo;
    pp->enabled = true;
    _params.insert(_params.count(), pp);
  }

  if (pType == Multiselect && !pExtraInfo.length())
  {
      qWarning("%s::setType(%s, %s, %d, %s, %s) called for Multiselect but "
               "was not given a query to use",
               qPrintable(objectName()), qPrintable(pName), qPrintable(pParam),
               pType, qPrintable(pDefault.toString()),
               qPrintable(pExtraInfo));
  }

}

/** \brief Add a new XComboBox-based filter to the set of available filters.

  \param pName      The user-visible name for this filter to distinguish
                    it from other filters in the same set
  \param pParam     The name to use for the parameter in the ParameterList
  \param pType      Set the XComboBox to use the corresponding
                    XComboBox::XComboBoxTypes value
  \param pDefault   The default internal id to select for this XComboBox
  \param pRequired  Required parameters cannot be removed from the filter set
*/
void ParameterWidget::appendComboBox(QString pName, QString pParam, int pType, QVariant pDefault, bool pRequired)
{
  ParamProps *pp = _params.value(_params.count(), 0);

  if (! pp)
  {
    pp = new ParamProps();
    pp->name = pName;
    pp->param = pParam;
    pp->paramType = XComBox;
    pp->defaultValue = pDefault;
    pp->required = pRequired;
    pp->comboType = (XComboBox::XComboBoxTypes)pType;
    pp->enabled = true;
    _params.insert(_params.count(), pp);
  }
}

/** \brief Add a new XComboBox-based filter to the set of available filters.

  \param pName      The user-visible name for this filter to distinguish
                    it from other filters in the same set
  \param pParam     The name to use for the parameter in the ParameterList
  \param pQuery     Set the XComboBox to populate itself with this query
  \param pDefault   The default internal id to select for this XComboBox
  \param pRequired  Required parameters cannot be removed from the filter set
*/
void ParameterWidget::appendComboBox(QString pName, QString pParam, QString pQuery, QVariant pDefault, bool pRequired)
{
  ParamProps *pp = _params.value(_params.count(), 0);

  if (! pp)
  {
    pp = new ParamProps();
    pp->name = pName;
    pp->param = pParam;
    pp->paramType = XComBox;
    pp->defaultValue = pDefault;
    pp->required = pRequired;
    pp->comboType = XComboBox::Adhoc;
    pp->query = pQuery;
    pp->enabled = true;
    _params.insert(_params.count(), pp);
  }
}

void ParameterWidget::applySaved(int pId, int filter_id)
{
  QWidget *found = 0;
  XSqlQuery qry;
  QString query;
  QString filterValue;
  QDate today = QDate::currentDate();
  int xid, init_filter_id;

  init_filter_id = filter_id;

  clearFilters();

  if (!parent())
    return;

  if (_filterList->id() == -1)
  {
    emit updated();
    return;
  }

  if (filter_id == 0 && _filterList->id() != -1)
    filter_id = _filterList->id(_filterList->currentIndex());

  QString classname(parent()->objectName());
  if (classname.isEmpty())
    classname = parent()->metaObject()->className();

  query = " SELECT filter_value, "
          "  CASE WHEN (filter_username IS NULL) THEN true "
          "  ELSE false END AS shared "
          " FROM filter "
          " WHERE filter_id=:id ";

  qry.prepare(query);
  qry.bindValue(":id", filter_id );

  qry.exec();

  if (qry.first())
  {
    filterValue = qry.value("filter_value").toString();
    _shared = qry.value("shared").toBool();
  }


  QStringList filterRows = filterValue.split("|");
  QString tempFilter = QString();

  int windowIdx = _filtersLayout->rowCount();

  if (filterRows.size() == 1  && pId == 0 && filter_id != 0)
  {
    emit updated();
    return;
  }

  for (int i = 0; i < filterRows.size(); ++i)
  {
    tempFilter = filterRows[i];
    if ( !(tempFilter.isEmpty()) )
    {
      //0 is filterType, 1 is filterValue, 2 is parameterwidgettype
      QStringList tempFilterList = tempFilter.split(":");
      QString key = this->getParameterTypeKey(tempFilterList[0]);
      if (key.isEmpty())
      {
        //parametertype is no longer found, prompt user to delete filter
        if (!isVisible() ||
            QMessageBox::question(this, tr("Invalid Filter Set"), tr("This filter set contains an obsolete filter and will be deleted. Do you want to do this?"),
                                  QMessageBox::No | QMessageBox::Default,
                                  QMessageBox::Yes) == QMessageBox::No)
          return;
        else
        {
          QString query = "delete from filter where filter_id=:filter_id";
          XSqlQuery qry;

          qry.prepare(query);
          qry.bindValue(":filter_id", filter_id);
          qry.exec();

          setSavedFilters();
          return;
        }
      }
      else
      {
        addParam();

        QLayoutItem *test = _filtersLayout->itemAtPosition(windowIdx, 0)->layout()->itemAt(0);
        XComboBox *mybox = (XComboBox*)test->widget();

        int idx = mybox->findText(key);

        mybox->setCurrentIndex(idx);
	
        found = getFilterWidget(windowIdx);

        int widgetType = tempFilterList[2].toInt();

        //grab pointer to newly created filter object
        switch (widgetType)
        {
        case Date:
          DLineEdit *dLineEdit;
          dLineEdit = qobject_cast<DLineEdit*>(found);
          if (dLineEdit != 0)
            dLineEdit->setDate(today.addDays(tempFilterList[1].toInt()), true);
          break;
        case User:
          UsernameCluster *usernameCluster;
          usernameCluster = qobject_cast<UsernameCluster*>(found);
          if (usernameCluster != 0)
            usernameCluster->setUsername(tempFilterList[1]);
          break;
        case Crmacct:
          CRMAcctCluster *crmacctCluster;
          crmacctCluster = qobject_cast<CRMAcctCluster*>(found);
          if (crmacctCluster != 0)
            crmacctCluster->setId(tempFilterList[1].toInt());
          break;
        case Customer:
          CustCluster *custCluster;
          custCluster = qobject_cast<CustCluster*>(found);
          if (custCluster != 0)
            custCluster->setId(tempFilterList[1].toInt());
          break;
        case Shipto:
          ShiptoCluster *shiptoCluster;
          shiptoCluster = qobject_cast<ShiptoCluster*>(found);
          if (shiptoCluster != 0)
            shiptoCluster->setId(tempFilterList[1].toInt());
          break;
        case Vendor:
          VendorCluster *vendCluster;
          vendCluster = qobject_cast<VendorCluster*>(found);
          if (vendCluster != 0)
            vendCluster->setId(tempFilterList[1].toInt());
          break;
        case Contact:
          ContactCluster *contactCluster;
          contactCluster = qobject_cast<ContactCluster*>(found);
          if (contactCluster != 0)
            contactCluster->setId(tempFilterList[1].toInt());
          break;
        case GLAccount:
          GLCluster *glCluster;
          glCluster = qobject_cast<GLCluster*>(found);
          if (glCluster != 0)
            glCluster->setId(tempFilterList[1].toInt());
          break;
        case Project:
          ProjectCluster *projectCluster;
          projectCluster = qobject_cast<ProjectCluster*>(found);
          if (projectCluster != 0)
            projectCluster->setId(tempFilterList[1].toInt());
          break;
        case Item:
          ItemCluster *itemCluster;
          itemCluster = qobject_cast<ItemCluster*>(found);
          if (itemCluster != 0)
            itemCluster->setId(tempFilterList[1].toInt());
          break;
        case Employee:
          EmpCluster *employeeCluster;
          employeeCluster = qobject_cast<EmpCluster*>(found);
          if (employeeCluster != 0)
            employeeCluster->setId(tempFilterList[1].toInt());
          break;
        case SalesOrder:
          OrderCluster *soCluster;
          soCluster = qobject_cast<OrderCluster*>(found);
          if (soCluster != 0)
            soCluster->setId(tempFilterList[1].toInt());
          break;
        case PurchaseOrder:
          OrderCluster *poCluster;
          poCluster = qobject_cast<OrderCluster*>(found);
          if (poCluster != 0)
            poCluster->setId(tempFilterList[1].toInt());
          break;
        case TransferOrder:
          OrderCluster *toCluster;
          toCluster = qobject_cast<OrderCluster*>(found);
          if (toCluster != 0)
            toCluster->setId(tempFilterList[1].toInt());
          break;
        case WorkOrder:
          WoCluster *woCluster;
          woCluster = qobject_cast<WoCluster*>(found);
          if (woCluster != 0)
            woCluster->setId(tempFilterList[1].toInt());
          break;
        case Site:
          WComboBox *wBox;
          wBox = qobject_cast<WComboBox*>(found);
          if (wBox != 0)
            wBox->setId(tempFilterList[1].toInt());
          break;
        case XComBox:
          XComboBox *xBox;
          xBox = qobject_cast<XComboBox*>(found);
          if (xBox != 0)
          {
            //fix for setid not emitting id signal if id found for filter is first in list
            //set to any other valid id first to fix it
            xBox->setId(2);

            xid = tempFilterList[1].toInt();
            xBox->setId(xid);
          }
          break;
         case Multiselect:
          {
            QTableWidget *tab;
            tab = qobject_cast<QTableWidget*>(found);
            if (tab != 0)
            {
              QStringList   savedval = tempFilterList[1].split(",");
              bool oldblk = tab->blockSignals(true);

	      /* the obvious, loop calling tab->selectRow(), gives
		 one selected row, so try this to get multiple
		 selections: make only the desired values selectable,
		 select everything, and connect to a slot that can
		 clean up after us.  yuck.
               */
              for (int j = 0; j < tab->rowCount(); j++)
              {
                if (! savedval.contains(tab->item(j, 0)->data(Qt::UserRole).toString()))
                  tab->item(j, 0)->setFlags(tab->item(j, 0)->flags() & (~ Qt::ItemIsSelectable));
              }
              QTableWidgetSelectionRange range(0, 0, tab->rowCount() - 1,
                                               tab->columnCount() - 1);
              tab->setRangeSelected(range, true);
              connect(tab, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resetMultiselect(QTableWidgetItem*)));

              tab->blockSignals(oldblk);
              storeFilterValue(-1, tab);
            }
          }
          break;
        case CheckBox:
          QCheckBox *checkBox;
          checkBox = qobject_cast<QCheckBox*>(found);
          if (checkBox != 0)
            checkBox->setChecked(QVariant(tempFilterList[1]).toBool());
          break;
        default:
          {
            QLineEdit *lineEdit;
            lineEdit = qobject_cast<QLineEdit*>(found);
            if (lineEdit != 0)
            {
              lineEdit->setText(tempFilterList[1]);
              storeFilterValue(-1, lineEdit);
            }
          }
          break;
        }//end of switch

      }//end of not empty key else
      windowIdx++;
    }//end of if tempfilter not empty
  }//end of for

  // TODO: the switch below is like the switch above. can we remove duplication?
  //This code is similar in concept, but the implementation is different.
  if (filterRows.size() == 1  && pId == 0)
  {
    //apply filter defaults
    QMapIterator<int, ParamProps*> k(_params);

    while(k.hasNext())
    {
      k.next();
      ParamProps* pp = k.value();

      if (!pp->defaultValue.isValid())
        continue;

      int windowIdx = _filtersLayout->rowCount();
      addParam();

      QLayoutItem *test = _filtersLayout->itemAtPosition(windowIdx, 0)->layout()->itemAt(0);
      XComboBox *mybox = (XComboBox*)test->widget();

      int idx = mybox->findText(pp->name);

      mybox->setCurrentIndex(idx);

      found = getFilterWidget(windowIdx);

      switch (pp->paramType)
      {
      case Date:
        DLineEdit *dLineEdit;
        dLineEdit = qobject_cast<DLineEdit*>(found);
        if (dLineEdit != 0)
          dLineEdit->setDate(pp->defaultValue.toDate(), true);
        break;
      case User:
        UsernameCluster *usernameCluster;
        usernameCluster = qobject_cast<UsernameCluster*>(found);
        if (usernameCluster != 0)
          usernameCluster->setUsername(pp->defaultValue.toString());
        break;
      case Crmacct:
        CRMAcctCluster *crmacctCluster;
        crmacctCluster = qobject_cast<CRMAcctCluster*>(found);
        if (crmacctCluster != 0)
          crmacctCluster->setId(pp->defaultValue.toInt());
        break;
      case Customer:
        CustCluster *custCluster;
        custCluster = qobject_cast<CustCluster*>(found);
        if (custCluster != 0)
          custCluster->setId(pp->defaultValue.toInt());
        break;
      case Shipto:
        ShiptoCluster *shiptoCluster;
        shiptoCluster = qobject_cast<ShiptoCluster*>(found);
        if (shiptoCluster != 0)
          shiptoCluster->setId(pp->defaultValue.toInt());
        break;
      case Vendor:
        VendorCluster *vendCluster;
        vendCluster = qobject_cast<VendorCluster*>(found);
        if (vendCluster != 0)
          vendCluster->setId(pp->defaultValue.toInt());
        break;
      case Contact:
        ContactCluster *contactCluster;
        contactCluster = qobject_cast<ContactCluster*>(found);
        if (contactCluster != 0)
          contactCluster->setId(pp->defaultValue.toInt());
        break;
      case GLAccount:
        GLCluster *glCluster;
        glCluster = qobject_cast<GLCluster*>(found);
        if (glCluster != 0)
          glCluster->setId(pp->defaultValue.toInt());
        break;
      case Project:
        ProjectCluster *projectCluster;
        projectCluster = qobject_cast<ProjectCluster*>(found);
        if (projectCluster != 0)
          projectCluster->setId(pp->defaultValue.toInt());
        break;
      case Item:
        ItemCluster *itemCluster;
        itemCluster = qobject_cast<ItemCluster*>(found);
        if (itemCluster != 0)
          itemCluster->setId(pp->defaultValue.toInt());
        break;
      case Employee:
        EmpCluster *employeeCluster;
        employeeCluster = qobject_cast<EmpCluster*>(found);
        if (employeeCluster != 0)
          employeeCluster->setId(pp->defaultValue.toInt());
        break;
      case SalesOrder:
        OrderCluster *soCluster;
        soCluster = qobject_cast<OrderCluster*>(found);
        if (soCluster != 0)
          soCluster->setId(pp->defaultValue.toInt());
        break;
      case TransferOrder:
        OrderCluster *toCluster;
        toCluster = qobject_cast<OrderCluster*>(found);
        if (toCluster != 0)
          toCluster->setId(pp->defaultValue.toInt());
        break;
      case PurchaseOrder:
        OrderCluster *poCluster;
        poCluster = qobject_cast<OrderCluster*>(found);
        if (poCluster != 0)
          poCluster->setId(pp->defaultValue.toInt());
        break;
      case WorkOrder:
        WoCluster *woCluster;
        woCluster = qobject_cast<WoCluster*>(found);
        if (woCluster != 0)
          woCluster->setId(pp->defaultValue.toInt());
        break;
      case Site:
        WComboBox *wBox;
        wBox = qobject_cast<WComboBox*>(found);
        if (wBox != 0)
          wBox->setId(pp->defaultValue.toInt());
        break;
      case XComBox:
        XComboBox *xBox;
        xBox = qobject_cast<XComboBox*>(found);
        if (xBox != 0)
        {
          //fix for setid not emitting id signal if id found for filter is first in list
          //set to any other valid id first to fix it
          xBox->setId(2);

          xid = pp->defaultValue.toInt();
          xBox->setId(xid);
        }
        break;
       case Multiselect:
        {
          QTableWidget *tab;
          tab = qobject_cast<QTableWidget *>(found);
          if (tab != 0)
          {
            QVariantList vlist = pp->defaultValue.toList();
            bool oldblk = tab->blockSignals(true);
            /* see the comment in the 'case Multiselect:' above */
            for (int j = 0; j < tab->rowCount(); j++)
            {
              if (! vlist.contains(tab->item(j, 0)->data(Qt::UserRole).toString()))
                tab->item(j, 0)->setFlags(tab->item(j, 0)->flags() & (~ Qt::ItemIsSelectable));
            }
            QTableWidgetSelectionRange range(0, 0, tab->rowCount() - 1,
                                             tab->columnCount() - 1);
            tab->setRangeSelected(range, true);
            connect(tab, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resetMultiselect(QTableWidgetItem*)));

            tab->blockSignals(oldblk);
            storeFilterValue(-1, tab);
          }
        }
        break;
      case CheckBox:
          QCheckBox *checkBox;
          checkBox = qobject_cast<QCheckBox*>(found);
          if (checkBox != 0)
            checkBox->setChecked(pp->defaultValue.toBool());
          break;
      default:
        QLineEdit *lineEdit;
        lineEdit = qobject_cast<QLineEdit*>(found);
        if (lineEdit != 0)
        {
          lineEdit->setText(pp->defaultValue.toString());
          storeFilterValue(-1, lineEdit);
        }
        break;
      }
      found->setEnabled(pp->enabled);
      windowIdx++;
    }//end of while _defaultTypes
  }

  emit updated();
}

void ParameterWidget::resetMultiselect(QTableWidgetItem *item)
{
  QTableWidget *tab = qobject_cast<QTableWidget*>(sender());
  if (! tab)
    return;

  if (DEBUG)
    qDebug("%s::resetMultiselect(%p) entered for %p",
           qPrintable(objectName()), item, tab);

  disconnect(tab, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resetMultiselect(QTableWidgetItem*)));

  for (int j = 0; j < tab->rowCount(); j++)
    tab->item(j, 0)->setFlags(tab->item(j, 0)->flags() | Qt::ItemIsSelectable);

  tab->selectRow(tab->row(item));
  storeFilterValue(-1, tab); // why is this necessary?
}

void ParameterWidget::changeFilterObject(int index)
{
  XComboBox *mybox = (XComboBox *)sender();
  QStringList split = mybox->itemData(index).toString().split(":");
  QString row = split.at(0);
  int type = split.at(1).toInt();
  XSqlQuery qry;
  int siteid = -1;

  QWidget *widget = _filterGroup->findChild<QWidget *>("widget" + row);
  QWidget *widget2 = _filterGroup->findChild<QWidget *>("widget2" + row);
  QWidget *button = _filterGroup->findChild<QToolButton *>("button" + row);
  QHBoxLayout *layout = _filterGroup->findChild<QHBoxLayout *>("widgetLayout1" + row);

  if (widget && layout && button)
  {
    delete widget;
    if (widget2)
      delete widget2;
  }
  else
    return;

  mybox->setDisabled(_params[paramIndex(mybox->currentText())]->required);
  button->setDisabled(_params[paramIndex(mybox->currentText())]->required);

  QWidget *newWidget = 0;
  switch (type)
  {
  case Date:
    {
      DLineEdit *dLineEdit= new DLineEdit(_filterGroup);
      newWidget = dLineEdit;
      connect(dLineEdit, SIGNAL(newDate(QDate)), this, SLOT( storeFilterValue() ) );
    }
    break;
  case User:
    {
      UsernameCluster *usernameCluster = new UsernameCluster(_filterGroup);
      newWidget = usernameCluster;
      usernameCluster->setOrientation(Qt::Horizontal);
      usernameCluster->setLabel("");

      connect(usernameCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case Crmacct:
    {
      CRMAcctCluster *crmacctCluster = new CRMAcctCluster(_filterGroup);
      newWidget = crmacctCluster;
      crmacctCluster->setOrientation(Qt::Horizontal);
      crmacctCluster->setLabel("");

      connect(crmacctCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case Customer:
    {
      CustCluster *custCluster = new CustCluster(_filterGroup);
      newWidget = custCluster;
      custCluster->setOrientation(Qt::Horizontal);
      custCluster->setLabel("");
      custCluster->setDescriptionVisible(false);

      connect(custCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case Shipto:
    {
      CustCluster *custCluster = new CustCluster(_filterGroup);
      QWidget *custWidget = custCluster;
      custCluster->setOrientation(Qt::Horizontal);
      custCluster->setLabel("");
      custCluster->setNameVisible(false);
      custCluster->setDescriptionVisible(false);

      connect(button, SIGNAL(clicked()), custCluster, SLOT( deleteLater() ) );

      ShiptoCluster *shiptoCluster = new ShiptoCluster(_filterGroup);
      QWidget *newWidgetAlt = shiptoCluster;
      shiptoCluster->setOrientation(Qt::Horizontal);
      shiptoCluster->setLabel("");
      shiptoCluster->setDescriptionVisible(false);

      connect(custCluster, SIGNAL(newId(int)), shiptoCluster, SLOT(setCustid(int)));
      connect(button, SIGNAL(clicked()), shiptoCluster, SLOT( deleteLater() ) );
      connect(shiptoCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );

      custCluster->setObjectName("widget" + row);
      newWidgetAlt->setObjectName("widget2" + row);
      layout->insertWidget(0,custWidget);
      layout->insertWidget(1,newWidgetAlt);
      layout->addStretch(1);
    }
    break;
  case Vendor:
    {
      VendorCluster *vendCluster = new VendorCluster(_filterGroup);
      newWidget = vendCluster;
      vendCluster->setOrientation(Qt::Horizontal);
      vendCluster->setLabel("");

      connect(vendCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case Contact:
    {
      ContactCluster *contactCluster = new ContactCluster(_filterGroup);
      newWidget = contactCluster;
      contactCluster->setDescriptionVisible(false);
      contactCluster->setLabel("");

      connect(contactCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case GLAccount:
    {
      GLCluster *glCluster = new GLCluster(_filterGroup);
      newWidget = glCluster;
      glCluster->setLabel("");

      connect(glCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case Project:
    {
      ProjectCluster *projectCluster = new ProjectCluster(_filterGroup);
      newWidget = projectCluster;
      projectCluster->setOrientation(Qt::Horizontal);
      projectCluster->setLabel("");

      connect(projectCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case Item:
    {
      ItemCluster *itemCluster = new ItemCluster(_filterGroup);
      newWidget = itemCluster;
      itemCluster->setOrientation(Qt::Horizontal);
      itemCluster->setLabel("");
      itemCluster->setNameVisible(false);

      connect(itemCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case Employee:
    {
      EmpCluster *employeeCluster = new EmpCluster(_filterGroup);
      newWidget = employeeCluster;
      employeeCluster->setOrientation(Qt::Horizontal);
      employeeCluster->setLabel("");

      connect(employeeCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case SalesOrder:
    {
      OrderCluster *soCluster = new OrderCluster(_filterGroup);
      newWidget = soCluster;
      soCluster->setDescriptionVisible(false);
      soCluster->setOrientation(Qt::Horizontal);
      soCluster->setAllowedTypes(OrderLineEdit::Sales);
      soCluster->setAllowedStatuses(OrderLineEdit::AnyStatus);
      soCluster->setLabel("");

      connect(soCluster, SIGNAL(newId(int,QString)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case TransferOrder:
    {
      OrderCluster *toCluster = new OrderCluster(_filterGroup);
      newWidget = toCluster;
      toCluster->setDescriptionVisible(false);
      toCluster->setOrientation(Qt::Horizontal);
      toCluster->setAllowedTypes(OrderLineEdit::Transfer);
      toCluster->setAllowedStatuses(OrderLineEdit::AnyStatus);
      toCluster->setLabel("");

      connect(toCluster, SIGNAL(newId(int,QString)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case PurchaseOrder:
    {
      OrderCluster *poCluster = new OrderCluster(_filterGroup);
      newWidget = poCluster;
      poCluster->setDescriptionVisible(false);
      poCluster->setOrientation(Qt::Horizontal);
      poCluster->setAllowedTypes(OrderLineEdit::Purchase);
      poCluster->setAllowedStatuses(OrderLineEdit::AnyStatus);
      poCluster->setLabel("");

      connect(poCluster, SIGNAL(newId(int,QString)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case WorkOrder:
    {
      WoCluster *woCluster = new WoCluster(_filterGroup);
      newWidget = woCluster;
      woCluster->setOrientation(Qt::Horizontal);
      woCluster->setLabel("");

      connect(woCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case Site:
    {
      WComboBox *wBox = new WComboBox(_filterGroup);
      siteid = wBox->id();
      newWidget = wBox;

      connect(wBox, SIGNAL(newID(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case XComBox:
    {
      XComboBox *xBox = new XComboBox(_filterGroup);
      newWidget = xBox;
      xBox->setNullStr(tr("--- Please Select ---"));
      xBox->setAllowNull(true);

      xBox->setType(_params[paramIndex(mybox->currentText())]->comboType);
      if (_params[paramIndex(mybox->currentText())]->comboType == XComboBox::Adhoc &&
          _params[paramIndex(mybox->currentText())]->query.length())
      {
        qry.prepare( _params[paramIndex(mybox->currentText())]->query );

        qry.exec();
        xBox->populate(qry);
      }
      //xBox->setAllowNull(true);
      connect(xBox, SIGNAL(newID(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;

  case Multiselect:
    {
      QTableWidget *tab = new QTableWidget(_filterGroup);
#ifdef Q_WS_MAC
      QFont f = tab->font();
      f.setPointSize(f.pointSize() - 2);
      tab->setFont(f);
#endif
      newWidget = tab;
      tab->setShowGrid(false);
      tab->setSelectionBehavior(QAbstractItemView::SelectRows);
      tab->setSelectionMode(QAbstractItemView::ExtendedSelection);
      tab->horizontalHeader()->setVisible(false);
      tab->horizontalHeader()->setStretchLastSection(true);
      tab->verticalHeader()->setVisible(false);
      tab->setColumnCount(1);

      XSqlQuery msq;
      msq.prepare(_params[paramIndex(mybox->currentText())]->query);
      msq.exec();
      for (int i = 0; msq.next(); i++)
      {
        if (i >= tab->rowCount())
          tab->insertRow(tab->rowCount());
        QTableWidgetItem *item = new QTableWidgetItem(msq.value(1).toString());
        item->setData(Qt::UserRole, msq.value(0));
        item->setFlags(item->flags() & (~ Qt::ItemIsEditable));
        if (DEBUG)
          qDebug("%s::changeFilterObject() Multiselect added %s (%s) at %d",
                 qPrintable(objectName()), qPrintable(item->text()),
                 qPrintable(item->data(Qt::UserRole).toString()), i);
        tab->setItem(i, 0, item);
        tab->setRowHeight(i, (tab->fontMetrics().height() * 110) / 100); // TODO: a better way?
      }
      // keep the filter vertically small
      tab->setMaximumHeight(qMin(6, tab->rowCount()) * tab->rowHeight(0));
      tab->show();

      connect(tab, SIGNAL(itemSelectionChanged()), this, SLOT(storeFilterValue()));
    }
    break;
  case Exists:
    {
      QLineEdit *lineEdit = new QLineEdit(_filterGroup);
      lineEdit->hide();
      newWidget = lineEdit;
    }
    break;
  case CheckBox:
    {
      QCheckBox *checkBox = new QCheckBox(_filterGroup);
      newWidget = checkBox;

      connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  default:
    {
      QLineEdit *lineEdit = new QLineEdit(_filterGroup);
      newWidget = lineEdit;

      connect(lineEdit, SIGNAL(textChanged(QString)), this, SLOT( storeFilterValue() ) );
    }
    break;
  }

  if (newWidget)
  {
    connect(button, SIGNAL(clicked()), newWidget, SLOT( deleteLater() ) );
    newWidget->setObjectName("widget" + row);
    layout->insertWidget(0, newWidget);
    // Get initial values pre-set for special boolean cases
    if (type == Exists || type == CheckBox)
    {
      storeFilterValue(1, newWidget);
      return;
    }
    else if (type == Site)
    {
      storeFilterValue(siteid, newWidget);
      return;
    }
    else
      storeFilterValue(-1, newWidget);
  }
  _saveButton->setDisabled(true);
}

void ParameterWidget::clearFilters()
{
  QStringList del;
  QList<QLayout *> llist = _filterGroup->findChildren<QLayout*>();
  QList<QWidget *> wlist = _filterGroup->findChildren<QWidget*>();

  // Find layout deletion candidates first
  for (int i = 0; i < llist.count(); i++)
  {
    if (llist.at(i)->objectName().startsWith("topLayout") ||
        llist.at(i)->objectName().startsWith("widgetLayout2") ||
        llist.at(i)->objectName().startsWith("buttonLayout"))
      del.append(llist.at(i)->objectName());
  }

  // Then find widget deletion candidates
  for (int i = 0; i < wlist.count(); i++)
  {
    if (wlist.at(i)->objectName().startsWith("xcombo") ||
        wlist.at(i)->objectName().startsWith("widget") ||
        wlist.at(i)->objectName().startsWith("button"))
      del.append(wlist.at(i)->objectName());
  }

  // Perform deletions
  while (del.count())
  {
    QObject *obj = _filterGroup->findChild<QObject*>(del.at(0));
    delete obj;
    del.takeFirst();
  }

  _filtersLayout = new QGridLayout();
  _filtersLayout->setObjectName("_filtersLayout");
  gridLayout_2->addLayout(_filtersLayout, 1, 0, 1, 1);

  _filterValues.clear();
  _filterWidgets.clear();
  _usedTypes.clear();
  _addFilterRow->setDisabled(false);

  emit cleared();
}


ParameterList ParameterWidget::parameters()
{
  ParameterList params;

  appendValue(params);

  return params;
}

void ParameterWidget::repopulateComboboxes()
{
  QRegExp rx("^xcomboBox");

  QList<XComboBox *> xlist = _filterGroup->findChildren<XComboBox*>(rx);
  QMapIterator<int, ParamProps* > j(_params);
  QString current;
  QString value;
  int idx;
  for (int i = 0; i < xlist.count(); i++)
  {
    current = xlist.at(i)->currentText();
    QStringList split = xlist.at(i)->itemData(xlist.at(i)->currentIndex()).toString().split(":");
    disconnect(xlist.at(i), 0, this, 0);
    xlist.at(i)->clear();
    while (j.hasNext())
    {
      j.next();
      ParamProps* pp = j.value();
      value = split[0] + ":" + QString().setNum(pp->paramType);
      if ( !containsUsedType(pp->name) || current == pp->name )
        xlist.at(i)->addItem(pp->name, value );
    }
    j.toFront();

    idx = xlist.at(i)->findText(current);
    xlist.at(i)->setCurrentIndex(idx);
    connect(xlist.at(i), SIGNAL(currentIndexChanged(int)), this, SLOT( addUsedType() ) );
    connect(xlist.at(i), SIGNAL(currentIndexChanged(int)), this, SLOT( changeFilterObject(int)) );
    connect(xlist.at(i), SIGNAL(currentIndexChanged(int)), this, SLOT( repopulateComboboxes() ) );
  }
}

void ParameterWidget::addUsedType()
{
  XComboBox *mybox = (XComboBox *)sender();
  QVariant filterVar(mybox->itemData(mybox->currentIndex()));
  QString filterType = filterVar.toString();
  QStringList split = filterType.split(":");
  int row = split[0].toInt();

  if (!_usedTypes.isEmpty())
    _usedTypes.remove(row);

  _usedTypes[row] = mybox->currentText();
}

void ParameterWidget::removeParam(int pRow)
{
  QLayoutItem *test;
  QLayoutItem *test2;

  test = _filtersLayout->itemAtPosition(pRow, 0)->layout()->itemAt(0);
  XComboBox *mybox = (XComboBox*)test->widget();

  QVariant filterVar(mybox->itemData(mybox->currentIndex()));
  QString filterType = filterVar.toString();
  QStringList split = filterType.split(":");

  _filterValues.remove(split[0].toInt());
  _filterWidgets.remove(split[0].toInt());


  test2 = _filtersLayout->itemAtPosition(pRow, 0)->layout()->takeAt(1);
  delete test2;
  test2 = 0;
  _filtersLayout->update();
  _addFilterRow->setDisabled(false);
  repopulateComboboxes();
  if (!mybox->currentText().isEmpty())
    _usedTypes.remove(pRow);
  emit updated();
  emit filterChanged();
}

void ParameterWidget::save()
{
  QString filter = "";
  QString variantString;
  QVariant tempVar;
  QDate today = QDate::currentDate();
  int filter_id;

  QMapIterator<int, QPair<QString, QVariant> > i(_filterValues);
  while (i.hasNext())
  {
    i.next();
    QPair<QString, QVariant> tempPair = i.value();

    tempVar = tempPair.second;

    QLayoutItem *test = _filtersLayout->itemAtPosition(i.key(), 0)->layout()->itemAt(0);
    XComboBox* mybox = (XComboBox*)test->widget();
    QStringList split = mybox->itemData(mybox->currentIndex()).toString().split(":");

    if ( tempVar.canConvert(QVariant::String) )
    {
      QDate variantDate;
      if ( tempVar.canConvert<QDate>() )
        variantDate = tempVar.toDate();

      if (variantDate.isValid())
      {
        variantString = QString().setNum(today.daysTo(variantDate));
      }
      else
        variantString = tempVar.toString();

      filter = filter + tempPair.first + ":" + variantString + ":" + split[1] + "|";
    }
    else if (tempVar.canConvert(QVariant::StringList))
      filter += tempPair.first + ":" + tempVar.toStringList().join(",")
      + ":" + split[1] + "|";
  }

  QString classname(parent()->objectName());
  if (classname.isEmpty())
    classname = parent()->metaObject()->className();

  ParameterList params;
  params.append("filter", filter);
  if (_filterList->id())
    params.append("filtersetname", _filterList->currentText());
  params.append("classname", classname);
  if (_shared)
    params.append("shared", true);

  filterSave newdlg(this);
  newdlg.set(params);
  filter_id = newdlg.exec();
  if (!filter_id)
    return;
  emit filterSetSaved();

  setSavedFilters();
  _filterList->setId(filter_id);
}

/*!
  Set the default value on an existing parameter \a pName to \a pDefault.  If
  \a pDefault is not passed or 0 then the previous default, if any, will be removed.
  if \a pAutoApply is true, then the default filter set will be automatically applied.

  \sa applyDefaultFilterSet();
*/
void ParameterWidget::setDefault(QString pName, QVariant pDefault, bool pAutoApply)
{
  int idx = paramIndex(pName);
  if (idx != -1)
  {
    ParamProps* pp = _params[idx];
    pp->defaultValue = pDefault;
  }
  else
    qWarning("Parameter %s not found.", qPrintable(pName));

  if (pAutoApply)
    applyDefaultFilterSet();
}

/*!
  Enable or disable an existing parameter \a pName to \a pEnabled.  Useful if
  you want a specific value to be forced based on privilege control or some
  other specific criteria that would limit the user's selection ability. Is
  effective the next time a user instantiates the parameter.
*/
void ParameterWidget::setEnabled(QString pName, bool pEnabled)
{
  int idx = paramIndex(pName);
  if (idx != -1)
  {
    ParamProps* pp = _params[idx];
    pp->enabled = pEnabled;
  }
  else
    qWarning("Parameter %s not found.", qPrintable(pName));
}

void ParameterWidget::setFiltersVisible(bool visible)
{
  if (_filterGroup->isVisible() == visible)
    return;

  if (visible && _filtersLayout->rowCount() == 1)
    addParam();

  _filterGroup->setVisible(visible);
  if (!_filterButton->isChecked() == visible)
    _filterButton->setChecked(visible);
  setFiltersVisabiltyPreference();
}

void ParameterWidget::setFiltersVisabiltyPreference()
{
  QString pname;
  if(window())
    pname = window()->objectName() + "/";
  _settingsName = pname + this->objectName();
  if (!_settingsName.isEmpty() && _x_preferences)
  {
    _x_preferences->set(_settingsName + "/checked", _filterGroup->isVisible());
  }
}

/*!
  Sets the current filter as the default for the parent object it resides in.
*/
void ParameterWidget::setFiltersDefault()
{
  QString pname;
  int filter_id;

  if(parent())
    pname = parent()->metaObject()->className();
  pname += "/";
  _settingsName2 = pname + this->objectName();

  filter_id = _filterList->id(_filterList->currentIndex());

  if (!_settingsName2.isEmpty() && _x_preferences)
    _x_preferences->set(_settingsName2 + "/filter_default", filter_id);
}

/*!
  Sets a saved filter set with Id \a defaultId to the currently selected filter.
  If no \a pDefaultId is provided the default filter will be used.

  \sa setSavedFiltersIndex(QString);
  */
void ParameterWidget::setSavedFilters(int defaultId)
{
  QString query;
  XSqlQuery qry;

  if(this->parent())
  {
    QString classname(parent()->objectName());
    if (classname.isEmpty())
      classname = parent()->metaObject()->className();

    query = " SELECT 0 AS filter_id, :none AS filter_name, 1 AS seq "
            " UNION "
            " SELECT filter_id, filter_name, 2 AS seq "
            " FROM filter "
            " WHERE COALESCE(filter_username,getEffectiveXtUser())=getEffectiveXtUser() "
            " AND filter_screen=:screen "
            " ORDER BY seq, filter_name ";

    qry.prepare(query);

    qry.bindValue(":screen", classname);
    qry.bindValue(":none", tr("Default"));
    qry.exec();
    if (defaultId)
      _filterList->populate(qry, defaultId);
    else
      _filterList->populate(qry, 0);
  }
}

void ParameterWidget::setSavedFiltersIndex(QString filterSetName)
{
  _filterList->findText(filterSetName);
  _filterList->setText(filterSetName);
}

void ParameterWidget::setType(QString pName, QString pParam, ParameterWidgetTypes type, QVariant pDefault, QVariant extraInfo)
{
  append(pName, pParam, type, pDefault, false, extraInfo.toString());
  qWarning("setType(QString pName, QString pParam, ParameterWidgetTypes type, QVariant pDefault, QVariant extraInfo) on parameterWidget is deprecated.  "
           "Use appendQString pName, QString pParam, ParameterWidgetTypes type, QVariant pDefault, bool pRequired, QString extraInfo).");
}

void ParameterWidget::setXComboBoxType(QString pName, QString pParam, XComboBox::XComboBoxTypes xType, QVariant pDefault)
{
  appendComboBox(pName, pParam, xType, pDefault);
  qWarning("setType(QString pName, QString pParam, XComboBox::XComboBoxTypes xType, QVariant pDefault) is deprecated. "
           "Use appendComboBox(QString pName, QString pParam, XComboBox::XComboBoxTypes xType, QVariant pDefault).");
}

void ParameterWidget::setXComboBoxType(QString pName, QString pParam, QString pQry, QVariant pDefault)
{
  appendComboBox(pName, pParam, pQry, pDefault);
  qWarning("setXComboBoxType(QString pName, QString pParam, QString pQry, QVariant pDefault) is deprecated."
           "Use appendComboBox(QString pName, QString pParam, QString pQry, QVariant pDefault).");
}

void ParameterWidget::sManageFilters()
{
  ParameterList params;
  QString classname(parent()->objectName());
  if (classname.isEmpty())
    classname = parent()->metaObject()->className();
  params.append("screen", classname);

  filterManager *newdlg = new filterManager(this, "");
  newdlg->set(params);
  newdlg->exec();
}

void ParameterWidget::toggleSave()
{
  if (!_filterValues.isEmpty())
  {
    _saveButton->setDisabled(false);
  }
}

// get the row number of a particular filter widget
// this has to change if we change the way things get laid out
int ParameterWidget::getFilterIndex(const QWidget *filterwidget)
{
  int index = -1;

  for (int i = 1; i < _filtersLayout->rowCount(); i++)
  {
    QLayoutItem *rowLI   = _filtersLayout->itemAtPosition(i, 1);
    if (rowLI)
    {
      QGridLayout *rowgrid = qobject_cast<QGridLayout *>(rowLI->layout());
      if (rowgrid)
      {
        QLayoutItem *childLI = rowgrid->itemAtPosition(0, 0);
        QHBoxLayout *filterlyt = qobject_cast<QHBoxLayout *>(childLI->layout()->itemAt(0)->layout());
        if (filterlyt)
        {
          for (int n = 0; n < filterlyt->count(); n++)
          {
            if (filterlyt->itemAt(n)->widget() == filterwidget)
            {
              index = i;
              break;
            }
          }
        }
      }
    }
  }

  return index;
}

// get the filter widget at a particular row
// this has to change if we change the way things get laid out
QWidget *ParameterWidget::getFilterWidget(const int index)
{
  QWidget *filterwidget = 0;

  QGridLayout *rowgrid = _filtersLayout->findChild<QGridLayout *>("topLayout"
                                                                  + QString::number(index));
  QLayoutItem *childLI = rowgrid->itemAtPosition(0, 0)->layout()->itemAt(0);
  QHBoxLayout *datalyt = (QHBoxLayout *)childLI->layout();
  QLayoutItem *filterLI = datalyt->itemAt(0);
  if (filterLI)
    filterwidget = filterLI->widget();

  return filterwidget;
}

// stores the value of a filter object into the filtervalues map
void ParameterWidget::storeFilterValue(int pId, QObject* filter)
{
  if (!filter)
    filter = (QObject *)sender();

  if (DEBUG)
    qDebug("%s::storeFilterValue(%d, %p)",
           qPrintable(objectName()), pId, filter);

  if (! filter)
  {
    qWarning("%s::storeFilterValue(%d, %p) could not find filter",
             qPrintable(objectName()), pId, filter);
    return;
  }

  int foundRow = getFilterIndex(qobject_cast<QWidget*>(filter));
  if (foundRow < 0)
  {
    qWarning("%s::storeFilterValue(%d, %p) could not find filter index",
             qPrintable(objectName()), pId, filter);
    return;
  }

  QLayoutItem *selectorLI = _filtersLayout->itemAtPosition(foundRow, 0)->layout()->itemAt(0);
  XComboBox *selector = qobject_cast<XComboBox*>(selectorLI->widget());
  QString currText = selector->currentText();
  ParamProps* pp = _params[paramIndex(currText)];

  QString classname(filter->metaObject()->className());

  // check usernamecluster before pid because we key off text value for users
  if (classname == "UsernameCluster")
  {
    UsernameCluster *usernameCluster = (UsernameCluster *)filter;
    QString username = usernameCluster->username();
    _filterValues[foundRow] = qMakePair(pp->param, QVariant(username));
    emit updated();
    if (classname == "XComboBox")
    {
      _saveButton->setDisabled(true);
    }
  }
  else if (classname == "QCheckBox")
  {
    QCheckBox *checkBox = (QCheckBox *)filter;
    _filterValues[foundRow] = qMakePair(pp->param, QVariant(checkBox->isChecked()));
    emit updated();
  }
  else if (pId != -1)
  {
    _filterValues[foundRow] = qMakePair(pp->param, QVariant(pId));
    emit updated();
  }
  else if (classname == "QLineEdit")
  {
    QLineEdit *lineEdit = (QLineEdit *)filter;
    _filterValues[foundRow] = qMakePair(pp->param, QVariant(lineEdit->text()));
    emit updated();
  }
  else if (classname == "DLineEdit")
  {
    DLineEdit *dateedit = qobject_cast<DLineEdit *>(filter);
    _filterValues[foundRow] = qMakePair(pp->param, QVariant(dateedit->date()));
    emit updated();
  }
  else if (classname == "QTableWidget")
  {
    QList<QTableWidgetItem*> selected = (qobject_cast<QTableWidget*>(filter))->selectedItems();
    QVariantList value;
    for (int i = 0; i < selected.size(); i++)
      value.append(selected.at(i)->data(Qt::UserRole));
    _filterValues[foundRow] = qMakePair(pp->param, QVariant(value));
    emit updated();
  }
  else if (classname == "ProjectCluster")
  {
    ProjectCluster *projectCluster = (ProjectCluster *)filter;
    _filterValues[foundRow] = qMakePair(pp->param, QVariant(projectCluster->id()));
    emit updated();
  }
  _filterWidgets[foundRow] = (QWidget *)filter;

  if (!_usedTypes.isEmpty())
    _usedTypes.remove(foundRow);
  _usedTypes[foundRow] = selector->currentText();

  _addFilterRow->setDisabled(false);
  repopulateComboboxes();
  emit filterChanged();
}

QString ParameterWidget::getParameterTypeKey(QString pValue)
{
  QMapIterator<int, ParamProps* > i(_params);
  while (i.hasNext())
  {
    i.next();
    ParamProps* pp = i.value();

    if (pValue == pp->param)
      return pp->name;
  }

  return QString();
}

/*!
  Returns the index of an appended parameter definition of \a pName.
*/
int ParameterWidget::paramIndex(QString pName)
{
  QMapIterator<int, ParamProps* > i(_params);
  while (i.hasNext())
  {
    i.next();
    ParamProps* pp = i.value();

    if (pName == pp->name)
      return i.key();
  }

  return -1;
}


//updates selected filter set to be the default upon loading of the current screen
//for the current user
void ParameterWidget::setSelectedFilter(int filter_id)
{
  XSqlQuery qry;

  QString classname(parent()->objectName());
  if (classname.isEmpty())
    classname = parent()->metaObject()->className();

  QString query = "UPDATE filter SET filter_selected=false "
                  "WHERE filter_screen=:screen AND filter_username=getEffectiveXtUser()";
  QString query2 = "UPDATE filter SET filter_selected=true "
                   "WHERE filter_screen=:screen AND filter_id=:id AND filter_username=getEffectiveXtUser()";
  qry.prepare(query);
  qry.bindValue(":screen", classname);

  qry.exec();

  qry.prepare(query2);
  qry.bindValue(":screen", classname);
  qry.bindValue(":id", filter_id);

  qry.exec();
}

bool ParameterWidget::containsUsedType(QString value)
{
  QMapIterator<int, QString > i(_usedTypes);
  while (i.hasNext()) {
    i.next();
    if (i.value() == value)
      return true;
  }

  return false;
}

/*!
  Returns a text based list separated by line feeds of parameter names and selected values.
  Can be useful when passed to printed reports as a header description of what filter has been
  applied to the presented data.
*/
QString ParameterWidget::filter()
{
  QStringList filters;
  QMapIterator<int, QPair<QString, QVariant> > i(_filterValues);
  while (i.hasNext())
  {
    i.next();
    QPair<QString, QVariant> tempPair = i.value();

    QMapIterator<int, ParamProps*> p(_params);
    while (p.hasNext())
    {
      p.next();
      ParamProps *pp = p.value();
      if (pp->param == tempPair.first)
      {
        QString value = ": ";
        QWidget *w = _filterWidgets.value(i.key());
        if (w->inherits("UsernameCluster"))
        {
          UsernameCluster *usernameCluster = (UsernameCluster *)w;
          value.append(usernameCluster->username());
        }
        else if (w->inherits("ContactCluster"))
        {
          ContactCluster *contactCluster = (ContactCluster *)w;
          value.append(contactCluster->name());
        }
        else if (w->inherits("ItemCluster"))
        {
          ItemCluster *itemCluster = (ItemCluster *)w;
          value.append(itemCluster->number() + " - " + itemCluster->description());
        }
        else if (w->inherits("VirtualCluster"))
        {
          VirtualCluster *virtualCluster = (VirtualCluster *)w;
          value.append(virtualCluster->number() + " - " + virtualCluster->name());
        }
        else if (w->inherits("XComboBox"))
        {
          XComboBox *comboBox = (XComboBox *)w;
          value.append(comboBox->currentText());
        }
        else if (w->inherits("DLineEdit"))
        {
          DLineEdit *dlineEdit = (DLineEdit *)w;
          value.append(dlineEdit->date().toString());
        }
        else if (w->inherits("QLineEdit"))
        {
          QLineEdit *lineEdit = (QLineEdit *)w;
          if (!w->isHidden())
            value.append(lineEdit->text());
          else
            value.clear();
        }
        else if (w->inherits("QTableWidget"))
        {
          QTableWidget *table = (QTableWidget *)w;
          QList<QTableWidgetItem *> items = table->selectedItems();
          QStringList rowvals;
          for (int i = 0; i < items.count(); i++)
            rowvals.append(items.at(i)->data(Qt::DisplayRole).toString());
          value.append(rowvals.join(","));
        }
        else if (w->inherits("QCheckBox"))
        {
          QCheckBox *checkBox = (QCheckBox *)w;
          if (checkBox->isChecked())
            value.append(tr("True"));
          else
            value.append(tr("False"));
        }
        else
          value.clear();

        filters.append(pp->name + value);
        break;
      }
    }
  }
  return filters.join("\n");
}

// script exposure ///////////////////////////////////////////////////////////

void ParameterWidgetfromScriptValue(const QScriptValue &obj, ParameterWidget* &item)
{
  item = qobject_cast<ParameterWidget*>(obj.toQObject());
}

QScriptValue ParameterWidgettoScriptValue(QScriptEngine *engine, ParameterWidget* const &item)
{
  return engine->newQObject(item);
}

QScriptValue constructParameterWidget(QScriptContext *context,
                                      QScriptEngine  *engine)
{
  ParameterWidget *obj = 0;
  if (context->argumentCount() == 1 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new ParameterWidget(qscriptvalue_cast<QWidget*>(context->argument(0)));
  else if (context->argumentCount() >= 2 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new ParameterWidget(qscriptvalue_cast<QWidget*>(context->argument(0)),
                              qPrintable(context->argument(1).toString()));

  return engine->toScriptValue(obj);
}

void setupParameterWidget(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, ParameterWidgettoScriptValue, ParameterWidgetfromScriptValue);

  QScriptValue widget = engine->newFunction(constructParameterWidget);

  widget.setProperty("Crmacct", QScriptValue(engine, ParameterWidget::Crmacct), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Customer", QScriptValue(engine, ParameterWidget::Customer), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Shipto", QScriptValue(engine, ParameterWidget::Shipto), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Vendor", QScriptValue(engine, ParameterWidget::Vendor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("User", QScriptValue(engine, ParameterWidget::User), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Text", QScriptValue(engine, ParameterWidget::Text), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Date", QScriptValue(engine, ParameterWidget::Date), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("XComBox", QScriptValue(engine, ParameterWidget::XComBox), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Contact", QScriptValue(engine, ParameterWidget::Contact), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("GLAccount", QScriptValue(engine, ParameterWidget::GLAccount), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Multiselect", QScriptValue(engine, ParameterWidget::Multiselect), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Exists", QScriptValue(engine, ParameterWidget::Exists), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CheckBox", QScriptValue(engine, ParameterWidget::CheckBox), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Project", QScriptValue(engine, ParameterWidget::Project), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Item", QScriptValue(engine, ParameterWidget::Item), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Employee", QScriptValue(engine, ParameterWidget::Employee), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Site", QScriptValue(engine, ParameterWidget::Site), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesOrder", QScriptValue(engine, ParameterWidget::SalesOrder), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WorkOrder", QScriptValue(engine, ParameterWidget::WorkOrder), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PurchaseOrder", QScriptValue(engine, ParameterWidget::PurchaseOrder), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TransferOrder", QScriptValue(engine, ParameterWidget::TransferOrder), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("ParameterWidget", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

