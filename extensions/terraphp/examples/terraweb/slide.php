<?php
session_start();
?>
<html>
<head>
<title> Animação </TITLE>
<?php
require("css.php");
import_request_variables("gpc");
?>
<!-- configurable script -->
<script language="JavaScript">
theimage = new Array();

///// Configurable Variables

// The dimensions of ALL the images should be the same or some of them may look stretched or reduced in Netscape 4.
// Format: theimage[...]=[name/description, image_src, linking_url]
<?php
$i=0;
$tamx = 600;
$tamy = 450;
$xc = $X1+($X2 - $X1)/2.;
$yc = $Y1+($Y2 - $Y1)/2.;
for ($i=0; $i<8; $i++)
{
	$slide = "slide".$i;
	if (isset($_SESSION["$slide"])) // check if this slide is stored as session variable
		session_unregister("$slide"); 
	$params = "?";

	$params .= "V=".$V."&";
	$params .= "CT=".$CT."&";
	if (strlen($themes) > 0)
		$params .= "T=".$themes."&";
	if (isset($TEXT))
		$params .= "TEXT=".$TEXT."&";
	if (isset($OBJS))
		$params .= "OBJS=".$OBJS."&";
	$params .= "TX=".$tamx."&";
	$params .= "TY=".$tamy."&";
	$params .= "X1=".$X1."&";
	$params .= "Y1=".$Y1."&";
	$params .= "X2=".$X2."&";
	$params .= "Y2=".$Y2."&";
	if (isset($X))
		$params .= "X=".$X."&";
	if (isset($Y))
		$params .= "Y=".$Y."&";

	$params .= "N=".$i."&";
	$fn = "plotVector.php".$params;
	$cn = "canvas.php".$params;

	echo "theimage[$i]=['$i', '$fn', '$cn'];\n";
	$dx = ($X2 - $X1) * 0.6/2.;
	$dy = ($Y2 - $Y1) * 0.6/2.;

	$X1 = $xc - $dx;
	$X2 = $xc + $dx;
	$Y1 = $yc - $dy;
	$Y2 = $yc + $dy;
}
?>


///// Plugin variables

playspeed=1500;// The playspeed determines the delay for the "Play" button in ms
dotrans=1; // if value = 1 then there are transitions played in IE
transtype='blendTrans';// 'blendTrans' or 'revealtrans'
transattributes='duration=1,transition=3';// duration=seconds,transition=#<24
linksperline=4; // For the text links
playdiffernce=500; // The speed that the autplay speed is changed by
autostart=1;
preloadslides=1;
</script>
<!-- end of configurable script -->
<!-- plugin scripts -->
<script>

// Preloader
function preloadSlide()
{
	for(k=0;k<til;k++)
		theimage[k][1]=new Image().src=theimage[k][1];
}

// Add extra functions between the {} if you need other functions that require the onload
window.onload=function()
{
	if(window.preloadslides)preloadSlide();
	if(window.autostart)slidePlay();
}

// NS4 resize handler
window.onresize=function()
{
	if(document.layers)this.location=this.location;
}
</script>
<!-- end of plugin scripts -->
<!-- slide show HTML -->
</head>

<body>

<form NAME="slideshow">
<div ALIGN="CENTER">
<table border=1 cellpadding=0 cellspacing=0 >
	<tr>
	<td align=center>
		<table border=0 cellpadding=2 cellspacing=1>
		<tr>
		<td align="center"><select name="theimages" 
	onchange="slideImage(document.slideshow.theimages.selectedIndex);"></select><script>
for (y=0;y<theimage.length;y++) {document.slideshow.theimages[y]=new Option(theimage[y][0])}</script>
		</td>
		</tr>
		<tr>
		<td align="right">
		<INPUT TYPE="button" VALUE="|<<" onclick="slideImage(0);" TITLE="Início" > 
		<INPUT TYPE="button" VALUE="<<" onclick="slideImage(i-1);" TITLE="Imagem Anterior" > 
		<INPUT TYPE="button" NAME="play" VALUE="   Play   " onclick="slidePlay();" TITLE="Automático" > 
		<INPUT TYPE="button" VALUE=">>" onclick="slideImage(i+1);" TITLE="Imagem Posterior" > 
		<INPUT TYPE="button" VALUE=">>|" onclick="slideImage(theimage.length-1);" TITLE="Fim" >
		</td>
		</tr>
		</td>
		</tr>
		</table>
	<tr>
	<td>
	<table border=0 cellpadding=0 cellspacing=0 height="100%" width="100%">
	<tr>
	<td valign=top align=center width="<?=$tamx?>" height="<?=$tamy?>">
	<a HREF="#" onmouseover="this.href=theimage[i][2];return false">
	<script>
	document.write('<IMG NAME="slideimage" STYLE="filter:'+transtype+'('+transattributes+');" SRC="'+theimage[0][1]+'" BORDER=0>')
	</script>
	</a>	
	</td>
</tr>
</table>
</td>
</tr>
</table>
</form></div>
<!-- end of slide show html -->
<!-- fixed script -->
<script>
///// DO NOT EDIT THIS SCRIPT
with(document)
{
	i=0,
	play=0,
	di=images,
	dl=links,
	form=slideshow,
	til=theimage.length
}

function slideImage(num)
{
// Required Script
	i=num%til;
	if(i<0)i=til-1;
	if(document.all&&dotrans==1)eval('di.slideimage.filters.'+transtype+'.apply()')
	di.slideimage.src=theimage[i][1];
	if(document.all&&dotrans==1)eval('di.slideimage.filters.'+transtype+'.play()')
// Plug-in handlers
	if(form.theimages)form.theimages.selectedIndex = i;
	if(form.slidebox)form.slidebox.value = theimage[i][0];
	if(form.slidecount)form.slidecount.value = "Image "+(i+1)+" of "+til;
	if(form.slidespeed)doSpeed(0);
}

function slidePlay()
{
	if (!window.playing)
	{
		slidePlaying(i+1);
		if(form.play)form.play.value="   Stop   ";
	}
	else
	{
		playing=clearTimeout(playing);
		if(form.play)form.play.value="   Play   ";
	}
	if(di.imgPlay)
	{
		setTimeout('di.imgPlay.src="'+imgStop+'"',1);
		imgStop=di.imgPlay.src
	}
}

function slidePlaying(num)
{
	playing=setTimeout('slidePlaying(i+1);slideImage(i+1);', playspeed);
}

//slideImage(0);
</script>
<!-- end of fixed script -->
</body>
</html>