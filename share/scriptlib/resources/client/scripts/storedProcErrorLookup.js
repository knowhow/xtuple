/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

/* This simplified version of xtuple/common/storedProcErrorLookup.cpp takes
   an error structure, allowing packages to define their own errors.
*/
function storedProcErrorLookup(procName, returnVal, errorStruct)
{
  var message;

  if (null != errorStruct)
  {
    if (procName in errorStruct &&
      returnVal in errorStruct[procName])
      message =  errorStruct[procName][returnVal];
    else
    {
      procName = procName.toLowerCase();
      for (var tmpProcName in errorStruct)
        if (procName == tmpProcName.toLowerCase())
        {
          message = errorStruct[tmpProcName][returnVal];
          break;
        }
    }

    if (message != null)
      return "<p>" + message
           + qsTr("<br>(%1, %2)<br>").arg(procName).arg(returnVal);
  }

  return toolbox.storedProcErrorLookup(procName, returnVal);
}
