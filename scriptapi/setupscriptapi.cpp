/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "addressclustersetup.h"
#include "alarmssetup.h"
#include "calendarTools.h"
#include "clineeditsetup.h"
#include "commentssetup.h"
#include "contactwidgetsetup.h"
#include "crmacctlineeditsetup.h"
#include "currdisplaysetup.h"
#include "documentssetup.h"
#include "exporthelper.h"
#include "filemoveselector.h"
#include "glclustersetup.h"
#include "include.h"
#include "itemlineeditsetup.h"
#include "metasqlhighlighterproto.h"
#include "orderlineeditsetup.h"
#include "orreportproto.h"
#include "parametereditproto.h"
#include "parametergroupsetup.h"
#include "parameterlistsetup.h"
#include "parameterwidget.h"
#include "projectlineeditsetup.h"
#include "qactionproto.h"
#include "qboxlayoutproto.h"
#include "qbytearrayproto.h"
#include "qdialogsetup.h"
#include "qdialogbuttonboxproto.h"
#include "qdirproto.h"
#include "qdockwidgetproto.h"
#include "qdomattrproto.h"
#include "qdomcdatasectionproto.h"
#include "qdomcharacterdataproto.h"
#include "qdomcommentproto.h"
#include "qdomdocumentfragmentproto.h"
#include "qdomdocumentproto.h"
#include "qdomdocumenttypeproto.h"
#include "qdomelementproto.h"
#include "qdomentityproto.h"
#include "qdomentityreferenceproto.h"
#include "qdomimplementationproto.h"
#include "qdomnamednodemapproto.h"
#include "qdomnodelistproto.h"
#include "qdomnodeproto.h"
#include "qdomnotationproto.h"
#include "qdomprocessinginstructionproto.h"
#include "qdomtextproto.h"
#include "qdoublevalidatorproto.h"
#include "qeventproto.h"
#include "qfileproto.h"
#include "qfileinfoproto.h"
#include "qfontproto.h"
#include "qformlayoutproto.h"
#include "qgridlayoutproto.h"
#include "qiconproto.h"
#include "qiodeviceproto.h"
#include "qitemdelegateproto.h"
#include "qlayoutitemproto.h"
#include "qlayoutproto.h"
#include "qmainwindowproto.h"
#include "qmenuproto.h"
#include "qmessageboxsetup.h"
#include "qnetworkreplyproto.h"
#include "qnetworkrequestproto.h"
#include "qprinterproto.h"
#include "qprocessproto.h"
#include "qprocessenvironmentproto.h"
#include "qpushbuttonproto.h"
#include "qsizepolicyproto.h"
#include "qspaceritemproto.h"
#include "qsqldatabaseproto.h"
#include "qsqlerrorproto.h"
#include "qsqlrecordproto.h"
#include "qstackedwidgetproto.h"
#include "qtabwidgetproto.h"
#include "qtextdocumentproto.h"
#include "qtexteditproto.h"
#include "qtimerproto.h"
#include "qtoolbarproto.h"
#include "qtreewidgetitemproto.h"
#include "qtsetup.h"
#include "queryset.h"
#include "qurlproto.h"
#include "qvalidatorproto.h"
#include "qwebpageproto.h"
#include "qwebviewproto.h"
#include "qwidgetproto.h"
#include "ralineeditsetup.h"
#include "recurrencewidget.h"
#include "revisionlineeditsetup.h"
#include "screensetup.h"
#include "setupscriptapi.h"
#include "shipmentclusterlineeditsetup.h"
#include "usernamecluster.h"
#include "vendorgroupsetup.h"
#include "wcomboboxsetup.h"
#include "wocluster.h"
#include "womatlclustersetup.h"
#include "xcheckbox.h"
#include "xcombobox.h"
#include "xdatawidgetmapperproto.h"
#include "xdateeditsetup.h"
#include "xnetworkaccessmanager.h"
#include "xsqltablemodelproto.h"
#include "xsqlqueryproto.h"
#include "xt.h"
#include "xtreewidget.h"
#include "xvariantsetup.h"

/*! \defgroup scriptapi The xTuple ERP Scripting API

  The xTuple ERP Scripting API defines the interface between extension %scripts
  and the xTuple ERP C++ core.

 */

