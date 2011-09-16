/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef INCIDENT_H
#define INCIDENT_H

#include "guiclient.h"
#include "xdialog.h"
#include <QStringList>
#include <parameter.h>

#include "ui_incident.h"

class incident : public XDialog, public Ui::incident
{
    Q_OBJECT

public:
    incident(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~incident();
    
    Q_INVOKABLE virtual int     aropenid()  const;
    Q_INVOKABLE virtual QString arDoctype() const;
    Q_INVOKABLE virtual int     id()        const;
    Q_INVOKABLE virtual int     mode()      const;
    Q_INVOKABLE virtual void    populate();
    Q_INVOKABLE virtual bool    save(bool);

    QPushButton* _print;

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sCancel();
    virtual void sCRMAcctChanged(const int);
    virtual void sDeleteTodoItem();
    virtual void sEditTodoItem();
    virtual void sFillHistoryList();
    virtual void sFillTodoList();
    virtual void sHandleTodoPrivs();
    virtual void sNewTodoItem();
    virtual void sPopulateTodoMenu(QMenu*);
    virtual void sSave();
    virtual void sPrint();
    virtual void sViewTodoItem();
    virtual void sReturn();
    virtual void sViewAR();
    virtual void sAssigned();
    virtual void sDeleteCharacteristic();
    virtual void sEditCharacteristic();
    virtual void sFillCharacteristicsList();
    virtual void sNewCharacteristic();

signals:
    void populated();

protected slots:
    virtual void languageChange();

private:
    int		_cntctid;
    int		_incdtid;
    int		_mode;
    int         _aropenid;
    bool	_saved;
    QString     _ardoctype;
    QStringList	_statusCodes;
    
};

#endif // INCIDENT_H
