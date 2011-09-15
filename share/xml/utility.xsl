<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:html="http://www.w3.org/1999/xhtml"
                xmlns:fo="http://www.w3.org/1999/XSL/Format">

  <!--
    this identity template will ensure that untransformed elements will be
    passed through intact. this should eventually lead to invalid
    documents, whether XML that doesn't match the DTD or SQL with bad syntax.
    -->

  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template name="cleanSQLChars">
    <xsl:param name="inputStr"/>
    <xsl:variable name="squote">'</xsl:variable>

    <!-- add when statements for chars in SQL that require special treatment.
         don't split the long lines or you'll get extra spaces in the output
     -->
    <xsl:choose>

      <xsl:when test="contains($inputStr, $squote)">
	<xsl:value-of select="substring-before($inputStr, $squote)"/>\'<xsl:call-template name="cleanSQLChars"><xsl:with-param name="inputStr"><xsl:value-of select="substring-after($inputStr, $squote)"/></xsl:with-param></xsl:call-template>
      </xsl:when>

      <xsl:when test="contains($inputStr, '&#x22;')">
	<xsl:value-of select="substring-before($inputStr, '&#x22;')"/>\"<xsl:call-template name="cleanSQLChars"><xsl:with-param name="inputStr"><xsl:value-of select="substring-after($inputStr, '&#x22;')"/></xsl:with-param></xsl:call-template>
      </xsl:when>

      <xsl:otherwise>
	<xsl:value-of select="$inputStr"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="stripAlpha">
    <xsl:param name="number"/>
    <xsl:value-of select="translate($number,
				    '-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ',
				    '')"/>
  </xsl:template>

</xsl:stylesheet>
