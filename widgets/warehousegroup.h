/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef warehousegroup_h
#define warehousegroup_h

#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>

#include "widgets.h"

class WComboBox;
class ParameterList;
class XSqlQuery;

class XTUPLEWIDGETS_EXPORT WarehouseGroup : public QGroupBox
{
  Q_OBJECT
  
  Q_PROPERTY(bool fixedSize READ fixedSize WRITE setFixedSize)
  
  public:
    WarehouseGroup(QWidget * = 0, const char * = 0);

    bool fixedSize() {return _fixed;};
    void appendValue(ParameterList &);
    void bindValue(XSqlQuery &);

    Q_INVOKABLE int  id();
    Q_INVOKABLE bool isAll() const;
    Q_INVOKABLE bool isSelected() const;

  public slots:
    void setFixedSize(bool);
    void setAll();
    void setId(int);
    void findItemSites(int);

  signals:
    void updated();

  private:
    bool         _fixed;
    bool         _selectedOnly;
    QLabel	 *_site;
    QRadioButton *_all;
    QRadioButton *_selected;
    QWidget      *_selectedGroup;
    WComboBox    *_warehouses;
};

#endif