void setupScriptApi(QScriptEngine *engine)
{
  setupAddressCluster(engine);
  setupAlarms(engine);
  setupCLineEdit(engine);
  setupCRMAcctLineEdit(engine);
  setupComments(engine);
  setupContactWidget(engine);
  setupCurrDisplay(engine);
  setupDocuments(engine);
  setupExportHelper(engine);
  setupFileMoveSelector(engine);
  setupGLCluster(engine);
  setupInclude(engine);
  setupItemLineEdit(engine);
  setupMetaSQLHighlighterProto(engine);
  setupOrderLineEdit(engine);
  setupOrReportProto(engine);
  setupParameterEditProto(engine);
  setupParameterGroup(engine);
  setupParameterList(engine);
  setupParameterWidget(engine);
  setupPeriodListViewItem(engine);
  setupProjectLineEdit(engine);
  setupQActionProto(engine);
  setupQBoxLayoutProto(engine);
  setupQByteArrayProto(engine);
  setupQDialog(engine);
  setupQDialogButtonBoxProto(engine);
  setupQDirProto(engine);
  setupQDockWidgetProto(engine);
  setupQDomAttrProto(engine);
  setupQDomCDATASectionProto(engine);
  setupQDomCharacterDataProto(engine);
  setupQDomCommentProto(engine);
  setupQDomDocumentFragmentProto(engine);
  setupQDomDocumentProto(engine);
  setupQDomDocumentTypeProto(engine);
  setupQDomElementProto(engine);
  setupQDomEntityProto(engine);
  setupQDomEntityReferenceProto(engine);
  setupQDomImplementationProto(engine);
  setupQDomNamedNodeMapProto(engine);
  setupQDomNodeListProto(engine);
  setupQDomNodeProto(engine);
  setupQDomNotationProto(engine);
  setupQDomProcessingInstructionProto(engine);
  setupQDomTextProto(engine);
  setupQDoubleValidatorProto(engine);
  setupQEventProto(engine);
  setupQFileProto(engine);
  setupQFileInfoProto(engine);
  setupQFontProto(engine);
  setupQFormLayoutProto(engine);
  setupQGridLayoutProto(engine);
  setupQIconProto(engine);
  setupQIODeviceProto(engine);
  setupQItemDelegateProto(engine);
  setupQLayoutItemProto(engine);
  setupQLayoutProto(engine);
  setupQMainWindowProto(engine);
  setupQMenuProto(engine);
  setupQMessageBox(engine);
  setupQNetworkAccessManagerProto(engine);
  setupQNetworkReplyProto(engine);
  setupQNetworkRequestProto(engine);
  setupQPrinterProto(engine);
  setupQProcessProto(engine);
  setupQProcessEnvironmentProto(engine);
  setupQPushButtonProto(engine);
  setupQSizePolicy(engine);
  setupQSpacerItem(engine);
  setupQSqlDatabaseProto(engine);
  setupQSqlErrorProto(engine);
  setupQSqlRecordProto(engine);
  setupQStackedWidgetProto(engine);
  setupQTabWidgetProto(engine);
  setupQTextDocumentProto(engine);
  setupQTextEditProto(engine);
  setupQTimerProto(engine);
  setupQToolBarProto(engine);
  setupQTreeWidgetItemProto(engine);
  setupQt(engine);
  setupQuerySet(engine);
  setupQUrlProto(engine);
  setupQValidatorProto(engine);
  setupQWebPageProto(engine);
  setupQWebViewProto(engine);
  setupQWidgetProto(engine);
  setupRaLineEdit(engine);
  setupRecurrenceWidget(engine);
  setupRevisionLineEdit(engine);
  setupScreen(engine);
  setupShipmentClusterLineEdit(engine);
  setupUsernameCluster(engine);
  setupUsernameLineEdit(engine);
  setupVendorGroup(engine);
  setupWComboBox(engine);
  setupWoCluster(engine);
  setupWomatlCluster(engine);
  setupXCheckBox(engine);
  setupXComboBox(engine);
  setupXDataWidgetMapperProto(engine);
  setupXDateEdit(engine);
  setupXSqlTableModelProto(engine);
  setupXSqlQueryProto(engine);
  setupXt(engine);
  setupXTreeWidget(engine);
  setupXTreeWidgetItem(engine);
  setupXVariant(engine);
}
