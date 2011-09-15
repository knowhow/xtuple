/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef itemAliasList_h
#define itemAliasList_h

#include <QVariant>
#include <QDialog>

#include "widgets.h"
#include "itemcluster.h"

class QLineEdit;
class QCheckBox;
class QPushButton;
class XTreeWidget;
class ParameterList;

class XTUPLEWIDGETS_EXPORT itemAliasList : public QDialog
{
    Q_OBJECT

  public:
    itemAliasList(QWidget * = 0, const char * = 0, bool = FALSE, Qt::WFlags = 0 );

    QLineEdit* _alias;
    QCheckBox* _showInactive;
    QPushButton* _close;
    QPushButton* _select;
    XTreeWidget* _item;

    virtual void set( ParameterList & pParams );

  protected slots:
    virtual void sSelect();
    virtual void sFillList();

  private:
    unsigned int _itemType;
    bool _useQuery;
    QString _sql;
    QStringList _extraClauses;
};

#endif
