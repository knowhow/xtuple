/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "include.h"
#include <xsqlquery.h>

/*! \file include.cpp

  \brief Implement the include facility of the \ref scriptapi "Scripting API"

  This file implements the include() function used in %scripts to
  extend xTuple ERP.
*/

/*! \brief Sets the include() function as a property of the global object.

  This installs the include() function as a property
  in the global namespace of the given QScriptEngine.
  It should only be called by setupScriptApi().

  \param engine A pointer to the script engine in which to install the include
                function.
 */

void setupInclude(QScriptEngine *engine)
{
  engine->globalObject().setProperty("include",
                                     engine->newFunction(includeScript));
}

/*! \brief Implements an include facility for use in xTuple ERP %scripts.

  This is the C++ implementation of the include() function.
  It should never be called directly.

  \param context The context contains a list of arguments passed to the
                 include() function by
                 the calling script. All arguments are processed as
                 potential script names to be loaded and run.
  \param engine  The QScriptEngine in which to load the included %scripts.

  \see include()

 */

QScriptValue includeScript(QScriptContext *context, QScriptEngine *engine)
{
  int count = 0;
  XSqlQuery scriptq;
  scriptq.prepare("SELECT script_id, script_source AS src"
                  "  FROM script"
                  " WHERE ((script_name=:script_name)"
                  "   AND  (script_enabled))"
                  " ORDER BY script_order;");

  context->setActivationObject(context->parentContext()->activationObject());
  context->setThisObject(context->parentContext()->thisObject());

  for (; count < context->argumentCount(); count++)
  {
    QString scriptname = context->argument(count).toString();
    scriptq.bindValue(":script_name", scriptname);
    scriptq.exec();
    while (scriptq.next())
    {
      QScriptValue result = engine->evaluate(scriptq.value("src").toString(),
                                             scriptname,
                                             1);
      if (engine->hasUncaughtException())
      {
        qWarning() << "uncaught exception in" << scriptname
                   << "(id" << scriptq.value("script_id").toInt()
                   << ") at line"
                   << engine->uncaughtExceptionLineNumber() << ":"
                   << result.toString();
        break;
      }
    }
  }

  return engine->toScriptValue(count);
}

/*! \ingroup scriptapi

  \brief Include the named script(s) in the current script.

  This lets the script developer put commonly used QtScript (JavaScript)
  code in a single place and use it in multiple other %scripts.
  All named %scripts are included in the order listed.
  If more than one script exists with the given name, all of them are included
  in increasing order by script_order. The inclusion is done as the script
  is executed, so if an include() call appears in the middle of a function,
  the include will not be done until that function gets called.
  If any of the included %scripts throws an exception while being included,
  no subsequent %scripts in the list will be processed.

  For example:
  \code
    include("soap2js");
  \endcode
  This tells the current QScriptEngine to use the contents of the script
  named "soap2js" and appears near the top of the weatherForAddress.js script.

  \code
    include("storedProcErrLookup", "xtmfgErrors");
  \endcode
  This tells the current QScriptEngine to use the contents of two %scripts,
  one named "storedProcErrLookup" and the other named "xtmfgErrors".

  \param first  The name of a script to include
  \param second Optional: The name of a second script to include

  \return The number of arguments that were processed

  \see weatherForAddress.js
  \see include.cpp

 */
#ifdef XTUPLEDOXYGEN
include(String first, String second, ... ) {}
#endif
