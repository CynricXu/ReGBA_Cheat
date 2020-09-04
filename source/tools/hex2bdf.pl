#!/usr/bin/perl
# From http://czyborra.com/unifont/hex2bdf
# usage: ./hex2bdf.pl < HEX_FILE > BDF_FILE

while (<>) { $glyph{$1} = $2 if /(....):(.+)\n/; }
@chars = sort keys %glyph; $[ = 1;
# dbmopen (%charname, "/usr/share/unicode/unidata/charname.db", 0);

print "STARTFONT 2.1
FONT -gnu-unifont-medium-r-normal--16-160-75-75-c-80-iso10646-1
SIZE 16 75 75
FONTBOUNDINGBOX 16 16 0 -2
STARTPROPERTIES 3
FONT_ASCENT 14
FONT_DESCENT 2
DEFAULT_CHAR 32
ENDPROPERTIES
CHARS $#chars\n";

foreach $character (@chars)
{
	$encoding = hex($character); $glyph = $glyph{$character};
	$width = length ($glyph) > 32 ? 2 : 1;
	$dwidth = $width * 8; $swidth= $width * 500;
	$glyph =~ s/((..){$width})/\n$1/g;
	$character = "$character $charname"
	    if $charname = $charname{pack("n",hex($character))};

	print "STARTCHAR U+$character
ENCODING $encoding
SWIDTH $swidth 0
DWIDTH $dwidth 0
BBX $dwidth 16 0 -2
BITMAP $glyph
ENDCHAR\n";
}

print "ENDFONT\n";