/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TRANSFERORDER_H
#define TRANSFERORDER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "toitemTableModel.h"

#include "ui_transferOrder.h"

class transferOrder : public XWidget, public Ui::transferOrder
{
    Q_OBJECT

public:
    transferOrder(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~transferOrder();

    virtual	bool save( bool partial );
    virtual	void setViewMode();
    static	void newTransferOrder( int , int );
    static	void editTransferOrder( int pId, bool enableSaveAndAdd );
    static	void viewTransferOrder( int pId );

public slots:
    virtual SetResponse set(const ParameterList & pParams );

    virtual void clear();
    virtual void closeEvent( QCloseEvent * pEvent );
    virtual void populate();
    virtual void populateOrderNumber();
    virtual void sAction();
    virtual void sCalculateTax();
    virtual void sCalculateTotal();
    virtual void sClear();
    virtual void sDelete();
    virtual void sEdit();
    virtual void sFillItemList();
    virtual void sHandleButtons();
    virtual void sHandleDstWhs(const int);
    virtual void sHandleOrderNumber();
    virtual void sHandleShipchrg( int pShipchrgid );
    virtual void sHandleSrcWhs(const int);
    virtual void sHandleTransferOrderEvent(int);
    virtual void sHandleTrnsWhs(const int);
    virtual void sIssueLineBalance();
    virtual void sIssueStock();
    virtual void sNew();
    virtual void sPopulateMenu(QMenu * pMenu);
    virtual void sReleaseTohead();
    virtual void sReturnStock();
    virtual void sSave();
    virtual void sSaveAndAdd();
    virtual void sSetUserEnteredOrderNumber();
    virtual void sTaxDetail();

protected:
    virtual void keyPressEvent( QKeyEvent * e );

protected slots:
    virtual void languageChange();
    virtual void sCurrencyChanged();
    virtual void sQEDelete();
    virtual bool sQESave();
    virtual void sTabChanged(int);

signals:
    void saved(int);

private:
    bool 		deleteForCancel();
    void		getWhsInfo(const int, const QWidget*);
    bool		insertPlaceholder();
    void		setToheadid(const int);

    int			_cachedTabIndex;
    bool		_captive;
    bool		_doAuthorize;
    bool		_ffShipto;
    int			_freighttaxid;
    bool		_ignoreSignals;
    int			_lineMode;
    int			_mode;
    int			_orderNumberGen;
    int			_originalPrjid;
    bool		_passPrecheck;
    QString		_port;
    ToitemTableModel*	_qeitem;
    QString		_response;
    bool		_saved;
    int			_shiptoid;
    int			_toheadid;
    int			_taxzoneidCache;
    bool		_userEnteredOrderNumber;
    int			_whstaxzoneid;
    QString		doServer;
    bool        _locked;


};

#endif // TRANSFERORDER_H
