<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:str="http://exslt.org/strings"
                extension-element-prefixes="str"
>
<!-- when using xsltproc:
                xmlns:str="http://exslt.org/strings"
     when using an xslt processor that understands xpath 2.0
                xmlns:str="http://www.w3.org/2005/xpath-functions"
-->
  <xsl:import href="utility.xsl"/>
  <xsl:output indent="yes" method="xml" doctype-system="xtupleapi.dtd" />

  <!-- parameters supply default values for mandatory fields that we can't
       derive from the data. param name = (view-name)_(column-name)
    -->
  <xsl:param name="customer_default_tax_authority" select="'Yahoo'"/>
  <xsl:param name="customer_corresp_type" select="'bill'" />   <!-- bill|ship -->
  <xsl:param name="salesline_sched_offset"	   select="'1 day'" />

  <xsl:template match="/">
    <xsl:element name="xtupleimport">
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>

  <xsl:template name="YahooCountry">
    <xsl:param name="country"/>
    SELECT country_name FROM country WHERE (country_abbr=
    <xsl:choose>
      <xsl:when test="substring($country,1,2) = 'GB'">'UK'</xsl:when>
      <xsl:otherwise>'<xsl:value-of select="substring($country,1,2)"/>'</xsl:otherwise>
    </xsl:choose>)
  </xsl:template>

  <xsl:template name="customerNumberFromAddress">
    <xsl:param name="address"/>
    <xsl:param name="generate" select="'false'"/>
    getCustNumberFromInfo('<xsl:value-of select="str:replace($address/Email, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			  '<xsl:value-of select="str:replace($address/Company, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			  '<xsl:value-of select="str:replace($address/Name/First, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			  '<xsl:value-of select="str:replace($address/Name/Last, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			  '<xsl:value-of select="str:replace($address/Name/Full, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			  '<xsl:value-of select="$generate"/>')
  </xsl:template>

  <xsl:template name="customerNameFromAddress">
    <xsl:param name="address"/>
    <xsl:param name="generate" select="'false'"/>
    getCustNameFromInfo('<xsl:value-of select="str:replace($address/Email, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			'<xsl:value-of select="str:replace($address/Company, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			'<xsl:value-of select="str:replace($address/Name/First, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			'<xsl:value-of select="str:replace($address/Name/Last, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			'<xsl:value-of select="str:replace($address/Name/Full, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			'<xsl:value-of select="$generate"/>')
  </xsl:template>

  <xsl:template name="shiptoNumberFromAddress">
    <xsl:param name="address"/>
    <xsl:param name="billaddr"/>
    <xsl:param name="generate"	select="'false'"/>
    <xsl:param name="create"	select="'false'"/>
    getShiptoNumberFromInfo(<xsl:if test="$billaddr">
			      <xsl:call-template name="customerNameFromAddress">
				<xsl:with-param name="address" select="$billaddr"/>
			      </xsl:call-template>
			    </xsl:if>,
			    '<xsl:value-of select="str:replace($address/Email, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			    '<xsl:value-of select="str:replace($address/Company, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			    '<xsl:value-of select="str:replace($address/Name/First, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			    '<xsl:value-of select="str:replace($address/Name/Last, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			    '<xsl:value-of select="str:replace($address/Name/Full, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			    '<xsl:value-of select="str:replace($address/Address1, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			    '<xsl:value-of select="str:replace($address/Address2, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			    NULL,
			    '<xsl:value-of select="str:replace($address/City, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			    '<xsl:value-of select="str:replace($address/State, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			    '<xsl:value-of select="str:replace($address/Zip, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			    '<xsl:value-of select="str:replace($address/Country, &quot;&apos;&quot;, &quot;&apos;&apos;&quot;)"/>',
			     <xsl:value-of select="$generate"/>,
			     <xsl:value-of select="$create"/>)
  </xsl:template>

  <xsl:template match="OrderList">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="Order">
    <!-- try to make a new customer record but ignore errors 'cause it might
         already exist
     -->
    <customer ignore="true">
      <customer_number quote="false">
	<xsl:choose>
	  <xsl:when test="AddressInfo[@type = 'bill']">
	    <xsl:call-template name="customerNumberFromAddress">
	      <xsl:with-param name="address"
			      select="AddressInfo[@type = 'bill']"/>
	      <xsl:with-param name="generate" select="'true'"/>
	    </xsl:call-template>
	  </xsl:when>
	  <xsl:when test="AddressInfo[@type = 'ship']">
	    <xsl:call-template name="customerNumberFromAddress">
	      <xsl:with-param name="address"
			      select="AddressInfo[@type = 'ship']"/>
	      <xsl:with-param name="generate" select="'true'"/>
	    </xsl:call-template>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:message terminate="yes">
	      Cannot find Customer Number for Order
	      <xsl:value-of select="number"/>
	    </xsl:message>
	  </xsl:otherwise>
	</xsl:choose>
      </customer_number>

      <!-- customer_type -->

      <customer_name quote="false">
	<xsl:choose>
	  <xsl:when test="AddressInfo[@type = 'bill']">
	    <xsl:call-template name="customerNameFromAddress">
	      <xsl:with-param name="address"
			      select="AddressInfo[@type = 'bill']"/>
	      <xsl:with-param name="generate" select="'true'"/>
	    </xsl:call-template>
	  </xsl:when>
	  <xsl:when test="AddressInfo[@type = 'ship']">
	    <xsl:call-template name="customerNameFromAddress">
	      <xsl:with-param name="address"
			      select="AddressInfo[@type = 'ship']"/>
	      <xsl:with-param name="generate" select="'true'"/>
	    </xsl:call-template>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:message terminate="yes">
	      Cannot find Customer Name for Order
	      <xsl:value-of select="number"/>
	    </xsl:message>
	  </xsl:otherwise>
	</xsl:choose>
      </customer_name>

      <!-- active -->
      <!-- sales_rep -->

      <!-- commission -->

      <xsl:if test="Shipping">
	<ship_via>
	  <xsl:value-of select="Shipping"/>
	</ship_via>
      </xsl:if>

      <!-- ship_form -->
      <!-- shipping_charges -->
      <!-- accepts_backorders -->
      <!-- accepts_partial_shipments -->
      <!-- allow_free_form_shipto -->
      <!-- allow_free_form_billto -->
      <!-- preferred_selling_whs -->
      <!--<default_tax_authority><xsl:value-of select="$customer_default_tax_authority"/></default_tax_authority> -->
      <!-- default_terms -->
      <!-- balance_method -->
      <!-- default_discount -->

      <xsl:if test="@currency">
	<default_currency><xsl:value-of select="@currency"/></default_currency>
      </xsl:if>

      <!-- credit_limit_currency -->
      <!-- credit_limit -->
      <!-- credit_rating -->
      <!-- credit_status -->
      <!-- credit_status_exceed_warn -->
      <!-- credit_status_exceed_hold -->
      <!-- uses_purchase_orders -->
      <!-- uses_blanket_pos -->

      <xsl:if test="AddressInfo[@type = 'bill']">
	<!-- billing_contact_id -->
	<!-- billing_contact_honorific -->
	<xsl:if test="AddressInfo[@type = 'bill']/Name/First">
	  <billing_contact_first>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Name/First"/>
	  </billing_contact_first>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/Name/Last">
	  <billing_contact_last>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Name/Last"/>
	  </billing_contact_last>
	</xsl:if>
	<!-- billing_contact_job_title -->
	<xsl:if test="AddressInfo[@type = 'bill']/Phone">
	  <billing_contact_voice>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Phone"/>
	  </billing_contact_voice>
	</xsl:if>
	<!-- billing_contact_alternate -->
	<!-- billing_contact_fax -->
	<xsl:if test="AddressInfo[@type = 'bill']/Email">
	  <billing_contact_email>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Email"/>
	  </billing_contact_email>
	</xsl:if>
	<!-- billing_contact_web -->
	<!-- billing_contact_address_change -->
	<xsl:if test="AddressInfo[@type = 'bill']/Address1">
	  <billing_contact_address1>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Address1"/>
	  </billing_contact_address1>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/Address2">
	  <billing_contact_address2>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Address2"/>
	  </billing_contact_address2>
	</xsl:if>
	<!-- billing_contact_address3 -->
	<xsl:if test="AddressInfo[@type = 'bill']/City">
	  <billing_contact_city>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/City"/>
	  </billing_contact_city>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/State">
	  <billing_contact_state>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/State"/>
	  </billing_contact_state>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/Zip">
	  <billing_contact_postalcode>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Zip"/>
	  </billing_contact_postalcode>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/Country">
	  <billing_contact_country quote="false">
	    <xsl:call-template name="YahooCountry">
	      <xsl:with-param name="country"
			      select="AddressInfo[@type='bill']/Country"/>
	    </xsl:call-template>
	  </billing_contact_country>
	</xsl:if>
	<billing_contact_address_change value="CHANGEALL"/>
      </xsl:if>

      <xsl:if test="$customer_corresp_type != 'bill' and
		    $customer_corresp_type != 'ship'">
	<xsl:message terminate="yes">
	  The customer_corresp_type parameter has the value
	  '<xsl:value-of select="$customer_corresp_type"/>' but must be either
	  'bill' or 'ship'.
	</xsl:message>
      </xsl:if>
      <xsl:if test="AddressInfo[@type = $customer_corresp_type]">
	<!-- correspond_contact_id -->
	<!-- correspond_contact_honorific -->
	<xsl:if test="AddressInfo[@type = $customer_corresp_type]/Name/First">
	  <correspond_contact_first>
	    <xsl:value-of select="AddressInfo[@type = $customer_corresp_type]/Name/First"/>
	  </correspond_contact_first>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = $customer_corresp_type]/Name/Last">
	  <correspond_contact_last>
	    <xsl:value-of select="AddressInfo[@type = $customer_corresp_type]/Name/Last"/>
	  </correspond_contact_last>
	</xsl:if>
	<!-- correspond_contact_job_title -->
	<xsl:if test="AddressInfo[@type = $customer_corresp_type]/Phone">
	  <correspond_contact_voice>
	    <xsl:value-of select="AddressInfo[@type = $customer_corresp_type]/Phone"/>
	  </correspond_contact_voice>
	</xsl:if>
	<!-- correspond_contact_alternate -->
	<!-- correspond_contact_fax -->
	<xsl:if test="AddressInfo[@type = $customer_corresp_type]/Email">
	  <correspond_contact_email>
	    <xsl:value-of select="AddressInfo[@type = $customer_corresp_type]/Email"/>
	  </correspond_contact_email>
	</xsl:if>
	<!-- correspond_contact_web -->
	<!-- correspond_contact_address_change -->
	<xsl:if test="AddressInfo[@type = $customer_corresp_type]/Address1">
	  <correspond_contact_address1>
	    <xsl:value-of select="AddressInfo[@type = $customer_corresp_type]/Address1"/>
	  </correspond_contact_address1>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = $customer_corresp_type]/Address2">
	  <correspond_contact_address2>
	    <xsl:value-of select="AddressInfo[@type = $customer_corresp_type]/Address2"/>
	  </correspond_contact_address2>
	</xsl:if>
	<!-- correspond_contact_address3 -->
	<xsl:if test="AddressInfo[@type = $customer_corresp_type]/City">
	  <correspond_contact_city>
	    <xsl:value-of select="AddressInfo[@type = $customer_corresp_type]/City"/>
	  </correspond_contact_city>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = $customer_corresp_type]/State">
	  <correspond_contact_state>
	    <xsl:value-of select="AddressInfo[@type = $customer_corresp_type]/State"/>
	  </correspond_contact_state>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = $customer_corresp_type]/Zip">
	  <correspond_contact_postalcode>
	    <xsl:value-of select="AddressInfo[@type = $customer_corresp_type]/Zip"/>
	  </correspond_contact_postalcode>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = $customer_corresp_type]/Country">
	  <correspond_contact_country quote="false">
	    <xsl:call-template name="YahooCountry">
	      <xsl:with-param name="country"
			      select="AddressInfo[@type=$customer_corresp_type]/Country"/>
	    </xsl:call-template>
	  </correspond_contact_country>
	</xsl:if>
      </xsl:if>

      <notes>
	<xsl:if test="Comments">
	  <xsl:call-template name="cleanSQLChars">
	    <xsl:with-param name="inputStr" select="Comments"/>
	  </xsl:call-template>
	  <xsl:text>
</xsl:text>
	</xsl:if>
	<xsl:if test="CreditCard">
	  <xsl:value-of select="CreditCard"/>
	  type <xsl:value-of select="CreditCard/@type"/>
	  expiration <xsl:value-of select="CreditCard/@expiration"/>
	  <xsl:text>
</xsl:text>
	</xsl:if>
	<xsl:for-each select="AddressInfo/Custom">
	  <xsl:value-of select="concat(name(), '-', @name)"/>:
	  <xsl:call-template name="cleanSQLChars">
	    <xsl:with-param name="inputStr" select="text()"/>
	  </xsl:call-template>
	  <xsl:text>
</xsl:text>
	</xsl:for-each>
	<xsl:for-each select="NumericTime		|
			      Referer			|
			      Entry-Point		|
			      IPAddress			|
			      YahooLogin		|
			      Cookie			|
			      Trackable-Link		|
			      Warning			|
			      Suspect			|
			      Store-Status		|
			      HTTP-User-Agent" >
	  <xsl:sort select="name()"/>
	  <xsl:variable name="name" select="name()"/>
	  <xsl:value-of select="name()"/>:
	  <xsl:call-template name="cleanSQLChars">
	    <xsl:with-param name="inputStr" select="text()"/>
	  </xsl:call-template>
	  <xsl:for-each select="@*">
	    <xsl:value-of select="name()"/>=
	    <xsl:call-template name="cleanSQLChars">
	      <xsl:with-param name="inputStr" select="text()"/>
	    </xsl:call-template>
	  </xsl:for-each>
	  <xsl:text>
</xsl:text>
	</xsl:for-each>
      </notes>

      <!-- so_edi_profile -->
      <!-- so_edi_email -->
      <!-- so_edi_cc -->
      <!-- so_edi_subject -->
      <!-- so_edi_filename -->
      <!-- so_edi_emailbody -->
      <!-- invc_edi_profile -->
      <!-- invc_edi_email -->
      <!-- invc_edi_cc -->
      <!-- invc_edi_subject -->
      <!-- invc_edi_filename -->
      <!-- invc_edi_emailbody -->
    </customer>

    <xsl:if test="not(starts-with(@id, /OrderList/@StoreAccountName))">
      <xsl:message terminate="yes">
	Order number <xsl:value-of select="@id"/> is not valid since it
	does not start with the store account name
	<xsl:value-of select="/OrderList/@StoreAccountName"/><xsl:text>

</xsl:text>
      </xsl:message>
    </xsl:if>

    <salesorder>
      <order_number>
	<xsl:value-of select="substring-after(@id, concat(/OrderList/@StoreAccountName, '-'))"/>
      </order_number>

      <!-- warehouse -->

      <order_date>
	<xsl:value-of select="Time"/>
      </order_date>

      <!-- pack_date -->

      <originated_by value="Internet"/>

      <!-- sales_rep -->
      <!-- commission -->
      <!-- tax_authority -->
      <!-- terms -->
      <!-- project_number -->

      <customer_number quote="false">
	<xsl:choose>
	  <xsl:when test="AddressInfo[@type = 'bill']">
	    <xsl:call-template name="customerNumberFromAddress">
	      <xsl:with-param name="address"
			      select="AddressInfo[@type = 'bill']"/>
	    </xsl:call-template>
	  </xsl:when>
	  <xsl:when test="AddressInfo[@type = 'ship']">
	    <xsl:call-template name="customerNumberFromAddress">
	      <xsl:with-param name="address"
			      select="AddressInfo[@type = 'ship']"/>
	    </xsl:call-template>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:message terminate="yes">
	      Cannot find Customer Number for Order
	      <xsl:value-of select="number"/>
	    </xsl:message>
	  </xsl:otherwise>
	</xsl:choose>
      </customer_number>

      <xsl:if test="AddressInfo[@type = 'bill']">
	<xsl:if test="AddressInfo[@type = 'bill']/Name/Full">
	  <billto_name>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Name/Full"/>
	  </billto_name>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/Address1">
	  <billto_address1>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Address1"/>
	  </billto_address1>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/Address2">
	  <billto_address2>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Address2"/>
	  </billto_address2>
	</xsl:if>
	<!-- billto_address3 -->
	<xsl:if test="AddressInfo[@type = 'bill']/City">
	  <billto_city>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/City"/>
	  </billto_city>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/State">
	  <billto_state>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/State"/>
	  </billto_state>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/Zip">
	  <billto_postal_code>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Zip"/>
	  </billto_postal_code>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/Country">
	  <billto_country quote="false">
	    <xsl:call-template name="YahooCountry">
	      <xsl:with-param name="country"
			      select="AddressInfo[@type='bill']/Country"/>
	    </xsl:call-template>
	  </billto_country>
	</xsl:if>
      </xsl:if>

      <xsl:if test="AddressInfo[@type = 'ship']">
	<shipto_number quote="false">
	  <xsl:call-template name="shiptoNumberFromAddress">
	    <xsl:with-param name="address"
			    select="AddressInfo[@type = 'ship']"/>
	    <xsl:with-param name="billaddr"
			    select="AddressInfo[@type = 'bill']"/>
	    <xsl:with-param name="generate" select="'true'"/>
	    <xsl:with-param name="create"   select="'true'"/>
	  </xsl:call-template>
	</shipto_number>
	<xsl:if test="AddressInfo[@type = 'ship']/Name/Full">
	  <shipto_name>
	    <xsl:value-of select="AddressInfo[@type = 'ship']/Name/Full"/>
	  </shipto_name>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'ship']/Address1">
	  <shipto_address1>
	    <xsl:value-of select="AddressInfo[@type = 'ship']/Address1"/>
	  </shipto_address1>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'ship']/Address2">
	  <shipto_address2>
	    <xsl:value-of select="AddressInfo[@type = 'ship']/Address2"/>
	  </shipto_address2>
	</xsl:if>
	<!-- shipto_address3 -->
	<xsl:if test="AddressInfo[@type = 'ship']/City">
	  <shipto_city>
	    <xsl:value-of select="AddressInfo[@type = 'ship']/City"/>
	  </shipto_city>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'ship']/State">
	  <shipto_state>
	    <xsl:value-of select="AddressInfo[@type = 'ship']/State"/>
	  </shipto_state>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'ship']/Zip">
	  <shipto_postal_code>
	    <xsl:value-of select="AddressInfo[@type = 'ship']/Zip"/>
	  </shipto_postal_code>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'ship']/Country">
	  <shipto_country quote="false">
	    <xsl:call-template name="YahooCountry">
	      <xsl:with-param name="country"
			      select="AddressInfo[@type='ship']/Country"/>
	    </xsl:call-template>
	  </shipto_country>
	</xsl:if>
      </xsl:if>

      <!-- TODO: how does Yahoo encode Discover, MasterCard, and AmEx?
	   Once we know the answer we might be able to figure out how
	   to set cust_po_number
        -->
      <!-- cust_po_number-->

      <!-- fob -->

      <xsl:if test="Shipping">
	<ship_via>
	  <xsl:value-of select="Shipping"/>
	</ship_via>
      </xsl:if>

      <!-- hold_type -->

      <!-- shipping_chgs -->
      <!-- shipping_form -->
      <!-- ship_complete -->

      <currency>
	<xsl:choose>
	  <xsl:when test="@currency">
	    <xsl:value-of select="@currency"/>
	  </xsl:when>
	  <xsl:otherwise>
	    SELECT curr_abbr FROM curr_symbol WHERE (curr_id=basecurrid())
	  </xsl:otherwise>
	</xsl:choose>
      </currency>

      <!-- misc_charge_description -->
      <!-- misc_account_number -->
      <!-- misc_charge -->

      <xsl:if test="Total/Line[@type='Shipping']">
	<freight>
	  <xsl:value-of select="Total/Line[@type='Shipping']"/>
	</freight>
      </xsl:if>

      <order_notes>
	<xsl:if test="Comments">
	  <xsl:call-template name="cleanSQLChars">
	    <xsl:with-param name="inputStr" select="Comments"/>
	  </xsl:call-template>
	  <xsl:text>
