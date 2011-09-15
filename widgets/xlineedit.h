/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef xlineedit_h
#define xlineedit_h

#include <QAction>
#include <QLineEdit>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>

#include "widgets.h"
#include "xdatawidgetmapper.h"

class XTUPLEWIDGETS_EXPORT XLineEdit : public QLineEdit
{
  Q_OBJECT
  Q_PROPERTY(QString fieldName    READ fieldName   WRITE setFieldName)
  Q_PROPERTY(QString defaultText  READ defaultText WRITE setDefaultText)
  
  public:
    XLineEdit(QWidget *, const char * = 0);

    Q_INVOKABLE bool isValid();
    Q_INVOKABLE int  id();
    Q_INVOKABLE void setValidator(QValidator * v) { QLineEdit::setValidator(v); }

    Q_INVOKABLE double toDouble(bool * = 0);
    virtual QString defaultText() const { return _default; }
    virtual QString fieldName()   const { return _fieldName; }
    virtual void   setText(const QVariant &);
    Q_INVOKABLE virtual void   setDouble(const double, const int = -1);

    Q_INVOKABLE inline virtual QString nullStr() const { return _nullStr; }
    Q_INVOKABLE        virtual void setNullStr(const QString &text);

    Q_INVOKABLE bool isNull();

  public slots:
    virtual void sParse();
    virtual void setData();
    virtual void setData(const QString &text);
    virtual void setDataWidgetMap(XDataWidgetMapper* m);
    virtual void setDefaultText(QString p)  { _default = p; }
    virtual void setFieldName(QString p)    { _fieldName = p; }

  signals:
    void clicked();
    void doubleClicked();
    void requestList();
    void requestSearch();
    void requestInfo();
    void requestAlias();

  protected slots:
    virtual void sHandleNullStr();

  protected:
    int     _id;
    bool    _valid;
    bool    _parsed;
    QString _nullStr;

    QAction* _listAct;
    QAction* _searchAct;
    QAction* _aliasAct;
    
    virtual void focusInEvent(QFocusEvent * event);
    virtual void focusOutEvent(QFocusEvent * event);

    void mouseDoubleClickEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);
    
  private:    
    bool _isNull;
    QString _default;
    QString _fieldName;
    XDataWidgetMapper *_mapper;
};

#endif

