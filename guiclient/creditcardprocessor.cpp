/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QSqlError>
#include <QHttp>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QUrl>
#include <QBuffer>

#include <currcluster.h>
#include <metasql.h>
#include <openreports.h>

#include "guiclient.h"
#include "creditcardprocessor.h"
#include "storedProcErrorLookup.h"

#include "authorizedotnetprocessor.h"
#include "externalccprocessor.h"
#include "verisignprocessor.h"
#include "yourpayprocessor.h"
#include "paymentechprocessor.h"
#include "cybersourceprocessor.h"

#define DEBUG false

/** \defgroup creditcards Credit Card API
    The xTuple Credit Card subsystem contains the internal code
    for processing credit card transactions and the user interface
    for configuring the credit card handling.
  */
/** \ingroup creditcards
  
    \class CreditCardProcessor

    \brief This is a generic class that defines the interface between
           xTuple ERP and credit card processing services.

    CreditCardProcessor encapsulates the common functionality shared across
    different services, including interaction with the database server and
    the proper stored procedures to call.

    It should be subclassed once for each credit card processing %company to
    be supported.  Subclasses should override the following methods:
    \li doAuthorize
    \li doCharge
    \li doChargePreauthorized
    \li doCredit
    \li doReversePreauthorized
    \li doVoidPrevious
    \li doTestConfiguration

    Subclasses should also set
    \li _defaultLiveServer
    \li _defaultTestServer
    \li _defaultLivePort
    \li _defaultTestPort

    It is the subclass' responsibility to ensure that all of the
    configuration options available on the Credit Card Configuration
    window are implemented either here or in the subclass. An example
    of an option that \b must be implemented in each subclass is
    \c CCTestResult since the method for requesting error responses from
    the credit card processing service is different for every
    service.

    In addition to subclassing CreditCardProcessor, alternate credit
    card processing services require changing
    CreditCardProcessor::getProcessor and configureCC, and to subclass
    ConfigCreditCardProcessor.
    CreditCardProcessor::getProcessor must be modified to instantiate the
    right subclass of CreditCardProcessor based on the \c CCCompany
    metric or its QString argument.  configureCC must be modified
    to store the service-specific \c CCCompany value checked by getProcessor
    and to instantiate a ConfigCreditCardProcessor, which allows
    editing and saving service-specific metrics

    \b Error \b handling:

    The following rules are followed by CreditCardProcessor, which also expects
    its subclasses to follow them. It is the subclass' responsibility
    to follow these conventions. If they are not followed, the database will
    not represent what really happened and the %user will not be informed
    of problems.

    Each method in CreditCardProcessor and its subclasses involved in
    processing credit card transactions is expected to return an integer
    conforming to the following pattern and set _errorMsg to the string
    associated with this integer in the _msgHash:
    <table>
     <tr>
       <th>If the function returns:</th><th>It means this:</th>
      </tr>
      <tr>
       <td>greater than 0</td>
       <td>The interaction with the credit card processing service was
           successful but either there is a warning from the service,
   	   like the credit card processing %company denied the transaction,
	   or local post-processing generated an error, such as a database
           failure
        </td>
      </tr>
      <tr>
        <td>0</td>
        <td>the transaction succeeded in its entirety</td>
      </tr>
      <tr>
        <td>less than 0</td>
        <td>preprocessing failed or the credit card %company returned an error,
            like a malformed message or communications could not be established
            to process the transaction
        </td>
      </tr>
    </table>

    Error codes between -100 and 100 inclusive are reserved for the
    CreditCardProcessor class itself.
    Values less than -100 or greater than 100 are available for use by
    subclasses
    and should be loaded into _msgHash in the subclass' constructor.

    \see AuthorizeDotNetProcessor
    \see CyberSourceProcessor
    \see ExternalCCProcessor
    \see PaymentechProcessor
    \see YourPayProcessor
    \see configureCC

    \todo expose portions of this in the scriptapi doxygen module
    \todo use qabstractmessagehandler instead of qmessagebox
*/

QString			 CreditCardProcessor::_errorMsg = "";
QHash<int, QString>	 CreditCardProcessor::_msgHash;

static struct {
    int		code;
    const char*	text;
} messages[] = {

  {   0, QT_TRANSLATE_NOOP("CreditCardProcessor", "This transaction was approved.\n%1")			},

  {  -1, QT_TRANSLATE_NOOP("CreditCardProcessor", "Database Error")						},
  {  -2, QT_TRANSLATE_NOOP("CreditCardProcessor", "You don't have permission to process Credit Card transactions.") },
  {  -3, QT_TRANSLATE_NOOP("CreditCardProcessor", "The application is not set up to process credit cards.")	},
  {  -4, QT_TRANSLATE_NOOP("CreditCardProcessor", "The Bank Accounts are not set for all Credit Card types.") },
  {  -5, QT_TRANSLATE_NOOP("CreditCardProcessor", "The encryption key is not defined.")			},
  {  -6, QT_TRANSLATE_NOOP("CreditCardProcessor", "The login for the proxy server is not defined.")		},
  {  -7, QT_TRANSLATE_NOOP("CreditCardProcessor", "The password for the proxy server is not defined.")	},
  {  -8, QT_TRANSLATE_NOOP("CreditCardProcessor", "The proxy server is not defined.")				},
  {  -9, QT_TRANSLATE_NOOP("CreditCardProcessor", "The port to use for the proxy server is not defined.")	},
  { -10, QT_TRANSLATE_NOOP("CreditCardProcessor", "Credit Card %1 is not active. Make it active or select "
	    "another Credit Card.")					},
  { -11, QT_TRANSLATE_NOOP("CreditCardProcessor", "Credit Card %1 has expired.")				},
  { -12, QT_TRANSLATE_NOOP("CreditCardProcessor", "The Credit Card Processing Company reported an error:\n%1")		},
  { -13, QT_TRANSLATE_NOOP("CreditCardProcessor", "The Credit Card configuration is inconsistent and the application "
	    "cannot determine whether to run in Test or Live mode.")	},
  { -14, QT_TRANSLATE_NOOP("CreditCardProcessor", "Could not figure out which Credit Card Processing Company "
	    "to set up (based on %1).")					},
  { -15, QT_TRANSLATE_NOOP("CreditCardProcessor", "The digital certificate (.pem file) is not set.")		},
  { -16, QT_TRANSLATE_NOOP("CreditCardProcessor", "Could not open digital certificate (.pem file) %1.")	},
  { -17, QT_TRANSLATE_NOOP("CreditCardProcessor", "Could not find a Credit Card with internal ID %1.")	},
  { -18, QT_TRANSLATE_NOOP("CreditCardProcessor", "Error with message transfer program:\n%1 %2\n\n%3")	},
  { -19, QT_TRANSLATE_NOOP("CreditCardProcessor", "%1 is not implemented.")					},
  { -20, QT_TRANSLATE_NOOP("CreditCardProcessor", "The application does not support either Credit Cards or "
	    "Checks with %1. Please choose a different company.")	}, 
  // preauthorizing charges
  { -21, QT_TRANSLATE_NOOP("CreditCardProcessor", "The amount to charge must be greater than 0.00.")		},
  { -24, QT_TRANSLATE_NOOP("CreditCardProcessor", "Could not generate a sequence number while preauthorizing.") },

  // processing charges based on a pre-authorization
  { -30, QT_TRANSLATE_NOOP("CreditCardProcessor", "Could not find the Credit Card preauthorization to charge.") },
  { -32, QT_TRANSLATE_NOOP("CreditCardProcessor", "You must select a preauthorization to charge.") 		},
  { -33, QT_TRANSLATE_NOOP("CreditCardProcessor", "The preauthorization (for %1) is not sufficient to cover "
		 "the desired transaction amount (%2).")		},
  { -34, QT_TRANSLATE_NOOP("CreditCardProcessor", "No Preauthorization found")				},
  { -35, QT_TRANSLATE_NOOP("CreditCardProcessor", "This preauthorization may not be charged. It was created "
	    "for a Sales Order which has been canceled.")	},

  // stand-alone charges
  { -40, QT_TRANSLATE_NOOP("CreditCardProcessor", "Inconsistent data passed to charge(): [%1] [%2]")	},
  { -44, QT_TRANSLATE_NOOP("CreditCardProcessor", "Could not generate a sequence number while charging.")	},

  // credits
  { -50, QT_TRANSLATE_NOOP("CreditCardProcessor", "Could not find original Credit Card payment to credit.")	},

  // voids
  { -60, QT_TRANSLATE_NOOP("CreditCardProcessor", "Could not find the Credit Card transaction to void.")	},

  // user chose to cancel
  { -70, QT_TRANSLATE_NOOP("CreditCardProcessor", "User chose not to process the preauthorization.")		},
  { -71, QT_TRANSLATE_NOOP("CreditCardProcessor", "User chose not to post-authorize process the charge.")	},
  { -72, QT_TRANSLATE_NOOP("CreditCardProcessor", "User chose not to process the charge.")			},
  { -73, QT_TRANSLATE_NOOP("CreditCardProcessor", "User chose not to process the credit.")			},
  { -74, QT_TRANSLATE_NOOP("CreditCardProcessor", "User chose not to process the void.")			},
  { -75, QT_TRANSLATE_NOOP("CreditCardProcessor", "User chose not to proceed without CVV code.")		},

  // scripting errors
  { -81, QT_TRANSLATE_NOOP("CreditCardProcessor", "Scripting error in %1: Input parameter %2 is not a(n) %3") },

  // transaction was processed fine but was not successful
  { -90, QT_TRANSLATE_NOOP("CreditCardProcessor", "This Credit Card transaction was denied.\n%1")		},
  { -91, QT_TRANSLATE_NOOP("CreditCardProcessor", "This Credit Card transaction is a duplicate.\n%1")		},
  { -92, QT_TRANSLATE_NOOP("CreditCardProcessor", "This Credit Card transaction was declined.\n%1")		},
  { -93, QT_TRANSLATE_NOOP("CreditCardProcessor", "This Credit Card transaction was denied "
	    "because of possible fraud.\n%1")				},

  // other misc errors
  { -94, QT_TRANSLATE_NOOP("CreditCardProcessor", "The Bank Account is not set for Credit Card type %1. Either this "
            "card type is not accepted or the Credit Card configuration is not "
            "complete.")                                                },
  { -95, QT_TRANSLATE_NOOP("CreditCardProcessor", "The Credit Card Processor returned an error: %1")		},
  { -96, QT_TRANSLATE_NOOP("CreditCardProcessor", "This transaction failed the CVV check.")			},
  { -97, QT_TRANSLATE_NOOP("CreditCardProcessor", "This transaction failed the Address Verification check.")	},
  { -98, QT_TRANSLATE_NOOP("CreditCardProcessor", "You may not process this transaction without a CVV code. "
	    "Please enter one and try again.")				},
  { -99, QT_TRANSLATE_NOOP("CreditCardProcessor", "The CVV value is not valid.")				},
  {-100, QT_TRANSLATE_NOOP("CreditCardProcessor", "No approval code was received:\n%1\n%2\n%3")		},

  // positive values: credit card company successfully processed the
  // transaction but there was a local failure
  {   1, QT_TRANSLATE_NOOP("CreditCardProcessor", "Database Error")						},
  {   2, QT_TRANSLATE_NOOP("CreditCardProcessor", "Could not generate a unique key for the ccpay table.")	},
  {   3, QT_TRANSLATE_NOOP("CreditCardProcessor", "Stored Procedure Error")					},
  {   4, QT_TRANSLATE_NOOP("CreditCardProcessor", "The Credit Card transaction completed successfully but "
	    "it was not recorded correctly:\n%1")			},
  {   5, QT_TRANSLATE_NOOP("CreditCardProcessor", "The Server is %2 and is expected to be %3 in %1 mode, "
            "while the Port is %4 and is expected to be %5. Credit Card "
            "processing transactions may fail.")                        },
  {   6, QT_TRANSLATE_NOOP("CreditCardProcessor", "The Server is %2 and is expected to be %3 in %1 mode."
            "Credit Card processing transactions may fail.")            },
  {   7, QT_TRANSLATE_NOOP("CreditCardProcessor", "The Port is %2 and is expected to be %3 in %1 mode. "
            "Credit Card processing transactions may fail.")            },
  {  94, QT_TRANSLATE_NOOP("CreditCardProcessor", "There was a problem printing the credit card receipt.")    },
  {  96, QT_TRANSLATE_NOOP("CreditCardProcessor", "This transaction failed the CVV check but will be "
	    "processed anyway.")					},
  {  97, QT_TRANSLATE_NOOP("CreditCardProcessor", "This transaction failed the Address Verification check "
	    "but will be processed anyway.")				},

};