</xsl:text>
	</xsl:if>
	<xsl:if test="CreditCard">
	  <xsl:value-of select="CreditCard"/>
	  type <xsl:value-of select="CreditCard/@type"/>
	  expiration <xsl:value-of select="CreditCard/@expiration"/>
	  <xsl:text>
</xsl:text>
	</xsl:if>
	<xsl:for-each select="AddressInfo">
	  <xsl:value-of select="@type"/>: 
	  <xsl:for-each select="Phone|Email">
	    <xsl:value-of select="name()"/>= <xsl:value-of select="text()"/>
	    <xsl:text> </xsl:text>
	  </xsl:for-each>
	  <xsl:text>
</xsl:text>
	</xsl:for-each>
	<xsl:for-each select="AddressInfo/Custom">
	  <xsl:value-of select="concat(name(), '-', @name)"/>: <xsl:call-template name="cleanSQLChars">
	    <xsl:with-param name="inputStr" select="text()"/>
	  </xsl:call-template>
	  <xsl:text>
</xsl:text>
	</xsl:for-each>
	<xsl:if test="Coupon">
	  Coupon: <xsl:call-template name="cleanSQLChars">
	    <xsl:with-param name="inputStr" select="Coupon/*"/>
	  </xsl:call-template>
	  <xsl:text>
