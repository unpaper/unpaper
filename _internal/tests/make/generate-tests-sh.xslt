<?xml version="1.0" encoding="UTF-8" ?>

<!--
    XSLT Transformation to convert testcase-descriptions into ANT-build script.
    Author: Jens Gulden
-->

<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:fo="http://www.w3.org/1999/XSL/Format">

    <xsl:output method="text" encoding="ISO-8859-1"/>

    <xsl:template match="/">
        <xsl:apply-templates select="test-environment"/>
    </xsl:template>
    
    
    <xsl:template match="test-environment">
# generated via xslt
UNPAPER=/home/jgulden/workspace/unpaper/unpaper
PNGTOPNM=/usr/bin/pngtopnm
PNMTOPNG=/usr/bin/pnmtopng
TMP=/tmp
        <xsl:apply-templates select="testcase"/>            
    </xsl:template>

    <xsl:template match="testcase">
# ----------------------------------------------------------------------------    
# [<xsl:value-of select="@id"/>] <xsl:value-of select="@title"/>
        
<!-- convert input files  -->
<xsl:for-each select="input-file">
$PNGTOPNM ../img/<xsl:value-of select="substring-before(@name,'.')"/>.png > <xsl:value-of select="@name"/>
</xsl:for-each>
        
<!-- run unpaper with options specific to this testcase  -->
$UNPAPER --version > ../log/test<xsl:value-of select="@id"/>.txt
$UNPAPER -v --overwrite <xsl:value-of select="normalize-space(options)"/> >> ../log/test<xsl:value-of select="@id"/>.txt
cat ../log/test<xsl:value-of select="@id"/>.txt
<!-- convert output files  -->
<xsl:for-each select="output-file">
$PNMTOPNG <xsl:value-of select="@name"/> > ../img-results/<xsl:value-of select="substring-before(@name,'.')"/>.png
rm <xsl:value-of select="@name"/>
</xsl:for-each>
     
<!-- delete converted input files  -->
<xsl:for-each select="input-file">
rm <xsl:value-of select="@name"/>
</xsl:for-each>
        
echo Done: [<xsl:value-of select="@id"/>] <xsl:value-of select="@title"/>.
echo
            
    </xsl:template>

    <xsl:template match="*" priority="-1"/>

</xsl:stylesheet> 