CreditCardProcessor::FraudCheckResult::FraudCheckResult(QChar pcode, int /*TODO: FraudChecks*/ psev, QString ptext)
{
  code = pcode;
  sev  = psev;
  text = ptext;
}

/** \brief Construct and initialize a default CreditCardProcessor.

    This should never be called except by the constructor of a subclass.
    This should always be called by the constructor of a subclass.
    This constructor initializes _msgHash as well as some private and
    protected fields.

 */
CreditCardProcessor::CreditCardProcessor()
{
  if (DEBUG)
    qDebug("CCP:CreditCardProcessor()");
  _defaultLivePort    = 0;
  _defaultLiveServer  = "live.creditcardprocessor.com";
  _defaultTestPort    = 0;
  _defaultTestServer  = "test.creditcardprocessor.com";
  _errorMsg           = "";
  _http               = 0;
  _ignoreSslErrors    = _metrics->boolean("CCIgnoreSSLErrors");

  for (unsigned int i = 0; i < sizeof(messages) / sizeof(messages[0]); i++)
    _msgHash.insert(messages[i].code, tr(messages[i].text));

  _avsCodes.append(new FraudCheckResult('A', NoMatch  | PostalCode, tr("Street Address matches but not Postal Code")));
  _avsCodes.append(new FraudCheckResult('B', NotAvail | Address,    tr("Address not provided for AVS check")));
  _avsCodes.append(new FraudCheckResult('E', Invalid,               tr("Address Verification error")));
  _avsCodes.append(new FraudCheckResult('G', Unsupported,           tr("Card issuing bank is not a U.S. bank")));
  _avsCodes.append(new FraudCheckResult('N', NoMatch | Address | PostalCode, tr("No match on Street Address or Postal Code")));
  _avsCodes.append(new FraudCheckResult('P', Unsupported,           tr("Address Verification does not apply to this transaction")));
  _avsCodes.append(new FraudCheckResult('R', ServiceUnavailable,    tr("Retry - system unavailable or timed out")));
  _avsCodes.append(new FraudCheckResult('S', Unsupported,           tr("Address Verification service not supported")));
  _avsCodes.append(new FraudCheckResult('U', NotAvail | Address,    tr("Address information not available")));
  _avsCodes.append(new FraudCheckResult('W', NoMatch  | Address,    tr("9-Digit Postal Code matches but not Street Address")));
  _avsCodes.append(new FraudCheckResult('X', Match,                 tr("Street Address and 9-digit Postal Code match")));
  _avsCodes.append(new FraudCheckResult('Y', Match,                 tr("Street Address and 5-digit Postal Code match")));
  _avsCodes.append(new FraudCheckResult('Z', NoMatch  | Address,    tr("5-Digit Postal Code matches but not Street Address")));

  _cvvCodes.append(new FraudCheckResult('M', Match,        tr("CVV matches")));
  _cvvCodes.append(new FraudCheckResult('N', NoMatch,      tr("CVV does not match")));
  _cvvCodes.append(new FraudCheckResult('P', NotProcessed, tr("CVV was not processed")));
  _cvvCodes.append(new FraudCheckResult('S', NotAvail,     tr("CVV should be on the card but was not supplied")));
  _cvvCodes.append(new FraudCheckResult('U', Unsupported,  tr("Card issuing bank was not certified for CVV")));
  _cvvCodes.append(new FraudCheckResult('X', Unsupported,  tr("Card Verification is not supported for this processor or card type")));
}

CreditCardProcessor::~CreditCardProcessor()
{
  while (! _avsCodes.isEmpty())
    delete _avsCodes.takeFirst();

  while (! _cvvCodes.isEmpty())
    delete _cvvCodes.takeFirst();
}

/** \brief Get a new instance of a specific CreditCardProcessor subclass.

    This method is used to retrieve a CreditCardProcessor for actual use
    by the application, rather than calling
    CreditCardProcessor::CreditCardProcessor.

    getProcessor retrieves the right subclass for the current configuration.

    \param pcompany This causes the method to instantiate the subclass
                    for the named service, rather than the configured
                    service.  pcompany should be an empty string
                    except when checking for errors in configCC

    \return An instance of a CreditCardProcessor subclass

    \see configCC
 */
CreditCardProcessor * CreditCardProcessor::getProcessor(const QString pcompany)
{
  if (DEBUG)
    qDebug("CCP:getProcessor(%s)", pcompany.toAscii().data());

  if (pcompany == "Authorize.Net")
    return new AuthorizeDotNetProcessor();

  else if (pcompany == "Verisign")
    return new VerisignProcessor();

  else if (pcompany == "YourPay")
    return new YourPayProcessor();

  else if (pcompany == "Paymentech")
    return new PaymentechProcessor();

  else if (pcompany == "CyberSource")
    return new CyberSourceProcessor();

  else if (pcompany == "External")
    return new ExternalCCProcessor();

  else if (! pcompany.isEmpty())
  {
    _errorMsg = errorMsg(-14).arg(pcompany);
    return 0;
  }

  CreditCardProcessor *processor = 0;

  if (_metrics->value("CCCompany") == "Authorize.Net")
    processor = new AuthorizeDotNetProcessor();

  else if (_metrics->value("CCCompany") == "Verisign")
    processor = new VerisignProcessor();

  else if ((_metrics->value("CCCompany") == "YourPay"))
    processor = new YourPayProcessor();

  else if ((_metrics->value("CCCompany") == "Paymentech"))
    processor = new PaymentechProcessor();

  else if ((_metrics->value("CCCompany") == "CyberSource"))
    processor = new CyberSourceProcessor();

  else if ((_metrics->value("CCCompany") == "External"))
    processor = new ExternalCCProcessor();

  else
    _errorMsg = errorMsg(-14).arg(_metrics->value("CCServer"));

  // reset to 0 if the config is bad and we're not configuring the system
  if (processor && processor->testConfiguration() < 0 && pcompany.isEmpty())
  {
    delete processor;
    processor = 0;
  }

  if (processor)
    processor->reset();

  return processor;
}

/** \brief Processes pre-authorization transactions.

    This method performs application-level error checking and
    all of the database work required
    to handle a pre-authorization transaction. It calls
    doAuthorize to handle the direct communication with the service.

    \warning This method should never be overridden.
             Service-specific functionality should be implemented
             in the doAuthorize method of the service' subclass.

    \param[in]  pccardid   The internal id of the credit card to preauthorize
    \param[in]  pcvv       The CVV/CCV code of the credit card to preauthorize
    \param[in]  pamount    The total amount to preauthorize. If the credit card processor
                           does not preauthorize the full amount requested, the
                           database will store the actual amount that was authorized
                           If this occurs, the application will display an error if the
                           amount authorized is 0 or a warning if the amount authorized is
                           greater than 0.
    \param[in]  ptax       The subportion of the total which is tax
    \param[in]  ptaxexempt Whether or not this transaction is tax exempt
    \param[in]  pfreight   The subportion of the total which is freight
    \param[in]  pduty      The subportion of the total which is customs duty
    \param[in]  pcurrid    The %currency of the amount to preauthorize
    \param[in,out] pneworder The order number associated with this preauthorization
    \param[out] preforder  The reference number associated with this
                           preauthorization (may be required to 'capture' the
                           preauthorization)
    \param[out] pccpayid   The ccpay_id of the record created by this
                           transaction
    \param[in]  preftype   Either \c cohead or \c cashrcpt or blank
    \param[in,out] prefid  The cashrcpt_id or cohead_id associated with this
                           transaction

    \return An index into _errMsg; 0 indicates success
 */