</xsl:text>
	</xsl:if>
	<xsl:for-each select="NumericTime		|
			      Referer			|
			      Entry-Point		|
			      IPAddress			|
			      YahooLogin		|
			      Trackable-Link		|
			      Yahoo-Shopping-Order	|
			      Space-Id			|
			      Bogus			|
			      OSBS			|
			      Warning			|
			      Suspect			|
			      Store-Status		|
			      HTTP-User-Agent		|
			      GiftWrap			|
			      GiftWrapMessage		|
			      CardEvents">
	  <xsl:sort select="name()"/>
	  <xsl:variable name="name" select="name()"/>
	  <xsl:value-of select="name()"/>: <xsl:call-template name="cleanSQLChars">
	    <xsl:with-param name="inputStr" select="text()"/>
	  </xsl:call-template>
	  <xsl:for-each select="@*">
	    <xsl:value-of select="name()"/>=<xsl:call-template name="cleanSQLChars">
	      <xsl:with-param name="inputStr" select="text()"/>
	    </xsl:call-template>
	  </xsl:for-each>
	  <xsl:text>
</xsl:text>
	</xsl:for-each>
      </order_notes>

      <!-- shipping_notes -->
      <!-- add_to_packing_list_batch -->

    </salesorder>
    <xsl:apply-templates select="Item"/>
  </xsl:template>

  <xsl:template match="Item">
    <salesline>
      <order_number>
	<xsl:value-of select="substring-after(../@id, concat(/OrderList/@StoreAccountName, '-'))"/>
      </order_number>

      <line_number>
	<xsl:value-of select="@num + 1"/>
      </line_number>

      <item_number>
	<xsl:value-of select="Code"/>
      </item_number>

      <!-- customer_pn -->
      <!-- substitute_for -->
      <!-- sold_from_whs -->

      <qty_ordered>
	<xsl:value-of select="Quantity"/>
      </qty_ordered>

      <net_unit_price>
	<xsl:value-of select="Unit-Price"/>
      </net_unit_price>

      <scheduled_date quote="false">
	DATE '<xsl:value-of select="../Time"/>' + INTERVAL '<xsl:value-of select="$salesline_sched_offset"/>'
      </scheduled_date>

      <!-- tax_code -->
      <!-- discount_pct_from_list -->
      <!-- create_order -->
      <!-- overwrite_po_price -->

      <notes>
	<!-- ignore Thumb -->
	<xsl:for-each select="Code		|
			      Description	|
			      Url		|
			      Taxable">
	  <xsl:sort select="name()"/>
	  <xsl:value-of select="name()"/>: <xsl:call-template name="cleanSQLChars">
	    <xsl:with-param name="inputStr" select="text()"/>
	  </xsl:call-template>
	  <xsl:text>
  </xsl:text>
	</xsl:for-each>
      </notes>
    </salesline>

    <xsl:apply-templates select="Option"/>
  </xsl:template>

  <xsl:template name="saleslinechar">
    <xsl:param name="id"/>
    <xsl:param name="line-number"/>
    <xsl:param name="characteristic"/>
    <xsl:param name="value"/>
    <saleslinechar>
      <order_number>
	<xsl:value-of select="$id"/>
      </order_number>

      <line_number>
	<xsl:value-of select="$line-number"/>
      </line_number>

      <characteristic>
	<xsl:value-of select="$characteristic"/>
      </characteristic>

      <value>
	<xsl:value-of select="$value"/>
      </value>
    </saleslinechar>
  </xsl:template>

  <xsl:template match="Option">
    <xsl:call-template name="saleslinechar">
      <xsl:with-param name="id">
	<xsl:value-of select="substring-after(../../@id, concat(/OrderList/@StoreAccountName, '-'))"/>
      </xsl:with-param>
      <xsl:with-param name="line-number"	select="../@num + 1"	/>
      <xsl:with-param name="characteristic"	select="@name"		/>
      <xsl:with-param name="value"		select="text()"		/>
    </xsl:call-template>
  </xsl:template>

</xsl:stylesheet>
