/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QSqlError>

#include <currcluster.h>

#include "guiclient.h"
#include "verisignprocessor.h"

#define DEBUG false

/** \ingroup creditcards
    \class   VerisignProcessor
    \todo    Finish implementing this
 */

VerisignProcessor::VerisignProcessor() : CreditCardProcessor()
{
  _defaultLivePort   = 443;
  _defaultLiveServer = "payflow.verisign.com";
  _defaultTestPort   = 443;
  _defaultTestServer = "test-payflow.verisign.com";
}

int VerisignProcessor::doTestConfiguration()
{
  _errorMsg = errorMsg(-19).arg("Verisign");
  return -19;
}
