<?xml version="1.0"?>
<xsl:stylesheet version="2.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output indent="yes"
	      method="xml"
	      doctype-system="QPH"/>
  <xsl:param name="max-length" select="60"/>
  <xsl:param name="min-count"  select="5" />

  <xsl:template match="TS">
    <QPH>
      <xsl:for-each-group select="context[starts-with(name/text(), 'menu')]/message" group-by="source/text()">
        <xsl:sort select="current-grouping-key()"/>
        <!--xsl:message>found <xsl:value-of select="current-grouping-key()"/></xsl:message-->
        <phrase>
          <source>
            <xsl:value-of select="current-grouping-key()"/>
          </source>
          <target/>
        </phrase>
      </xsl:for-each-group>

      <xsl:for-each-group select="context[not(starts-with(name/text(), 'menu'))]/message" group-by="source/text()">
	<xsl:sort select="current-grouping-key()"/>
	<xsl:if test="string-length(current-grouping-key()) &lt;= $max-length
	           and count(current-group()) &gt;= $min-count">
	  <!--xsl:message>found <xsl:value-of select="current-grouping-key()"/><xsl:text> </xsl:text><xsl:value-of select="count(current-group())"/> times</xsl:message-->
	  <phrase>
	    <source>
	      <xsl:value-of select="current-grouping-key()"/>
	    </source>
	    <target/>
	  </phrase>
	</xsl:if>
      </xsl:for-each-group>
    </QPH>
  </xsl:template>

</xsl:stylesheet>
