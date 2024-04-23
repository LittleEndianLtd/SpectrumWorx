<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml"/>

<!-- copy all elements and their attributes-->
<xsl:template match="* | @*">
<xsl:copy><xsl:copy-of select="@*"/><xsl:apply-templates/></xsl:copy>
</xsl:template>

</xsl:stylesheet>
