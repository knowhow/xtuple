/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __REVISIONCLUSTER_H__
#define __REVISIONCLUSTER_H__

#include "widgets.h"
#include "xlineedit.h"

#include <QLabel>

#include <xsqlquery.h>

#include "virtualCluster.h"

class XTUPLEWIDGETS_EXPORT RevisionLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  friend class RevisionCluster;

  Q_ENUMS(Modes)
  Q_ENUMS(RevisionTypes)

  Q_PROPERTY(Modes     mode READ mode   WRITE setMode   )
  Q_PROPERTY(RevisionTypes   type READ type WRITE setType )

  public:
    RevisionLineEdit(QWidget *, const char * = 0);

    enum Modes { View, Use, Maintain };
    enum RevisionTypes { All, BOM, BOO };
    virtual Modes mode();
    virtual RevisionTypes type();
    virtual QString typeText();

  protected slots:
    virtual void setId(const int);
    virtual void sParse();

  public slots:
    void setActive();
      void setMode(QString);
      void setMode(Modes);
    void setTargetId(int pItem);
    void setType(QString);
    void setType(RevisionTypes);

  private:
    bool _allowNew;
    bool _isRevControl;
    enum Modes _mode;
    enum RevisionTypes _type;
    int _targetId;
    QString _cachenum;
    QString _typeText;

  signals:
    void canActivate(bool);
    void modeChanged();

};

class XTUPLEWIDGETS_EXPORT RevisionCluster : public VirtualCluster
{
  Q_OBJECT

  Q_ENUMS(RevisionLineEdit::Modes)
  Q_ENUMS(RevisionLineEdit::RevisionTypes)

  Q_PROPERTY(RevisionLineEdit::Modes     mode READ mode   WRITE setMode   )
  Q_PROPERTY(RevisionLineEdit::RevisionTypes   type READ type WRITE setType )

  public:
    RevisionCluster(QWidget *, const char * = 0);
    virtual RevisionLineEdit::Modes mode();
    virtual RevisionLineEdit::RevisionTypes type();

  private slots:
    void sModeChanged();
    void sCanActivate(bool p);
    void setActive();

  public slots:
    void activate();
    virtual void setMode(QString);
    virtual void setMode(RevisionLineEdit::Modes);
    virtual void setType(QString);
    virtual void setType(RevisionLineEdit::RevisionTypes);
    virtual void setTargetId(int pItem);

  signals:
    void canActivate(bool);
};

#endif
