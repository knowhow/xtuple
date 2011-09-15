/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPFINANCIALREPORT_H
#define DSPFINANCIALREPORT_H

class GroupBalances;

#include "display.h"
#include <QMap>
#include <parameter.h>

#include "ui_dspFinancialReport.h"

class dspFinancialReport : public display, public Ui::dspFinancialReport
{
    Q_OBJECT

public:
    dspFinancialReport(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);

    Q_INVOKABLE virtual bool setParams(ParameterList &);

    Q_INVOKABLE int projectId() { return _prjid; }
    Q_INVOKABLE void setProjectId(int p) { _prjid = p; }

    Q_INVOKABLE QString reportName() const;

    Q_INVOKABLE bool columnHasTransactions(int col);
    Q_INVOKABLE QDate columnStart(int col);
    Q_INVOKABLE QDate columnEnd(int col);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sFillList();
    virtual void sPrint();
    virtual void sPreview();
    virtual void sPopulateMenu(QMenu * pMenu);
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem * pSelected, int pColumn);
    virtual void sFillListStatement();
    virtual void sFillListTrend();
    virtual void sFillPeriods();
    virtual void sEditPeriodLabel();
    virtual void sTogglePeriod();
    virtual void sToggleTrend();
    virtual void sToggleNotes();
    virtual bool sCheck();
    virtual void sNotes();
    virtual void sViewTransactions();

protected slots:
    virtual void sCollapsed( QTreeWidgetItem * item );
    virtual void sExpanded( QTreeWidgetItem * item );
    virtual void sReportChanged(int);

protected:
    virtual bool forwardUpdate();
    Q_INVOKABLE ParameterList getParams();
    
private:
    int _prjid;
    int _col;
    QMap<int, QString> _columnLabels;
    QMap<int, QPair<QDate, QDate> > _columnDates;
    QAction *_notesAct;
    QString _typeCode;
};

#endif // DSPFINANCIALREPORT_H
