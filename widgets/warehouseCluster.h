/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef warehouseCluster_h
#define warehouseCluster_h

#include "widgets.h"
#include "xcombobox.h"

class XTUPLEWIDGETS_EXPORT WComboBox : public XComboBox
{
  Q_OBJECT

  Q_ENUMS(WComboBoxTypes)

  Q_PROPERTY(WComboBoxTypes type READ type WRITE setType )

  public:
    WComboBox(QWidget *, const char * = 0);

    enum WComboBoxTypes
    {
      All, AllActive, NonTransit,
      Shipping, Sold, Supply,
      Transit, AllActiveInventory
    };

    WComboBoxTypes type();
    void setType(WComboBoxTypes);
    virtual void setAllowNull(bool);

  public slots:
    void findItemsites(int);
    void setId(int);

  private:
    enum WComboBoxTypes  _type;
};


#endif

