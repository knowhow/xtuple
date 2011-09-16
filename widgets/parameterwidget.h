/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef parameterwidget_h
#define parameterwidget_h

#include <QDate>
#include <QSignalMapper>
#include <QMap>
#include <QPair>
#include <QToolButton>
#include <QRegExp>
#include <parameter.h>

#include "widgets.h"
#include "ui_parameterwidget.h"

class ParameterList;
class QTableWidgetItem;

class XTUPLEWIDGETS_EXPORT ParameterWidget : public QWidget, public Ui::ParameterWidget
{
  Q_OBJECT

  Q_ENUMS(ParameterWidgetTypes)
   
  public:
    enum ParameterWidgetTypes
    {
      Crmacct, User, Text, Date, XComBox, Contact,
      Multiselect, GLAccount, Exists, CheckBox, Project,
      Customer, Site, Vendor, Item, Employee
    };

    ParameterWidget(QWidget *pParent, const char * = 0);
    void appendValue(ParameterList &);
    Q_INVOKABLE void applyDefaultFilterSet();
    Q_INVOKABLE int paramIndex(QString pName);
    Q_INVOKABLE ParameterList parameters();
    
 public slots:
    void addParam();
    void append(QString pName, QString pParam, enum ParameterWidgetTypes pType = Text, QVariant pDefault = QVariant(),  bool pRequired = false, QString pExtraInfo =  QString());
    void appendComboBox(QString pName, QString pParam, QString pQuery, QVariant pDefault = QVariant(), bool pRequired = false);
    void appendComboBox(QString pName, QString pParam, int pType, QVariant pDefault = QVariant(), bool pRequired = false);
    void applySaved(int pId = 0, int filter_id = 0);
    void changeFilterObject(int index);
    void clearFilters();
    void removeParam(int);
    void save();
    void setDefault(QString pName, QVariant pDefault = QVariant(), bool pAutoApply = false);
    void setEnabled(QString pName, bool pEnabled);
    void setSavedFilters(int defaultId = -1);
    void setSavedFiltersIndex(QString);
    void setType(QString, QString, enum ParameterWidgetTypes = Text, QVariant pDefault = QVariant(), QVariant extraInfo =  QVariant());
    void setXComboBoxType(QString, QString, enum XComboBox::XComboBoxTypes, QVariant pDefault = QVariant());
    void setXComboBoxType(QString, QString, QString, QVariant pDefault = QVariant());
    void sManageFilters();
    void storeFilterValue(int pId = -1, QObject* filter = 0);
    void setFiltersVisabiltyPreference();
    void toggleSave();
    void setFiltersDefault();

  signals:
    void cleared();
    void filterChanged();
    void filterSetSaved();
    void updated();

  protected:
    virtual void showEvent(QShowEvent *);

  private:
    enum ParameterWidgetTypes _type;
    QSignalMapper *_filterSignalMapper;
    QMap<int, QString > _usedTypes;
    QString _settingsName, _settingsName2;
    QMap<int, QPair<QString, QVariant > > _filterValues;
    bool _initialized;
    bool _shared;

    struct ParamProps {
      QString name;
      QString param;
      ParameterWidgetTypes paramType;
      QVariant defaultValue;
      bool required;
      XComboBox::XComboBoxTypes comboType;
      QString query;
      bool enabled;
    };
    QMap<int, ParamProps*> _params;

  protected:
    int      getFilterIndex(const QWidget *filterwidget);
    QWidget *getFilterWidget(const int index);
    QString  getParameterTypeKey(QString);
    void setSelectedFilter(int filter_id);
    
    bool containsUsedType(QString);

  private slots:
    void repopulateComboboxes();
    void addUsedType();

  protected slots:
    void     resetMultiselect(QTableWidgetItem* item);

};

Q_DECLARE_METATYPE(ParameterWidget*);

void setupParameterWidget(QScriptEngine *engine);

#endif
