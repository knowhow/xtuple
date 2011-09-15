/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "todoCalendarControl.h"
#include "guiclient.h"
#include <parameter.h>
#include <QDebug>
#include <metasql.h>

#include "todoListCalendar.h"

todoCalendarControl::todoCalendarControl(todoListCalendar * parent)
  : CalendarControl(parent)
{
  _list = parent;
}

QString todoCalendarControl::contents(const QDate & date)
{
  QString sql = "SELECT count(*) AS result"
                "  FROM todoitem LEFT OUTER JOIN incdt ON (incdt_id=todoitem_incdt_id) "
                "                     LEFT OUTER JOIN crmacct ON (crmacct_id=todoitem_crmacct_id) "
                "                     LEFT OUTER JOIN cust ON (cust_id=crmacct_cust_id) "
                "                     LEFT OUTER JOIN incdtpriority ON (incdtpriority_id=todoitem_priority_id) "
                " WHERE((todoitem_due_date = <? value(\"date\") ?>)"
                "  <? if not exists(\"completed\") ?>"
                "   AND (todoitem_status != 'C')"
                "  <? endif ?>"
                "  <? if exists(\"username\") ?> "
                "   AND (todoitem_username=<? value(\"username\") ?>) "
                "  <? elseif exists(\"usr_pattern\") ?>"
                "   AND (todoitem_username ~ <? value(\"usr_pattern\") ?>) "
                "  <? endif ?>"
                "  <? if exists(\"active\") ?>AND (todoitem_active) <? endif ?>"
                "       );";

  ParameterList params;
  params.append("date", date);
  if(_list)
    _list->setParams(params);

  MetaSQLQuery mql(sql);
  XSqlQuery qry = mql.toQuery(params);
  if(qry.first())
  {
    if(qry.value(0).toInt() != 0)
      return qry.value(0).toString();
  }
  return QString::null;
}

