/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONFIGCREDITCARDPROCESSOR_H
#define CONFIGCREDITCARDPROCESSOR_H

#include "xwidget.h"

/** \class ConfigCreditCardProcessor
  
    \brief An abstract class for showing and saving configuration information
           for a specific credit card processor.

    Each credit card processing company has its own API and requirements
    for communicating with it over the Internet. The commonalities are
    encapsulated in the configureCC class. The ConfigCreditCardProcessor
    class abstracts a simple interface that configureCC can use to save
    the details specific to individual processing companies.

    Subclasses of ConfigCreditCardProcessor are expected to load the
    specific details in their constructors, present them in a UI for the user
    to view and change, and save these changes with the sSave slot.

    \see CreditCardProcessor
    \see configureCC
 */

class ConfigCreditCardProcessor : public XWidget
{
  Q_OBJECT

  public:
    ConfigCreditCardProcessor(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~ConfigCreditCardProcessor();

  public slots:
    /** \brief Save the details required for a specific credit card
               processing company.

        An abstract method.
        Child classes are expected to save the details specific to their
        own credit card processing companies with the sSave slot.

        \returns true if successful; false if there was an error, such as
                 a missing required field.
      */
    virtual bool sSave() = 0;

  protected slots:
    virtual void languageChange() = 0;
};

#endif