int CreditCardProcessor::authorize(const int pccardid, const int pcvv, const double pamount, double ptax, bool ptaxexempt, double pfreight, double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int &prefid)
{
  if (DEBUG)
    qDebug("CCP:authorize(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid,
	   preftype.toAscii().data(), prefid);
  reset();

  if (pamount <= 0)
  {
    _errorMsg = errorMsg(-21);
    return -21;
  }

  QString ccard_x;
  int returnVal = checkCreditCard(pccardid, pcvv, ccard_x);
  if (returnVal < 0)
    return returnVal;

  if (_metrics->boolean("CCConfirmPreauth") &&
      QMessageBox::question(0,
		    tr("Confirm Preauthorization of Credit Card Purchase"),
		    tr("<p>Are you sure that you want to preauthorize "
		       "a charge to credit card %1 in the amount of %2 %3?")
		       .arg(ccard_x)
		       .arg(CurrDisplay::currSymbol(pcurrid))
		       .arg(pamount),
		    QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
  {
    _errorMsg = errorMsg(-70);
    return -70;
  }

  ParameterList dbupdateinfo;
  double amount = pamount;
  returnVal = doAuthorize(pccardid, pcvv, amount, ptax, ptaxexempt, pfreight, pduty, pcurrid, pneworder, preforder, pccpayid, dbupdateinfo);
  if (returnVal == -70)
    return -70;
  else if (returnVal > 0)
    _errorMsg = errorMsg(4).arg(_errorMsg);

  int ccpayReturn = updateCCPay(pccpayid, dbupdateinfo);
  if (returnVal == 0 && ccpayReturn != 0)
    returnVal = ccpayReturn;

  if (returnVal >= 0)
  {
    returnVal = fraudChecks();
    if (returnVal < 0)
    {
      int voidReturnVal = voidPrevious(pccpayid);
      return (voidReturnVal < 0) ? voidReturnVal : returnVal;
    }

    XSqlQuery cashq;
    if (preftype == "cashrcpt")
    {
      if (prefid <= 0)
      {
	cashq.exec("SELECT NEXTVAL('cashrcpt_cashrcpt_id_seq') AS cashrcpt_id;");
	if (cashq.first())
	  prefid = cashq.value("cashrcpt_id").toInt();
	else if (q.lastError().type() != QSqlError::NoError)
	{
	  _errorMsg = errorMsg(4).arg(cashq.lastError().databaseText());
	  // TODO: log an event?
	  return 1;
	}

	cashq.prepare("INSERT INTO cashrcpt (cashrcpt_id,"
		  "  cashrcpt_cust_id, cashrcpt_amount, cashrcpt_curr_id,"
		  "  cashrcpt_fundstype, cashrcpt_docnumber,"
		  "  cashrcpt_bankaccnt_id, cashrcpt_notes, cashrcpt_distdate, cashrcpt_usecustdeposit) "
		  "SELECT :cashrcptid,"
		  "       ccpay_cust_id, :amount, :curr_id,"
		  "       ccard_type, ccpay_r_ordernum,"
		  "       ccbank_bankaccnt_id, :notes, current_date, :custdeposit"
		  "  FROM ccpay, ccard LEFT OUTER JOIN ccbank ON (ccard_type=ccbank_ccard_type)"
		  " WHERE (ccpay_ccard_id=ccard_id);");
      }
      else
	cashq.prepare( "UPDATE cashrcpt "
		       "SET cashrcpt_cust_id=ccard_cust_id,"
		       "    cashrcpt_amount=:amount,"
		       "    cashrcpt_fundstype=ccard_type,"
		       "    cashrcpt_bankaccnt_id=ccbank_bankaccnt_id,"
		       "    cashrcpt_distdate=CURRENT_DATE,"
		       "    cashrcpt_notes=:notes, "
		       "    cashrcpt_curr_id=:curr_id,"
                       "    cashrcpt_usecustdeposit=:custdeposit "
		       "FROM ccard LEFT OUTER JOIN ccbank ON (ccard_type=ccbank_ccard_type) "
		       "WHERE ((cashrcpt_id=:cashrcptid)"
		       "  AND  (ccard_id=:ccardid));" );

      cashq.bindValue(":cashrcptid",   prefid);
      cashq.bindValue(":ccardid",      pccardid);
      cashq.bindValue(":amount",       amount);
      cashq.bindValue(":curr_id",      pcurrid);
      cashq.bindValue(":notes",        "Credit Card Pre-Authorization");
      cashq.bindValue(":custdeposit",  _metrics->boolean("EnableCustomerDeposits"));
      cashq.exec();
      if (cashq.lastError().type() != QSqlError::NoError)
      {
	_errorMsg = errorMsg(4).arg(cashq.lastError().databaseText());
	// TODO: log an event?
	returnVal = 1;
      }
    }
    else if (preftype == "cohead")
    {
      cashq.prepare("INSERT INTO payco VALUES"
		    " (:payco_ccpay_id, :payco_cohead_id,"
		    "  :payco_amount, :payco_curr_id);");
      cashq.bindValue(":payco_ccpay_id",  pccpayid);
      cashq.bindValue(":payco_cohead_id", prefid);
      cashq.bindValue(":payco_amount",    amount);
      cashq.bindValue(":payco_curr_id",   pcurrid);
      cashq.exec();
      if (cashq.lastError().type() != QSqlError::NoError)
      {
	_errorMsg = errorMsg(4).arg(cashq.lastError().databaseText());
	// TODO: log an event?
	returnVal = 1;
      }
    }

    if (_metrics->boolean("CCPrintReceipt"))
    {
      int receiptReturn = printReceipt(pccpayid);
      if (receiptReturn != 0 && returnVal == 0)
        returnVal = receiptReturn;
    }
  }

  return returnVal;
}

/** \brief Processes charge transactions.

    This method performs application-level error checking and
    all of the database work required
    to handle a credit card charge transaction. It calls
    doCharge to handle the direct communication with the service.

    \warning This method should never be overridden.
             Service-specific functionality should be implemented
             in the doCharge method of the service' subclass.

    \param[in]  pccardid   The internal id of the credit card to charge
    \param[in]  pcvv       The CVV/CCV code of the credit card to charge
    \param[in]  pamount    The total amount to charge
    \param[in]  ptax       The subportion of the total which is tax
    \param[in]  ptaxexempt Whether or not this transaction is tax exempt
    \param[in]  pfreight   The subportion of the total which is freight
    \param[in]  pduty      The subportion of the total which is customs duty
    \param[in]  pcurrid    The %currency of the amount to charge
    \param[in,out] pneworder The order number associated with this charge
    \param[out] preforder  The reference number associated with this charge
    \param[out] pccpayid   The ccpay_id of the record created by this
                           transaction
    \param[in]  preftype   Either \c cohead or \c cashrcpt or blank
    \param[in,out] prefid  The cashrcpt_id or cohead_id associated with this
                           transaction

    \return An index into _errMsg; 0 indicates success
 */
int CreditCardProcessor::charge(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int &prefid)
{
  if (DEBUG)
    qDebug("CCP:charge(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid,
	   preftype.toAscii().data(), prefid);
  reset();

  if (pamount <= 0)
  {
    _errorMsg = errorMsg(-21);
    return -21;
  }

  if (preftype == "cohead" && prefid < 0)
  {
    _errorMsg = errorMsg(-40).arg(preftype).arg(prefid);
    return -40;
  }

  QString ccard_x;
  int returnVal = checkCreditCard(pccardid, pcvv, ccard_x);
  if (returnVal < 0)
    return returnVal;

  if (_metrics->boolean("CCConfirmCharge") &&
      QMessageBox::question(0, tr("Confirm Charge of Credit Card Purchase"),
	      tr("Are you sure that you want to charge credit card %1 "
		 "in the amount of %2 %3?")
		 .arg(ccard_x)
		 .arg(CurrDisplay::currSymbol(pcurrid))
		 .arg(pamount),
	      QMessageBox::Yes | QMessageBox::Default,
	      QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
  {
    _errorMsg = errorMsg(-72);
    return -72;
  }

  ParameterList dbupdateinfo;
  returnVal = doCharge(pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid, pneworder, preforder, pccpayid, dbupdateinfo);
  if (returnVal == -72)
    return -72;
  else if (returnVal > 0)
    _errorMsg = errorMsg(4).arg(_errorMsg);

  int ccpayReturn = updateCCPay(pccpayid, dbupdateinfo);
  if (returnVal == 0 && ccpayReturn != 0)
    returnVal = ccpayReturn;

  if (returnVal >= 0)
  {
    returnVal = fraudChecks();
    if (returnVal < 0)
    {
      int voidReturnVal = voidPrevious(pccpayid);
      return (voidReturnVal < 0) ? voidReturnVal : returnVal;
    }

    XSqlQuery cashq;
    cashq.prepare("SELECT postCCcashReceipt(:ccpayid,"
                  "                         :docid, :doctype) AS cm_id;");
    cashq.bindValue(":ccpayid",   pccpayid); 
    cashq.bindValue(":doctype",   preftype);
    cashq.bindValue(":docid",     prefid);
    cashq.exec();
    if (cashq.first())
    {
      int cm_id = cashq.value("cm_id").toInt();
      if (cm_id < 0)
      {
        _errorMsg = "<p>" + errorMsg(4)
                      .arg(storedProcErrorLookup("postCCcashReceipt", cm_id));
        returnVal = 3;
      }
    }
    else if (cashq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = errorMsg(4).arg(cashq.lastError().databaseText());
      // TODO: log an event?
      returnVal = 1;
    }

    if (preftype == "cohead")
    {
      XSqlQuery cashq;

      cashq.prepare("INSERT INTO payco VALUES"
                " (:payco_ccpay_id, :payco_cohead_id,"
                "  :payco_amount, :payco_curr_id);");
      cashq.bindValue(":payco_ccpay_id",  pccpayid);
      cashq.bindValue(":payco_cohead_id", prefid);
      cashq.bindValue(":payco_amount",    pamount);
      cashq.bindValue(":payco_curr_id",   pcurrid);
      cashq.exec();
      if (cashq.lastError().type() != QSqlError::NoError)
      {
        _errorMsg = errorMsg(4).arg(cashq.lastError().databaseText());
        // TODO: log an event?
        returnVal = 1;
      }
    }
  }

  if (_metrics->boolean("CCPrintReceipt"))
  {
    int receiptReturn = printReceipt(pccpayid);
    if (receiptReturn != 0 && returnVal == 0)
      returnVal = receiptReturn;
  }

  return returnVal;
}

/** \brief Processes 'capture' transactions, or charges against a
           prior preauthorization.

    This method performs application-level error checking and all
    of the database work required to 'capture' or complete the
    charge against a prior preauthorization. It calls doChargePreauthorized
    to handle the direct communication with the service.

    \warning This method should never be overridden.
             Service-specific functionality should be implemented
             in the doChargePreauthorized method of the service' subclass.

    \param[in]  pcvv       The CVV/CCV code of the credit card to charge
    \param[in]  pamount    The total amount to charge
    \param[in]  pcurrid    The %currency of the amount to charge
    \param[in,out] pneworder The order number associated with this charge
    \param[in,out] preforder The reference number (preforder) generated by
                             the preauthorization transaction
    \param[in,out] pccpayid  When calling the method, pccpayid should be the
                             ccpay_id of the preauthorization record.
                             On return, this is the ccpay_id of the
                             charge record, which may be the same as the
                             preauthorization record.

    \return An index into _errMsg; 0 indicates success
 */
int CreditCardProcessor::chargePreauthorized(const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("CCP:chargePreauthorized(%d, %f, %d, %s, %s, %d)",
	   pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  reset();

  int ccValidDays = _metrics->value("CCValidDays").toInt();
  if (ccValidDays < 1)
    ccValidDays = 7;

  if (pamount <= 0)
  {
    _errorMsg = errorMsg(-21);
    return -21;
  }

  if (pccpayid < 0)
  {
    _errorMsg = errorMsg(-32);
    return -32;
  }

  XSqlQuery ccq;
  ccq.prepare("SELECT *,"
	    "       currToCurr(ccpay_curr_id, :curr_id, ccpay_amount,"
	    "                  CURRENT_DATE) AS ccpay_amount_converted "
	    "FROM ccpay "
	    "WHERE ((ccpay_status = 'A')"
	    "  AND  (date_part('day', CURRENT_TIMESTAMP - "
	    "                         ccpay_transaction_datetime) < "
	    "        :ccValidDays)"
	    "  AND  (ccpay_id=:id));");
  ccq.bindValue(":id", pccpayid);
  ccq.bindValue(":curr_id",     pcurrid);
  ccq.bindValue(":ccValidDays", ccValidDays);
  ccq.exec();
  if (ccq.first())
  {
    if (pamount > ccq.value("ccpay_amount_converted").toDouble())
    {
      _errorMsg = errorMsg(-33)
		  .arg(ccq.value("ccpay_amount").toString())
		  .arg(pamount);
      return -33;
    }

  }
  else if (ccq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ccq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-34);
    return -34;
  }

  int ccardid = ccq.value("ccpay_ccard_id").toInt();
  preforder = ccq.value("ccpay_r_ordernum").toString();

  ccq.prepare("SELECT * FROM payco WHERE (payco_ccpay_id=:ccpayid)");
  ccq.bindValue(":ccpayid", pccpayid);
  ccq.exec();
  if (ccq.first())
  {
    int coheadid = ccq.value("payco_cohead_id").toInt();
    ccq.prepare("SELECT COUNT(*) AS linecount "
	      "FROM coitem "
	      "WHERE ((coitem_status IN ('O', 'C'))"
	      "  AND  (coitem_cohead_id=:coheadid));");
    ccq.bindValue(":coheadid", coheadid);
    ccq.exec();
    if (ccq.first() && ccq.value("linecount").toInt() <= 0)
    {
      _errorMsg = errorMsg(-35);
      return -35;
    }
    else if (ccq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }
  }
  else if (ccq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ccq.lastError().databaseText();
    return -1;
  }

  QString ccard_x;
  int returnVal = checkCreditCard(ccardid, pcvv, ccard_x);
  if (returnVal < 0)
    return returnVal;

  if (_metrics->boolean("CCConfirmChargePreauth") &&
      QMessageBox::question(0,
	      tr("Confirm Post-authorization of Credit Card Purchase"),
              tr("Are you sure that you want to charge a pre-authorized "
                 "transaction to credit card %1 in the amount of %2 %3?")
		 .arg(ccard_x)
		 .arg(CurrDisplay::currSymbol(pcurrid))
                 .arg(pamount),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
  {
    _errorMsg = errorMsg(-71);
    return -71;
  }

  ParameterList dbupdateinfo;
  returnVal = doChargePreauthorized(ccardid, pcvv, pamount, pcurrid, pneworder, preforder, pccpayid, dbupdateinfo);
  if (returnVal == -71)
    return -71;
  else if (returnVal > 0)
    _errorMsg = errorMsg(4).arg(_errorMsg);

  int ccpayReturn = updateCCPay(pccpayid, dbupdateinfo);
  if (returnVal == 0 && ccpayReturn != 0)
    returnVal = ccpayReturn;

  if (returnVal >= 0)
  {
    returnVal = fraudChecks();
    if (returnVal < 0)
    {
      int voidReturnVal = voidPrevious(pccpayid);
      return (voidReturnVal < 0) ? voidReturnVal : returnVal;
    }

    ccq.prepare("INSERT INTO cashrcpt ("
	      "  cashrcpt_cust_id, cashrcpt_amount, cashrcpt_curr_id,"
	      "  cashrcpt_fundstype, cashrcpt_docnumber,"
	      "  cashrcpt_bankaccnt_id, cashrcpt_notes, cashrcpt_distdate, cashrcpt_usecustdeposit) "
	      "SELECT ccpay_cust_id, :amount, :curr_id,"
	      "       ccard_type, ccpay_r_ordernum,"
	      "       ccbank_bankaccnt_id, :notes, current_date,"
              "       :custdeposit"
	      "  FROM ccpay, ccard LEFT OUTER JOIN ccbank ON (ccard_type=ccbank_ccard_type) "
	      "WHERE ((ccpay_ccard_id=ccard_id)"
	      "  AND  (ccpay_id=:pccpayid));");
    ccq.bindValue(":pccpayid",     pccpayid);
    ccq.bindValue(":amount",       pamount);
    ccq.bindValue(":curr_id",      pcurrid);
    ccq.bindValue(":notes",        "Converted Pre-auth");
    ccq.bindValue(":custdeposit",  _metrics->boolean("EnableCustomerDeposits"));
    ccq.exec();
    if (ccq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = errorMsg(4).arg(ccq.lastError().databaseText());
      // TODO: log an event?
      returnVal = 1;
    }

    if (_metrics->boolean("CCPrintReceipt"))
    {
      int receiptReturn = printReceipt(pccpayid);
      if (receiptReturn != 0 && returnVal == 0)
        returnVal = receiptReturn;
    }
  }

  return returnVal;
}

/** \brief Test whether common credit card processing configuration options
           are consistent.

    Calls toTestConfiguration to check if service-specific options are set.
 */
int CreditCardProcessor::testConfiguration()
{
  if (DEBUG)
    qDebug("CCP:testConfiguration()");
  reset();

  if (!_privileges->check("ProcessCreditCards"))
  {
    _errorMsg = errorMsg(-2);
    return -2;
  }

  if(!_metrics->boolean("CCAccept"))
  {
    _errorMsg = errorMsg(-3);
    return -3;
  }

  XSqlQuery ccbankq("SELECT ccbank_id"
                    "  FROM ccbank"
                    " WHERE (ccbank_bankaccnt_id IS NOT NULL);");
  if (ccbankq.first())
    ; // we're ok - we can accept at least one credit card
  else if (ccbankq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ccbankq.lastError().text();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-4);
    return -4;
  }

  if (omfgThis->_key.isEmpty())
  {
    _errorMsg = errorMsg(-5);
    return -5;
  }

  if(_metrics->boolean("CCUseProxyServer"))
  {
    _plogin = _metricsenc->value("CCProxyLogin");
    if (_plogin.isEmpty())
    {
      _errorMsg = errorMsg(-6);
      return -6;
    }

    _ppassword = _metricsenc->value("CCPassword");
    if (_ppassword.isEmpty())
    {
      _errorMsg = errorMsg(-7);
      return -7;
    }

    _pserver = _metrics->value("CCProxyServer");
    if (_pserver.isEmpty())
    {
      _errorMsg = errorMsg(-8);
      return -8;
    }

    _pport = _metrics->value("CCProxyPort");
    if (_pport.isEmpty())
    {
      _errorMsg = errorMsg(-9);
      return -9;
    }
  }

  if (! handlesChecks() && ! handlesCreditCards())
  {
    _errorMsg = errorMsg(-20);
    return -20;
  }

  if (isLive() == isTest())	// if both true or both false
  {
    _errorMsg = errorMsg(-13);
    return -13;
  }

  int returnValue = doTestConfiguration();

  // now handle warnings
  if (returnValue >= 0)
  {
    QString serverStr  = buildURL(_metrics->value("CCServer"), _metrics->value("CCPort"), false);
    QString defaultStr = buildURL(defaultServer(),        QString(defaultPort(isLive())), false);

    if (serverStr != defaultStr &&
        _metrics->value("CCPort").toInt() != defaultPort(isLive()) &&
        _metrics->value("CCPort").toInt() != 0)
    {
      _errorMsg = errorMsg(5).arg(isLive() ? tr("Live") : tr("Test"))
                             .arg(_metrics->value("CCServer"))
                             .arg(defaultServer())
                             .arg(_metrics->value("CCPort"))
                             .arg(defaultPort(isLive()));
      returnValue = 5;
    }
    else if (serverStr != defaultStr)
    {
      _errorMsg = errorMsg(6).arg(isLive() ? tr("Live") : tr("Test"))
                             .arg(_metrics->value("CCServer"))
                             .arg(defaultServer());
      returnValue = 6;
    }
    else if (_metrics->value("CCPort").toInt() != defaultPort(isLive()) &&
             _metrics->value("CCPort").toInt() != 0 &&
            ! _metrics->value("CCPort").isEmpty())
    {
      _errorMsg = errorMsg(7).arg(isLive() ? tr("Live") : tr("Test"))
                             .arg(_metrics->value("CCPort"))
                             .arg(defaultPort(isLive()));
      returnValue = 7;
    }
  }

  return returnValue;
}

/** \brief Processes credit transactions.

    This method performs application-level error checking and all
    of the database work required to credit a prior charge.
    It calls doCredit to handle the direct communication with the service.

    \warning This method should never be overridden.
             Service-specific functionality should be implemented
             in the doCredit method of the service' subclass.

    \param[in]  pccardid   The internal id of the credit card to credit
    \param[in]  pcvv       The CVV/CCV code of the credit card to credit
    \param[in]  pamount    The total amount to credit
    \param[in]  ptax       The subportion of the total which is tax
    \param[in]  ptaxexempt Whether or not this transaction is tax exempt
    \param[in]  pfreight   The subportion of the total which is freight
    \param[in]  pduty      The subportion of the total which is customs duty
    \param[in]  pcurrid    The %currency of the amount to credit
    \param[in,out] pneworder The order number associated with this credit
    \param[in,out] preforder The reference number (preforder) generated by
                             the charge transaction
    \param[in,out] pccpayid  When calling the method, pccpayid should be the
                             ccpay_id of the original charge record.
                             On return, this is the ccpay_id of the
                             credit record.
    \param[in]  preftype   Either \c cohead or \c cashrcpt or blank
    \param[in,out] prefid  The cashrcpt_id or cohead_id associated with this
                           transaction

    \return An index into _errMsg; 0 indicates success
 */
int CreditCardProcessor::credit(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int &prefid)
{
  if (DEBUG)
    qDebug("CCP:credit(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid,
	   preftype.toAscii().data(), prefid);
  reset();

  if (preftype == "cohead" && prefid < 0)
  {
    _errorMsg = errorMsg(-40).arg(preftype).arg(prefid);
    return -40;
  }

  QString ccard_x;
  int returnVal = checkCreditCard(pccardid, pcvv,  ccard_x);
  if (returnVal < 0)
    return returnVal;

  if (_metrics->boolean("CCConfirmCredit") &&
      QMessageBox::question(0,
	      tr("Confirm Credit Card Credit"),
              tr("Are you sure that you want to refund %2 %3 to credit card %1?")
		 .arg(ccard_x)
                 .arg(CurrDisplay::currSymbol(pcurrid))
                 .arg(pamount),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
  {
    _errorMsg = errorMsg(-73);
    return -73;
  }

  if (pccpayid > 0)
  {
    int oldccpayid = pccpayid;

    XSqlQuery ccq;
    ccq.exec("SELECT NEXTVAL('ccpay_ccpay_id_seq') AS ccpay_id;");
    if (ccq.first())
      pccpayid = ccq.value("ccpay_id").toInt();
    else if (ccq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }
    else // no rows found is fatal because we haven't processed the credit yet
    {
      _errorMsg = errorMsg(2);
      return -1;
    }

    int next_seq = -1;
    ccq.prepare("SELECT MAX(COALESCE(ccpay_order_number_seq, -1)) + 1"
		"       AS next_seq "
		"  FROM ccpay "
		" WHERE (ccpay_order_number=:ccpay_order_number);");
    ccq.bindValue(":ccpay_order_number", preforder);
    ccq.exec();
    if (ccq.first())
      next_seq = ccq.value("next_seq").toInt();
    else if (ccq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }

    ccq.prepare( "INSERT INTO ccpay ("
		 "    ccpay_id, ccpay_ccard_id, ccpay_cust_id,"
		 "    ccpay_auth_charge, ccpay_auth,"
		 "    ccpay_amount,"
		 "    ccpay_curr_id, ccpay_type, ccpay_status,"
                 "    ccpay_order_number, ccpay_order_number_seq, "
                 "    ccpay_ccpay_id "
		 ") SELECT "
		 "    :newccpayid, ccpay_ccard_id, ccpay_cust_id,"
		 "    ccpay_auth_charge, ccpay_auth,"
		 "    :amount, :currid, 'R', 'X',"
                 "    ccpay_order_number, :nextseq, ccpay_id "
		 "FROM ccpay "
		 "WHERE (ccpay_id=:oldccpayid);");

    ccq.bindValue(":newccpayid", pccpayid);
    ccq.bindValue(":currid",     pcurrid);
    ccq.bindValue(":amount",     pamount);
    ccq.bindValue(":nextseq",    next_seq);
    ccq.bindValue(":oldccpayid", oldccpayid);
    ccq.exec();
    if (ccq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }

    ccq.prepare("SELECT ccpay_r_ordernum FROM ccpay WHERE (ccpay_id=:ccpayid);");
    ccq.bindValue(":ccpayid", oldccpayid);
    ccq.exec();
    if (ccq.first())
      preforder = ccq.value("ccpay_r_ordernum").toString();
    else if (ccq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }
    else
    {
      _errorMsg = errorMsg(-50);
      return -50;
    }
  }

  ParameterList dbupdateinfo;
  returnVal = doCredit(pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid, pneworder, preforder, pccpayid, dbupdateinfo);
  if (returnVal < 0)
    return returnVal;
  else if (returnVal > 0)
    _errorMsg = errorMsg(4).arg(_errorMsg);

  int ccpayReturn = updateCCPay(pccpayid, dbupdateinfo);
  if (returnVal == 0 && ccpayReturn != 0)
    returnVal = ccpayReturn;

  if (returnVal >= 0)
  {
    returnVal = fraudChecks();
    if (returnVal < 0)
    {
      int voidReturnVal = voidPrevious(pccpayid);
      return (voidReturnVal < 0) ? voidReturnVal : returnVal;
    }

    if (pccpayid > 0)
    {
      XSqlQuery cq;
      cq.prepare("SELECT postCCCredit(:ccpayid, :reftype, :refid) AS result;");
      cq.bindValue(":ccpayid", pccpayid);
      cq.bindValue(":reftype", preftype);
      cq.bindValue(":refid",   prefid);
      cq.exec();
      if (cq.first())
      {
	int result = cq.value("result").toInt();
	if (result < 0)
	{
	  _errorMsg = "<p>" +
		      errorMsg(4).arg(storedProcErrorLookup("postCCCredit",
								   result));
	  returnVal = 1;
	}
      }
      else if (cq.lastError().type() != QSqlError::NoError)
      {
	_errorMsg = errorMsg(4).arg(cq.lastError().databaseText());
	returnVal = 1;
      }
    }

    if (_metrics->boolean("CCPrintReceipt"))
    {
      int receiptReturn = printReceipt(pccpayid);
      if (receiptReturn != 0 && returnVal == 0)
        returnVal = receiptReturn;
    }
  }

  return returnVal;
}

/** \brief Reverse a preauthorization.

    This method performs application-level error checking and all
    of the database work required to reverse a prior preauthorization.
    This makes available to the card holder the funds that have been
    reserved by that preauthorization.

    \warning This method should never be overridden.
              Service-specific functionality should be implemented in
              the doReversePreauthorized method of the service' subclass.

    \param pamount   The amount to be reversed, not necessarily the same
                     as the amount originally authorized.
    \param pcurrid   The currency of the amount.
    \param pneworder The order number associated with the preauthorization
    \param preforder The reference number (preforder) generated by
                     the preauthorization transaction
    \param pccpayid  The internal id of the preauthorization transaction
    \param preftype  Either \c cohead or \c cashrcpt or blank
    \param prefid    The cashrcpt_id or cohead_id associated with the
                     preauthorization transaction

    \return An index into _errMsg; 0 indicates success
  */
int CreditCardProcessor::reversePreauthorized(const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int prefid)
{
  if (DEBUG)
    qDebug("CCP:reversePreauthorized(%f, %d, %s, %s, %d)",
           pamount, pcurrid, qPrintable(pneworder), qPrintable(preforder), pccpayid);
  reset();

  XSqlQuery ccq;
  ccq.prepare("SELECT * FROM ccpay WHERE (ccpay_id=:ccpayid AND ccpay_status = 'A');");
  ccq.bindValue(":ccpayid", pccpayid);
  ccq.exec();

  int ccardid;
  if (ccq.first())
    ccardid = ccq.value("ccpay_ccard_id").toInt();
  else if (ccq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ccq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-60);
    return -60;
  }

  // don't checkCreditCard because we want to reverse the authorization regardless

  QString neworder = ccq.value("ccpay_order_number").toString();
  QString reforder = ccq.value("ccpay_r_ordernum").toString();
  QString approval = ccq.value("ccpay_r_ref").toString();
  ParameterList dbupdateinfo;
  int returnVal = doReversePreauthorized(ccq.value("ccpay_amount").toDouble(),
                                         ccq.value("ccpay_curr_id").toInt(),
                                         neworder, reforder,
                                         pccpayid, dbupdateinfo);
  if (returnVal == -74)
    return -74;
  else if (returnVal < 0)
    return returnVal;
  else if (returnVal > 0)
    _errorMsg = errorMsg(4).arg(_errorMsg);

  returnVal = updateCCPay(pccpayid, dbupdateinfo);
  if (returnVal < 0)
    return returnVal;

  double newamount = 0.0;
  ccq.prepare("UPDATE ccpay SET ccpay_amount = ccpay_amount - :revamount,"
              "             ccpay_status = CASE WHEN ccpay_amount - :revamount = 0 THEN 'R'"
              "                                 ELSE ccpay_status END"
              " WHERE ccpay_id = :ccpayid RETURNING ccpay_amount;");
  ccq.bindValue(":ccpayid", pccpayid);
  ccq.bindValue(":revamount", pamount);
  ccq.exec();
  if (ccq.first())
  {
    newamount = ccq.value("ccpay_amount").toDouble();
    if (DEBUG)
      qDebug("CCP::reverseAuthorization() set ccpay_amount to %f", newamount);
  }
  else if (ccq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = errorMsg(4).arg(ccq.lastError().databaseText());
    returnVal = 1;
  }

  XSqlQuery cashq;
  if (preftype == "cashrcpt")
  {
    cashq.prepare( "UPDATE cashrcpt "
                   "SET cashrcpt_cust_id=ccard_cust_id,"
                   "    cashrcpt_amount=:amount,"
                   "    cashrcpt_fundstype=ccard_type,"
                   "    cashrcpt_bankaccnt_id=ccbank_bankaccnt_id,"
                   "    cashrcpt_distdate=CURRENT_DATE,"
                   "    cashrcpt_notes=cashrcpt || E'\\n' || :notes, "
                   "    cashrcpt_curr_id=:curr_id,"
                   "    cashrcpt_usecustdeposit=:custdeposit "
                   "FROM ccard LEFT OUTER JOIN ccbank ON (ccard_type=ccbank_ccard_type) "
                   "WHERE ((cashrcpt_id=:cashrcptid)"
                   "  AND  (ccard_id=:ccardid)"
                   "  AND NOT cashrcpt_posted);" );

    cashq.bindValue(":cashrcptid",   prefid);
    cashq.bindValue(":ccardid",      ccardid);
    cashq.bindValue(":amount",       newamount);
    cashq.bindValue(":curr_id",      pcurrid);
    cashq.bindValue(":notes",        tr("Credit Card Pre-Authorization %1 reversed")
                                     .arg(newamount == 0.0 ? tr("fully") : tr("partially")));
    cashq.bindValue(":custdeposit",  _metrics->boolean("EnableCustomerDeposits"));
    cashq.exec();
    if (cashq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = errorMsg(4).arg(cashq.lastError().databaseText());
      // TODO: log an event?
      returnVal = 1;
    }
  }
  else if (preftype == "cohead")
  {
    if (newamount == 0.0)
      cashq.prepare("DELETE FROM payco"
                    " WHERE ((payco_ccpay_id=:payco_ccpay_id)"
                    "    AND (payco_cohead_id=:payco_cohead_id));");
    else
    {
      cashq.prepare("INSERT INTO payco VALUES"
                    " (:payco_ccpay_id, :payco_cohead_id,"
                    "  :payco_amount, :payco_curr_id);");
      cashq.bindValue(":payco_amount",    pamount);
      cashq.bindValue(":payco_curr_id",   pcurrid);
    }
    cashq.bindValue(":payco_ccpay_id",  pccpayid);
    cashq.bindValue(":payco_cohead_id", prefid);
    cashq.exec();
    if (cashq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = errorMsg(4).arg(cashq.lastError().databaseText());
      // TODO: log an event?
      returnVal = 1;
    }
  }

  if (_metrics->boolean("CCPrintReceipt"))
  {
    int receiptReturn = printReceipt(pccpayid);
    if (receiptReturn != 0 && returnVal == 0)
      returnVal = receiptReturn;
  }

  return returnVal;
}

/** \brief Processes void transactions.

    This method performs application-level error checking and all
    of the database work required to void a prior transaction.
    It calls doVoid to handle the direct communication with the service.

    \warning This method should never be overridden.
             Service-specific functionality should be implemented
             in the doVoid method of the service' subclass.

    \param pccpayid  This should be the ccpay_id of the original transaction record.

    \return An index into _errMsg; 0 indicates success
 */
int CreditCardProcessor::voidPrevious(int &pccpayid)
{
  if (DEBUG)
    qDebug("CCP:voidPrevious(%d)", pccpayid);
  // don't reset(); because we're probably voiding because of a previous error

  int ccv = -2;

  XSqlQuery ccq;
  ccq.prepare("SELECT * FROM ccpay WHERE (ccpay_id=:ccpayid);");
  ccq.bindValue(":ccpayid", pccpayid);
  ccq.exec();

  int ccardid;
  if (ccq.first())
    ccardid = ccq.value("ccpay_ccard_id").toInt();
  else if (ccq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ccq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-60);
    return -60;
  }

  // don't checkCreditCard because we want to void the transaction regardless

  QString neworder = ccq.value("ccpay_order_number").toString();
  QString reforder = ccq.value("ccpay_r_ordernum").toString();
  QString approval = ccq.value("ccpay_r_ref").toString();
  ParameterList dbupdateinfo;
  int returnVal = doVoidPrevious(ccardid, ccv,
				 ccq.value("ccpay_amount").toDouble(),
				 ccq.value("ccpay_curr_id").toInt(),
				 neworder, reforder, approval,
                                 pccpayid, dbupdateinfo);
  if (returnVal == -74)
    return -74;
  else if (returnVal < 0)
    return returnVal;
  else if (returnVal > 0)
    _errorMsg = errorMsg(4).arg(_errorMsg);

  returnVal = updateCCPay(pccpayid, dbupdateinfo);
  if (returnVal < 0)
    return returnVal;

  ccq.prepare("SELECT postCCVoid(:ccpayid) AS result;");
  ccq.bindValue(":ccpayid", pccpayid);
  ccq.exec();
  if (ccq.first())
  {
    int result = ccq.value("result").toInt();
    if (result < 0)
    {
      _errorMsg = "<p>" +
		  errorMsg(4).arg(storedProcErrorLookup("postCCVoid",
							       result));
      returnVal = 1;
    }
  }
  else if (ccq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = errorMsg(4).arg(ccq.lastError().databaseText());
    returnVal = 1;
  }

  if (_metrics->boolean("CCPrintReceipt"))
  {
    int receiptReturn = printReceipt(pccpayid);
    if (receiptReturn != 0 && returnVal == 0)
      returnVal = receiptReturn;
  }

  return returnVal;
}

/** \brief Returns whether credit card processing is configured in live mode. */
bool CreditCardProcessor::isLive()
{
  return (!_metrics->boolean("CCTest"));
}

/** \brief Returns whether credit card processing is configured in test mode. */
bool CreditCardProcessor::isTest()
{
  return (_metrics->boolean("CCTest"));
}

/** \brief Returns the most recent error message set by CreditCardProcessor
           or one of its subclasses.
 */
QString CreditCardProcessor::errorMsg()
{
  return _errorMsg;
}

/** \brief Returns the error message associated with the given pcode. */
QString CreditCardProcessor::errorMsg(const int pcode)
{
  return _msgHash.value(pcode);
}

/** \brief Check if the given credit card is consistent and active.

    This consistency %check is used in a number of places before executing
    a credit card transaction. It confirms that the given card is marked
    as active in the system and has not expired. If the card has expired
    based on the expiration month and year, then it is marked as expired
    in the database. It also makes sure that the CVV has been entered
    if the system is configured to require it.

    \param pccid The ccard_id of the credit card to %check
    \param pcvv  The CVV from the card holder, -1 if not known,
                 -2 if the caller knows it is not required or available
                 (such as for void transactions).
    \param[out] pccard_x The credit card number, with most of the digits
                         replaced with \c X. Used for reporting errors.
  */
int CreditCardProcessor::checkCreditCard(const int pccid, const int pcvv, QString &pccard_x)
{
  if (DEBUG)
    qDebug("CCP:checkCreditCard(%d, %d)", pccid, pcvv);
  reset();

  if(omfgThis->_key.isEmpty())
  {
    _errorMsg = errorMsg(-5);
    return -5;
  }

  q.prepare( "SELECT ccard_active, ccard_cust_id, "
             "       formatbytea(decrypt(setbytea(ccard_month_expired),"
	     "               setbytea(:key), 'bf')) AS ccard_month_expired,"
             "       formatbytea(decrypt(setbytea(ccard_year_expired),"
	     "               setbytea(:key), 'bf')) AS ccard_year_expired,"
             "       formatccnumber(decrypt(setbytea(ccard_number),"
	     "               setbytea(:key), 'bf')) AS ccard_number_x,"
             "       ccard_type, ccbank_bankaccnt_id "
             "  FROM ccard"
             "     LEFT OUTER JOIN ccbank ON (ccard_type=ccbank_ccard_type)"
             " WHERE (ccard_id=:ccardid);");
  q.bindValue(":ccardid", pccid);
  q.bindValue(":key",     omfgThis->_key);
  q.exec();
  if (q.first())
  {
    pccard_x = q.value("ccard_number_x").toString();

    if (!q.value("ccard_active").toBool())
    {
      _errorMsg = errorMsg(-10).arg(pccard_x);
      return -10;
    }

    if (q.value("ccbank_bankaccnt_id").isNull())
    {
      _errorMsg = errorMsg(-94).arg(q.value("ccard_type").toString());
      return -94;
    }

    if (q.value("ccard_year_expired").toInt() < QDate::currentDate().year()
	 || ((q.value("ccard_year_expired").toInt() == QDate::currentDate().year())
           && q.value("ccard_month_expired").toInt() < QDate::currentDate().month()))
    {
      XSqlQuery xpq;
      xpq.prepare("SELECT expireCreditCard(:custid, setbytea(:key)) AS result;");
      xpq.bindValue(":custid", q.value("ccard_cust_id"));
      xpq.bindValue(":key", omfgThis->_key);
      xpq.exec();
      // ignore errors from expirecreditcard()
      _errorMsg = errorMsg(-11).arg(pccard_x);
      return -11;
    }
  }

  if (pcvv == -1 && _metrics->boolean("CCRequireCVV"))
  {
    _errorMsg = errorMsg(-98);
    return -98;
  }
  else if (pcvv == -1)
  {
    if (QMessageBox::question(0,
	      tr("Confirm No CVV Code"),
              tr("<p>You must confirm that you wish to proceed "
                 "without a CVV code. Would you like to continue?"),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
    {
      _errorMsg = errorMsg(-75);
      return -75;
    }
  }
  else if (pcvv == -2)
    ; // the caller knows that this transaction doesn't have cvv available
  else if (pcvv > 9999) // YP docs are not consistent - 000-999 or 3-4 digits?
  {
    _errorMsg = errorMsg(-99);
    return -99;
  }

  return 0;
}

/** \brief Placeholder for subclasses to override.
     \see authorize
 */
int CreditCardProcessor::doAuthorize(const int pccardid, const int pcvv, double &pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &)
{
  if (DEBUG)
    qDebug("CCP:doAuthorize(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doAuthorize");
  return -19;
}

/** \brief Placeholder for subclasses to override.
    \see charge
 */
int CreditCardProcessor::doCharge(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &)
{
  if (DEBUG)
    qDebug("CCP:doCharge(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	    pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doCharge");
  return -19;
}

/** \brief Placeholder for subclasses to override.
    \see chargePreauthorized
 */
int CreditCardProcessor::doChargePreauthorized(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &)
{
  if (DEBUG)
    qDebug("CCP:doChargePreauthorized(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	    pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doChargePreauthorized");
  return -19;
}

/** \brief Placeholder for subclasses to override.
    \see testConfiguration
 */
int CreditCardProcessor::doTestConfiguration()
{
  if (DEBUG)
    qDebug("CCP:doTestConfiguration()");
  return 0;	// assume that subclasses will override IFF they need to
}

/** \brief Placeholder for subclasses to override.
    \see credit
 */
int CreditCardProcessor::doCredit(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &)
{
  if (DEBUG)
    qDebug("CCP:doCredit(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doCredit");
  return -19;
}

/** \brief Placeholder for subclasses to override.
    \see   reversePreauthorized
  */
int CreditCardProcessor::doReversePreauthorized(const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int pccpayid, ParameterList &/*pparams*/)
{
  if (DEBUG)
    qDebug("CCP:doReversePreauthorized(%f, %d, %s, %s, %d)",
	   pamount, pcurrid, qPrintable(pneworder), qPrintable(preforder), pccpayid);
  _errorMsg = errorMsg(-19).arg("doReversePreauthorized");
  return -19;
}

/** \brief Placeholder for subclasses to override.
    \see   voidPrevious
 */
int CreditCardProcessor::doVoidPrevious(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &)
{
  if (DEBUG)
    qDebug("CCP:doVoidPrevious(%d, %d, %f, %d, %s, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(),
	   papproval.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doVoidPrevious");
  return -19;
}

/** \brief Send an HTTP request to the configured credit card service and wait
           for its response.

    This method is intended to be called by subclasses of
    CreditCardProcessor.  It sends a message to the service using
    HTTP or HTTPS, as set by the configuration, and waits for a
    response. If necessary it applies a local certificate for
    bidirectional encryption.

    It is the caller's responsibility to format an
    appropriate message and decode the response.

    \param[in]  prequest  The string to send via HTTP
    \param[out] presponse The string returned by the service
 */

int CreditCardProcessor::sendViaHTTP(const QString &prequest,
				     QString &presponse)
{
  if (DEBUG)
    qDebug("CCP:sendViaHTTP(input, output) with input:\n%s",
	   prequest.toAscii().data());

  // TODO: find a better place to save this
  if (isTest())
    _metrics->set("CCOrder", prequest);

  QString pemfile;
#ifdef Q_WS_WIN
  pemfile = _metrics->value("CCYPWinPathPEM");
#elif defined Q_WS_MACX
  pemfile = _metrics->value("CCYPMacPathPEM");
#elif defined Q_WS_X11
  pemfile = _metrics->value("CCYPLinPathPEM");
#endif

#ifndef QT_NO_OPENSSL
  /* TODO: specific references to YourPay should be replaced with
     checking a config option indicating that a PEM file is required.
     http://bugreports.qt.nokia.com/browse/QTBUG-13418
     means we must use cURL to handle certificates in some Qt versions.
   */
  if(!_metrics->boolean("CCUseCurl") &&
     (_metrics->value("CCCompany") != "YourPay"
      || (_metrics->value("CCCompany") == "YourPay" && QT_VERSION > 0x040600)))
  {
    if (!pemfile.isEmpty() && (_metrics->value("CCCompany") == "YourPay"))
    {
      QFile pemio(pemfile);
      if (! pemio.exists())
        QMessageBox::warning(0, tr("Could not find PEM file"),
                             tr("<p>Failed to find the PEM file %1")
                             .arg(pemfile));
      else
      {
        QList<QSslCertificate> certlist = QSslCertificate::fromPath(pemfile);
        if (DEBUG) qDebug("%d certificates", certlist.size());
        if (certlist.isEmpty())
          QMessageBox::warning(0, tr("Failed to load Certificate"),
                               tr("<p>There are no Certificates in %1. "
                                  "This may cause communication problems.")
                               .arg(pemfile));
        else if (certlist.at(0).isNull())
          QMessageBox::warning(0, tr("Failed to load Certificate"),
                               tr("<p>Failed to load a Certificate from "
                                  "the PEM file %1. "
                                  "This may cause communication problems.")
                               .arg(pemfile));
        else if (certlist.at(0).isValid())
        {
          if (DEBUG)
            qDebug("Certificate details: valid from %s to %s, issued to %s @ %s in %s, %s",
                   qPrintable(certlist.at(0).effectiveDate().toString("MMM-dd-yyyy")),
                   qPrintable(certlist.at(0).expiryDate().toString("MMM-dd-yyyy")),
                   qPrintable(certlist.at(0).issuerInfo(QSslCertificate::CommonName)),
                   qPrintable(certlist.at(0).issuerInfo(QSslCertificate::Organization)),
                   qPrintable(certlist.at(0).issuerInfo(QSslCertificate::LocalityName)),
                   qPrintable(certlist.at(0).issuerInfo(QSslCertificate::CountryName)));
          QSslConfiguration sslconf = QSslConfiguration::defaultConfiguration();
          sslconf.setLocalCertificate(certlist.at(0));
          QSslConfiguration::setDefaultConfiguration(sslconf);
        }
        else
        {
          QMessageBox::warning(0, tr("Invalid Certificate"),
                               tr("<p>The Certificate in %1 appears to be invalid. "
                                  "This may cause communication problems.")
                               .arg(pemfile));
        }
      }
    }

    QHttp::ConnectionMode cmode = QHttp::ConnectionModeHttps;
    QUrl ccurl(buildURL(_metrics->value("CCServer"), _metrics->value("CCPort"), true));
    if(ccurl.scheme().compare("https", Qt::CaseInsensitive) != 0)
      cmode = QHttp::ConnectionModeHttp;
    _http = new QHttp(ccurl.host(), cmode, _metrics->value("CCPort").toInt());
    connect(_http, SIGNAL(sslErrors(const QList<QSslError> &)),
            this,  SLOT(sslErrors(const QList<QSslError> &)));

    if(_metrics->boolean("CCUseProxyServer"))
    {
      _http->setProxy(_metrics->value("CCProxyServer"), _metrics->value("CCProxyPort").toInt(),
                    _metricsenc->value("CCProxyLogin"), _metricsenc->value("CCPassword"));
    }

    QHttpRequestHeader request("POST", ccurl.encodedPath());
    if(!_extraHeaders.isEmpty())
    {
      request.setValues(_extraHeaders);
    }
    request.setValue("Host", ccurl.host());
    
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    _http->request(request, prequest.toUtf8());
    while(_http->hasPendingRequests() || _http->currentId() != 0)
    {
      QApplication::processEvents(QEventLoop::WaitForMoreEvents);
    }
    QApplication::restoreOverrideCursor();
    if(_http->error() != QHttp::NoError)
    {
      _errorMsg = errorMsg(-18)
                        .arg(ccurl.toString())
                        .arg(_http->error())
                        .arg(_http->errorString());
      return -18;
    }
    presponse = _http->readAll();
  }
  else
#endif // QT_NO_OPENSSL
  {
    // TODO: why have a hard-coded path to curl?
    QProcess proc(this);
    QString curl_path;
  #ifdef Q_WS_WIN
    curl_path = qApp->applicationDirPath() + "\\curl";
  #elif defined Q_WS_MACX
    curl_path = "/usr/bin/curl";
  #elif defined Q_WS_X11
    curl_path = "/usr/bin/curl";
  #endif

    QStringList curl_args;
    curl_args.append( "-k" );
    curl_args.append( "-d" );
    curl_args.append( prequest );

    if (!pemfile.isEmpty() && (_metrics->value("CCCompany") == "YourPay")) // This is currently only used for YourPay
    {
      curl_args.append( "-E" );
      curl_args.append(pemfile);
    }

    curl_args.append(buildURL(_metrics->value("CCServer"), _metrics->value("CCPort"), true));

    if(_metrics->boolean("CCUseProxyServer"))
    {
      curl_args.append( "-x" );
      curl_args.append(_metrics->value("CCProxyServer") +
		       QString(_metrics->value("CCProxyPort").toInt() == 0 ? "" :
					  (":" + _metrics->value("CCProxyPort"))));
      curl_args.append( "-U" );
      curl_args.append(_metricsenc->value("CCProxyLogin") + ":" +
		       _metricsenc->value("CCPassword"));
    }

    if(!_extraHeaders.isEmpty())
    {
      for(int i = 0; i < _extraHeaders.size(); ++i)
      {
        curl_args.append("-H");
        curl_args.append(_extraHeaders.at(i).first + ": " + _extraHeaders.at(i).second);
      }
    }

    QString curlCmd = curl_path + ((DEBUG) ? (" " + curl_args.join(" ")) : "");
    if (DEBUG)
      qDebug("%s", curlCmd.toAscii().data());

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    /* TODO: consider changing to the original implementation:
	        start the proc in a separate thread
	        capture the output as it's generated
	        while (proc->isRunning())
		  qApp->processEvents();
     */
    proc.start(curl_path, curl_args);
    if ( !proc.waitForStarted() )
    {
      QApplication::restoreOverrideCursor();
      _errorMsg = errorMsg(-18)
		    .arg(curlCmd)
		    .arg("")
		    .arg(QString(proc.readAllStandardError()));
      return -18;
    }

    if (! proc.waitForFinished())
    {
      QApplication::restoreOverrideCursor();
      _errorMsg = errorMsg(-18)
		    .arg(curlCmd)
		    .arg("")
		    .arg(QString(proc.readAllStandardError()));
      return -18;
    }

    QApplication::restoreOverrideCursor();

    if (proc.exitStatus() != QProcess::NormalExit)
    {
      _errorMsg = errorMsg(-18)
		    .arg(curlCmd)
		    .arg("")
		    .arg(QString(proc.readAllStandardError()));
      return -18;
    }

    if (proc.exitCode() != 0)
    {
      _errorMsg = errorMsg(-18)
		    .arg(curlCmd)
		    .arg(proc.exitCode())
		    .arg(QString(proc.readAllStandardError()));
      return -18;
    }

    presponse = proc.readAllStandardOutput();
  }

  if (isTest())
    _metrics->set("CCTestMe", presponse);

  return 0;
}


/** \brief Insert into or update the ccpay table based on parameters extracted
           from the credit card processing service' response to a transaction
           request.

    \todo document the parameter names and expected values to help subclassers

 */
int CreditCardProcessor::updateCCPay(int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("updateCCPay(%d, %d params)", pccpayid, pparams.size());

  QString sql;
  XSqlQuery ccq;

  bool valid;
  QVariant param;
  QString r_error;
  param = pparams.value("r_error", &valid);
  if (valid)
    r_error = param.toString();

  if (pccpayid > 0)
  {
    param = pparams.value("status", &valid);
    QString r_status;
    if (valid)
      r_status = param.toString();
    if (r_status == "X" || r_status == "D")
    {
      // ccpay_r_ordernum because it's used to build postauth/capture messages
      ccq.prepare("INSERT INTO ccpay ("
                  "    ccpay_ccard_id, ccpay_cust_id, ccpay_amount,"
                  "    ccpay_status, ccpay_type, ccpay_auth_charge,"
                  "    ccpay_order_number, ccpay_order_number_seq,"
                  "    ccpay_by_username, ccpay_curr_id, ccpay_r_ordernum "
                  ") SELECT "
                  "    ccpay_ccard_id, ccpay_cust_id, ccpay_amount,"
                  "    ccpay_status, ccpay_type, ccpay_auth_charge,"
                  "    ccpay_order_number, ccpay_order_number_seq + 1,"
                  "    ccpay_by_username, ccpay_curr_id, ccpay_r_ordernum "
                  "  FROM ccpay"
                  "  WHERE ((ccpay_id=:id)"
                  "     AND (ccpay_type='A'));");
      ccq.bindValue(":id", pccpayid);
      ccq.exec();
    }

    sql =  "UPDATE ccpay SET"
	   "<? if exists(\"fromcurr\") ?>"
	   "      ccpay_amount=ROUND(currToCurr(<? value(\"fromcurr\") ?>,"
	   "                                    <? value(\"tocurr\") ?>,"
	   "                                    <? value(\"amount\") ?>,"
	   "                                    CURRENT_DATE), 2),"
	   "       ccpay_curr_id=<? value(\"currid\") ?>,"
	   "<? else ?>"
	   "       ccpay_amount=ROUND(<? value(\"amount\") ?>, 2),"
	   "       ccpay_curr_id=<? value(\"currid\") ?>,"
	   "<? endif ?>"
	   "       ccpay_auth=<? value(\"auth\") ?>,"
	   "       ccpay_r_approved=<? value(\"approved\") ?>,"
	   "       ccpay_r_avs=<? value(\"avs\") ?>,"
	   "       ccpay_r_code=<? value(\"code\") ?>,"
	   "       ccpay_r_error=<? value(\"error\") ?>,"
	   "       ccpay_r_message=<? value(\"message\") ?>,"
	   "       ccpay_r_ordernum=<? value(\"ordernum\") ?>,"
	   "       ccpay_r_ref=<? value(\"ref\") ?>,"
	   "<? if exists(\"shipping\") ?>"
	   "       ccpay_r_shipping=<? value(\"shipping\") ?>,"
	   "<? endif ?>"
	   "<? if exists(\"score\") ?>"
	   "       ccpay_yp_r_score=<? value(\"score\") ?>,"
	   "<? endif ?>"
	   "<? if exists(\"tax\") ?>"
	   "       ccpay_r_tax=<? value(\"tax\") ?>,"
	   "<? endif ?>"
	   "<? if exists(\"tdate\") ?>"
	   "       ccpay_yp_r_tdate=<? value(\"tdate\") ?>,"
	   "<? endif ?>"
	   "<? if exists(\"time\") ?>"
	   "       ccpay_yp_r_time=<? value(\"time\")?>,"
	   "<? endif ?>"
	   "       ccpay_status=<? value(\"status\") ?>"
	   " WHERE (ccpay_id=<? value(\"ccpay_id\") ?>);" ;
  }
  else
  {
    ccq.exec("SELECT NEXTVAL('ccpay_ccpay_id_seq') AS ccpay_id;");
    if (ccq.first())
      pccpayid = ccq.value("ccpay_id").toInt();
    else if (ccq.lastError().type() != QSqlError::NoError && r_error.isEmpty())
    {
      _errorMsg = errorMsg(4).arg(ccq.lastError().databaseText());
      return 1;
    }
    else if (ccq.lastError().type() == QSqlError::NoError && r_error.isEmpty())
    {
      _errorMsg = errorMsg(4).arg(errorMsg(2));
      return 2;
    }
    else	// no rows found and YP reported an error
    {
      _errorMsg = errorMsg(-12).arg(r_error);
      return -12;
    }

    if (pparams.inList("ordernum") && pparams.value("ordernum").toInt() > 0)
    {
      QString maxs("SELECT MAX(COALESCE(ccpay_order_number_seq, -1)) + 1"
		   "       AS next_seq "
		   "  FROM ccpay "
		   " WHERE (ccpay_order_number=<? value(\"ordernum\") ?>);");
      MetaSQLQuery maxm(maxs);
      ccq = maxm.toQuery(pparams);
      if (ccq.first())
	pparams.append("next_seq", ccq.value("next_seq"));
      else
	pparams.append("next_seq", 0);
    }
    else
	pparams.append("next_seq", 0);

    sql =  "INSERT INTO ccpay ("
	   "    ccpay_id, ccpay_ccard_id, ccpay_cust_id,"
	   "    ccpay_type,"
	   "    ccpay_amount,"
	   "    ccpay_curr_id,"
	   "    ccpay_auth, ccpay_auth_charge,"
	   "    ccpay_order_number,"
	   "    ccpay_order_number_seq,"
	   "    ccpay_r_approved, ccpay_r_avs,"
	   "    ccpay_r_code,    ccpay_r_error,"
	   "    ccpay_r_message, ccpay_r_ordernum,"
	   "    ccpay_r_ref,"
	   "<? if exists(\"score\") ?>    ccpay_yp_r_score, <? endif ?>"
	   "<? if exists(\"shipping\") ?> ccpay_r_shipping, <? endif ?>"
	   "<? if exists(\"tax\") ?>      ccpay_r_tax,   <? endif ?>"
	   "<? if exists(\"tdate\") ?>    ccpay_yp_r_tdate, <? endif ?>"
	   "<? if exists(\"time\") ?>     ccpay_yp_r_time,  <? endif ?>"
	   "    ccpay_status"
	   ") SELECT <? value(\"ccpay_id\") ?>, ccard_id, cust_id,"
	   "    <? value(\"type\") ?>,"
	   "<? if exists(\"fromcurr\") ?>"
	   "    ROUND(currToCurr(<? value(\"fromcurr\") ?>,"
	   "                     <? value(\"tocurr\") ?>,"
	   "                     <? value(\"amount\") ?>,CURRENT_DATE), 2),"
	   "    <? value(\"tocurr\") ?>,"
	   "<? else ?>"
	   "    ROUND(<? value(\"amount\") ?>, 2),"
	   "    <? value(\"currid\") ?>,"
	   "<? endif ?>"
	   "    <? value(\"auth\") ?>,     <? value(\"auth_charge\") ?>,"
	   "    <? value(\"ordernum\") ?>,"
	   "    COALESCE(<? value(\"next_seq\") ?>, 1),"
	   "    <? value(\"approved\") ?>, <? value(\"avs\") ?>,"
	   "    <? value(\"code\") ?>,     <? value(\"error\") ?>,"
	   "    <? value(\"message\") ?>,  <? value(\"xactionid\") ?>,"
	   "    <? value(\"ref\") ?>,"
	   "<? if exists(\"score\") ?>    <? value(\"score\") ?>,   <? endif ?>"
	   "<? if exists(\"shipping\") ?> <? value(\"shipping\") ?>,<? endif ?>"
	   "<? if exists(\"tax\") ?>      <? value(\"tax\") ?>,     <? endif ?>"
	   "<? if exists(\"tdate\") ?>    <? value(\"tdate\") ?>,   <? endif ?>"
	   "<? if exists(\"time\") ?>     <? value(\"time\") ?>,    <? endif ?>"
	   "    <? value(\"status\") ?>"
	   "  FROM ccard, custinfo"
	   "  WHERE ((ccard_cust_id=cust_id)"
	   "    AND  (ccard_id=<? value(\"ccard_id\") ?>));";
  }

  pparams.append("ccpay_id", pccpayid);

  MetaSQLQuery mql(sql);
  ccq = mql.toQuery(pparams);

  if (ccq.lastError().type() != QSqlError::NoError)
  {
    pccpayid = -1;
    _errorMsg = errorMsg(4).arg(ccq.lastError().databaseText());
    return 1;
  }

  return 0;
}

/** \brief Return the default port expected by the subclass.

    This can differ depending on whether running in live or test mode.
    It is the subclass' responsibility to set both default ports.
 */
int CreditCardProcessor::defaultPort(bool ptestmode)
{
  if (ptestmode)
    return _defaultTestPort;
  else
    return _defaultLivePort;
}

/** \brief Return the default server expected by the subclass.

    This can differ depending on whether running in live or test mode.
    It is the subclass' responsibility to set both default servers.
 */
QString CreditCardProcessor::defaultServer()
{
  if (isTest())
    return _defaultTestServer;
  else if (isLive())
    return _defaultLiveServer;
  else
    return "";
}

/** \brief Reset error handling internal settings so previous transactions
           don't interfere with new transactions.
    \internal
 */
void CreditCardProcessor::reset()
{
  _errorMsg = "";
  _passedAvs = true;
  _passedCvv = true;
}

/** \brief Handle fraud checking as determined by the system configuration.

    \internal
 */
int CreditCardProcessor::fraudChecks()
{
  if (DEBUG)
    qDebug("CCP:fraudChecks()");

  int returnValue = 0;

  if (! _passedAvs && _metrics->value("CCAvsCheck") == "F")
  {
    _errorMsg = errorMsg(-97) + "\n" + _errorMsg;
    returnValue = -97;
  }
  else if (! _passedAvs && _metrics->value("CCAvsCheck") == "W")
  {
    _errorMsg = errorMsg(97) + "\n" + _errorMsg;
    returnValue = 97;
  }

  // not "else if" - maybe this next check will be fatal
  if (! _passedCvv && _metrics->value("CCCVVCheck") == "F")
  {
    _errorMsg = errorMsg(-96) + "\n" + _errorMsg;
    returnValue = -96;
  }
  else if (! _passedCvv && _metrics->value("CCCVVCheck") == "W")
  {
    _errorMsg = errorMsg(96) + "\n" + _errorMsg;
    returnValue = 96;
  }

  if (DEBUG)
    qDebug("CCP:fraudChecks() returning %d with _errorMsg %s",
	   returnValue, _errorMsg.toAscii().data());

  return returnValue;
}

/** \brief Print the CCReceipt report for a credit card transaction.

    \param pccpayid The internal id of the transaction for which to print the
                    receipt
 */
int CreditCardProcessor::printReceipt(const int pccpayid)
{
  if (DEBUG)
    qDebug("CCP:printReceipt()");

  int returnValue = 0;

  ParameterList params;
  params.append("ccpay_id",  pccpayid);
  params.append("preauth",   tr("Preauthorization"));
  params.append("charge",    tr("Charge"));
  params.append("refund",    tr("Refund"));
  params.append("authorized",tr("Authorized"));
  params.append("approved",  tr("Approved"));
  params.append("declined",  tr("Declined"));
  params.append("reversed",  tr("Reversed"));
  params.append("voided",    tr("Voided"));
  params.append("noapproval",tr("No Approval Code"));
  params.append("key",       omfgThis->_key);

  orReport report("CCReceipt", params);

  if (report.isValid())
    report.print();
  else
  {
    report.reportError(0);
    _errorMsg = errorMsg(94);
    returnValue = 94;
  }

  if (DEBUG)
    qDebug("CCP:printReceipt() returning %d with _errorMsg %s",
	   returnValue, _errorMsg.toAscii().data());

  return returnValue;
}

QString CreditCardProcessor::typeToCode(CCTransaction ptranstype)
{
  // TODO: do we need to distinguish between Capture and Charge? Reverse and Void
  switch (ptranstype)
  {
    case Authorize: return QString("A");
    case Capture:   return QString("C");
    case Charge:    return QString("C");
    case Credit:    return QString("R");
    case Reverse:   return QString("V");
    case Void:      return QString("V");
  }

  return QString::null;
}

/** \brief Convert between two %currencies.

    This is slightly different than the version in the CurrDisplay
    widget because credit card processing has special error reporting
    needs.

    \param pfrom   The source %currency
    \param pto     The destination %currency
    \param pamount The value of the transaction in the source %currency
    \param[out] perror The CreditCardProcessor error code for a failed conversion

    \return The value of the transaction in the destination %currency
 */
double CreditCardProcessor::currToCurr(const int pfrom, const int pto, const double pamount, int *perror)
{
  if (pfrom == pto)
    return pamount;

  XSqlQuery ccq;
  ccq.prepare("SELECT ROUND(currToCurr(:fromid, :toid,"
	      "                :amount, CURRENT_DATE), 2) AS result;");
  ccq.bindValue(":fromid", pfrom);
  ccq.bindValue(":toid",   pto);
  ccq.bindValue(":amount", pamount);
  ccq.exec();

  if (ccq.first())
    return ccq.value("result").toDouble();
  else if (ccq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ccq.lastError().databaseText();
    if (perror)
      *perror = -1;
  }

  return 0;
}

/** \brief Returns whether the subclass handles checks. */
bool CreditCardProcessor::handlesChecks()
{
  return false;
}

/** \brief Returns whether the subclass handles credit cards. */
bool CreditCardProcessor::handlesCreditCards()
{
  return false;
}

/** \brief Construct a valid URL from the information in the configuration.
  
    Handle the case where someone leaves off a piece of the URL when
    entering the basic configuration. Note that a lot of people are used
    to typing in web browsers, which fill in some parts for them.

    If the %user did not enter values for the server or the port on the
    configuration, use the defaults provided by the service' subclass.

    \param pserver   Use this server, or the defaultServer if blank
    \param pport     Use this port, or the defaultPort if blank
    \param pinclport Flag whether the port should be part of the constructed
                     URL or not

    \see defaultServer
    \see defaultPort
*/
QString CreditCardProcessor::buildURL(const QString pserver, const QString pport, const bool pinclport)
{
  if (DEBUG)
    qDebug("buildURL(%s, %s, %d)", pserver.toAscii().data(),
           pport.toAscii().data(), pinclport);

  QString defaultprotocol = "https";

  QString serverStr = pserver.isEmpty() ?  defaultServer() : pserver;
  QString protocol  = serverStr;
  protocol.remove(QRegExp("://.*"));
  if (protocol == serverStr)
    protocol = "";
  if (DEBUG) qDebug("protocol: %s", protocol.toAscii().data());
  if (protocol.isEmpty())
    protocol = defaultprotocol;

  QString host      = serverStr;
  host.remove(QRegExp("^.*://")).remove(QRegExp("[/:].*"));
  if (DEBUG) qDebug("host: %s", host.toAscii().data());

  QString port      = serverStr;
  port.remove(QRegExp("^([^:/]+://)?[^:]*:")).remove(QRegExp("/.*"));
  if (port == serverStr || port == host)
    port = "";
  if (DEBUG) qDebug("port: %s", port.toAscii().data());
  if (! pinclport)
    port = "";
  else if (port.isEmpty())
    port = pport.toInt() == 0 ?  QString::number(defaultPort()) : pport;

  QString remainder = serverStr;
  remainder.remove(QRegExp("^([^:/]+://)?[^:/]*(:[0-9]+)?/"));
  if (remainder == serverStr)
    remainder = "";
  if (DEBUG) qDebug("remainder: %s", remainder.toAscii().data());

  serverStr = protocol + "://" + host;

  if (! port.isEmpty() && ! (protocol == "https" && port == "443"))
    serverStr += ":" + port;

  serverStr += "/" + remainder;

  if (DEBUG) qDebug("buildURL: returning %s", serverStr.toAscii().data());
  return serverStr;
}

/** \brief Processes pre-authorization transactions.

    This version of authorize is intended for use by %scripts.
    Instead of passing all of the arguments in order, this method
    allows creating a QtScript object and setting properties
    on this object by name. Then the script can pass this object
    to 
    authorize(const int pccardid, const int pcvv, const double pamount, double ptax, bool ptaxexempt, double pfreight, double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int &prefid):
    \code
      var params = new Object;
      params.ccard_id = _ccardid;
      params.cvv      = _cvv.text;
      ...
      var processor = toolbox.getProcessor();
      var result = processor.authorize(params);

      if (result.returnVal < 0)
        // handle errors
      else if (result.returnVal > 0)
        // handle warnings
      ...
    \endcode
    
    \param pinput The parameter list to unpack and use to call authorize
    \return A parameter list containing the output parameters, plus one
            called returnVal holding the return value of authorize
 */
// TODO: memory leak of poutput!
ParameterList CreditCardProcessor::authorize(const ParameterList &pinput)
{
  if (DEBUG) qDebug("authorize(ParameterList&) entered");

  ParameterList *poutput = new ParameterList;
  QVariant       param;
  bool           valid;
  QString        context = "authorize";

  int    ccard_id = -1;
  int    cvv      =  0;
  double amount   =  0.0;
  double tax      =  0.0;
  bool   taxExempt=  true;
  double freight  =  0.0;
  double duty     =  0.0;
  int    curr_id  = -1;
  int    ccpay_id = -1;
  int    ref_id   = -1;
  QString neworder;
  QString reforder;
  QString reftype;

  param = pinput.value("ccard_id", &valid);
  if (valid)
  {
    ccard_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("ccard_id").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("cvv", &valid);
  if (valid)
  {
    cvv = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("cvv").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("amount", &valid);
  if (valid)
  {
    amount = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("amount").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("tax", &valid);
  if (valid)
  {
    tax = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("tax").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("taxExempt", &valid);
  if (valid)
    taxExempt = param.toBool();

  param = pinput.value("freight", &valid);
  if (valid)
  {
    freight = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("freight").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("duty", &valid);
  if (valid)
  {
    duty = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("duty").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("curr_id", &valid);
  if (valid)
  {
    curr_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("curr_id").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("neworder", &valid);
  if (valid)
    neworder = param.toString();

  param = pinput.value("reforder", &valid);
  if (valid)
    reforder = param.toString();

  param = pinput.value("reftype", &valid);
  if (valid)
    reftype = param.toString();

  param = pinput.value("refid", &valid);
  if (valid)
  {
    ref_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("refid").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  int returnVal = getProcessor()->authorize(ccard_id, cvv, amount, tax,
                                            taxExempt, freight, duty, curr_id,
                                            neworder, reforder, ccpay_id,
                                            reftype, ref_id);

  poutput->append("returnVal", returnVal);
  poutput->append("neworder",  neworder);
  poutput->append("reforder",  reforder);
  poutput->append("ccpayid",   ccpay_id);
  poutput->append("refid",     ref_id);

  return *poutput;
}

/** \brief Processes charge transactions.

    This version of charge is intended for use by %scripts.
    Instead of passing all of the arguments in order, this method
    allows creating a QtScript object and setting properties
    on this object by name. Then the script can pass this object
    to charge.
    
    \param pinput The parameter list to unpack and use to call charge
    \return A parameter list containing the output parameters, plus one
            called returnVal holding the return value of charge

    \see authorize(const ParameterList &pinput)
 */
// TODO: memory leak of poutput!
ParameterList CreditCardProcessor::charge(const ParameterList &pinput)
{
  if (DEBUG) qDebug("charge(ParameterList&) entered");

  ParameterList *poutput = new ParameterList;
  QVariant       param;
  bool           valid;
  QString        context = "charge";

  int    ccard_id = -1;
  int    cvv      =  0;
  double amount   =  0.0;
  double tax      =  0.0;
  bool   taxExempt=  true;
  double freight  =  0.0;
  double duty     =  0.0;
  int    curr_id  = -1;
  int    ccpay_id = -1;
  int    ref_id   = -1;
  QString neworder;
  QString reforder;
  QString reftype;

  param = pinput.value("ccard_id", &valid);
  if (valid)
  {
    ccard_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("ccard_id").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("cvv", &valid);
  if (valid)
  {
    cvv = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("cvv").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("amount", &valid);
  if (valid)
  {
    amount = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("amount").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("tax", &valid);
  if (valid)
  {
    tax = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("tax").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("taxExempt", &valid);
  if (valid)
    taxExempt = param.toBool();

  param = pinput.value("freight", &valid);
  if (valid)
  {
    freight = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("freight").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("duty", &valid);
  if (valid)
  {
    duty = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("duty").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("curr_id", &valid);
  if (valid)
  {
    curr_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("curr_id").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("neworder", &valid);
  if (valid)
    neworder = param.toString();

  param = pinput.value("reforder", &valid);
  if (valid)
    reforder = param.toString();

  param = pinput.value("reftype", &valid);
  if (valid)
    reftype = param.toString();

  param = pinput.value("refid", &valid);
  if (valid)
  {
    ref_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("refid").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  int returnVal = getProcessor()->charge(ccard_id, cvv, amount, tax,
                                            taxExempt, freight, duty, curr_id,
                                            neworder, reforder, ccpay_id,
                                            reftype, ref_id);

  poutput->append("returnVal", returnVal);
  poutput->append("neworder",  neworder);
  poutput->append("reforder",  reforder);
  poutput->append("ccpayid",   ccpay_id);
  poutput->append("refid",     ref_id);

  return *poutput;
}

/** \brief Captures preauthorized transactions.

    This version of chargePreauthorized is intended for use by %scripts.
    Instead of passing all of the arguments in order, this method
    allows creating a QtScript object and setting properties
    on this object by name. Then the script can pass this object
    to chargePreauthorized.
    
    \param pinput The parameter list to unpack and use to call chargePreauthorized
    \return A parameter list containing the output parameters, plus one
            called returnVal holding the return value of chargePreauthorized

    \see authorize(const ParameterList &pinput)
 */
// TODO: memory leak of poutput!
ParameterList CreditCardProcessor::chargePreauthorized(const ParameterList &pinput)
{
  if (DEBUG) qDebug("chargePreauthorized(ParameterList&) entered");

  ParameterList *poutput = new ParameterList;
  QVariant       param;
  bool           valid;
  QString        context = "chargePreauthorized";

  int    cvv      =  0;
  double amount   =  0.0;
  int    curr_id  = -1;
  int    ccpay_id = -1;
  QString neworder;
  QString reforder;

  param = pinput.value("cvv", &valid);
  if (valid)
  {
    cvv = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("cvv").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("amount", &valid);
  if (valid)
  {
    amount = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("amount").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("curr_id", &valid);
  if (valid)
  {
    curr_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("curr_id").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("neworder", &valid);
  if (valid)
    neworder = param.toString();

  param = pinput.value("reforder", &valid);
  if (valid)
    reforder = param.toString();

  param = pinput.value("ccpay_id", &valid);
  if (valid)
  {
    ccpay_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("ccpay_id").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  int returnVal = getProcessor()->chargePreauthorized(cvv, amount, curr_id,
                                                      neworder, reforder,
                                                      ccpay_id);

  poutput->append("returnVal", returnVal);
  poutput->append("neworder",  neworder);
  poutput->append("reforder",  reforder);
  poutput->append("ccpayid",   ccpay_id);

  return *poutput;
}

/** \brief Processes credit transactions.

    This version of credit is intended for use by %scripts.
    Instead of passing all of the arguments in order, this method
    allows creating a QtScript object and setting properties
    on this object by name. Then the script can pass this object
    to credit.
    
    \param pinput The parameter list to unpack and use to call credit
    \return A parameter list containing the output parameters, plus one
            called returnVal holding the return value of credit

    \see authorize(const ParameterList &pinput)
 */
// TODO: memory leak of poutput!
ParameterList CreditCardProcessor::credit(const ParameterList &pinput)
{
  if (DEBUG) qDebug("credit(ParameterList&) entered");

  ParameterList *poutput = new ParameterList;
  QVariant       param;
  bool           valid;
  QString        context = "credit";

  int    ccard_id = -1;
  int    cvv      =  0;
  double amount   =  0.0;
  double tax      =  0.0;
  bool   taxExempt=  true;
  double freight  =  0.0;
  double duty     =  0.0;
  int    curr_id  = -1;
  int    ccpay_id = -1;
  int    ref_id   = -1;
  QString neworder;
  QString reforder;
  QString reftype;

  param = pinput.value("ccard_id", &valid);
  if (valid)
  {
    ccard_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("ccard_id").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("cvv", &valid);
  if (valid)
  {
    cvv = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("cvv").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("amount", &valid);
  if (valid)
  {
    amount = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("amount").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("tax", &valid);
  if (valid)
  {
    tax = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("tax").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("taxExempt", &valid);
  if (valid)
    taxExempt = param.toBool();

  param = pinput.value("freight", &valid);
  if (valid)
  {
    freight = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("freight").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("duty", &valid);
  if (valid)
  {
    duty = param.toDouble(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("duty").arg("double");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("curr_id", &valid);
  if (valid)
  {
    curr_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("curr_id").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("neworder", &valid);
  if (valid)
    neworder = param.toString();

  param = pinput.value("reforder", &valid);
  if (valid)
    reforder = param.toString();

  param = pinput.value("ccpay_id", &valid);
  if (valid)
  {
    ccpay_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("ccpay_id").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  param = pinput.value("reftype", &valid);
  if (valid)
    reftype = param.toString();

  param = pinput.value("refid", &valid);
  if (valid)
  {
    ref_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("refid").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  int returnVal = getProcessor()->credit(ccard_id, cvv, amount, tax,
                                         taxExempt, freight, duty, curr_id,
                                         neworder, reforder, ccpay_id,
                                         reftype, ref_id);

  poutput->append("returnVal", returnVal);
  poutput->append("neworder",  neworder);
  poutput->append("reforder",  reforder);
  poutput->append("ccpayid",   ccpay_id);
  poutput->append("refid",     ref_id);

  return *poutput;
}

/** \brief Processes void transactions.

    This version of voidPrevious is intended for use by %scripts.
    Instead of passing all of the arguments in order, this method
    allows creating a QtScript object and setting properties
    on this object by name. Then the script can pass this object
    to voidPrevious.
    
    \param pinput The parameter list to unpack and use to call voidPrevious
    \return A parameter list containing the output parameters, plus one
            called returnVal holding the return value of voidPrevious

    \see authorize(const ParameterList &pinput)
 */
// TODO: memory leak of poutput!
ParameterList CreditCardProcessor::voidPrevious(const ParameterList &pinput)
{
  if (DEBUG) qDebug("voidPrevious(ParameterList&) entered");

  ParameterList *poutput = new ParameterList;
  QVariant       param;
  bool           valid;
  QString        context = "voidPrevious";

  int    ccpay_id = -1;

  param = pinput.value("ccpay_id", &valid);
  if (valid)
  {
    ccpay_id = param.toInt(&valid);
    if (!valid)
    {
      _errorMsg = errorMsg(-81).arg(context).arg("ccpay_id").arg("int");
      poutput->append("returnVal", -81);
      return *poutput;
    }
  }

  int returnVal = getProcessor()->voidPrevious(ccpay_id);

  poutput->append("returnVal", returnVal);
  poutput->append("ccpayid",   ccpay_id);

  return *poutput;
}

CreditCardProcessor::FraudCheckResult *CreditCardProcessor::avsCodeLookup(QChar pcode)
{
  for (int i = 0; i < _avsCodes.length(); i++)
    if (_avsCodes.at(i)->code == pcode)
      return _avsCodes.at(i);

  return 0;
}

CreditCardProcessor::FraudCheckResult *CreditCardProcessor::cvvCodeLookup(QChar pcode)
{
  for (int i = 0; i < _cvvCodes.length(); i++)
    if (_cvvCodes.at(i)->code == pcode)
      return _cvvCodes.at(i);

  return 0;
}

void CreditCardProcessor::sslErrors(const QList<QSslError> &errors)
{
  if (DEBUG)
    qDebug() << "CreditCardProcessor::sslErrors(" << errors << ")";

  QHttp *httpobj = qobject_cast<QHttp*>(sender());
  if (errors.size() > 0 && httpobj)
  {
    QString errlist;
    for (int i = 0; i < errors.size(); i++)
      errlist += QString("<li>%1</li>").arg(errors.at(i).errorString());
    if (_ignoreSslErrors ||
        QMessageBox::question(0,
                              tr("Questionable Security"),
                              tr("<p>The security of this transaction may be compromised."
                                 " The following SSL errors have been reported:"
                                 "<ul>%1</ul></p>"
                                 "<p>Would you like to continue anyway?</p>")
                              .arg(errlist),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
        httpobj->ignoreSslErrors();
  }
}
