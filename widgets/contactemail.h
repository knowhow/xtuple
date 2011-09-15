/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONTACTEMAIL_H
#define CONTACTEMAIL_H

#include <QSqlQueryModel>;

#include "documents.h"
#include "ui_contactemail.h"

class contactEmail : public QDialog, public Ui::contactEmail
{
    Q_OBJECT

public:
    contactEmail(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~contactEmail();

    QPushButton* _new;
    QPushButton* _delete;

public slots:
    void accept();
    void set( const ParameterList & pParams );
    void sAdd();
    void sDelete();
    void sFillList();
    void sOpenEdit(QTreeWidgetItem *item);
    void sCloseEdit(QTreeWidgetItem *, QTreeWidgetItem*);
    void sHandleButtons(bool);

protected slots:
    virtual void languageChange();

private:
    int _cntctid;
};

#endif // CONTACTEMAIL_H
