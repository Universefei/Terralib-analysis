<html>
<head>
<title>Mosaico do Brasil</title>
<?php
require("css.php");
require("evaluateTransformation.php");
import_request_variables("gpc");
?>
<script type="text/javascript">
<?php
if (!isset($check))
	$check="false";
echo "var check=$check\n";
?>
var params = "?";

function goNext(operation)
{
	if(check && operation=="")
		parent.graphic.location.href='content.php';
	params = params + "check="+check+"&";
	V = document.general.V.value;
	params = params+"V="+V+"&";
	CT = document.general.CT.value;
	params = params+"CT="+CT+"&";
	if (document.general.X.value.length > 0)
	{
		x = parseFloat(document.general.X.value);
		params = params+"X="+x+"&";
	}
	if (document.general.Y.value.length > 0)
	{
		y = parseFloat(document.general.Y.value);
		params = params+"Y="+y+"&";
	}
	
	T = document.general.T.value;
	params = params+"T="+T+"&";

	x1 = parseFloat(document.general.X1.value);
	y1 = parseFloat(document.general.Y1.value);
	x2 = parseFloat(document.general.X2.value);
	y2 = parseFloat(document.general.Y2.value);
	dx = x2-x1;
	dy = y2-y1;
	dxp = dx/4.;
	dyp = dy/4.;
	var f = 0.6;
	switch (operation)
	{
		case "NO":
		y1 = y1 + dyp;
		y2 = y2 + dyp;
		break;
		case "SO":
		y1 = y1 - dyp;
		y2 = y2 - dyp;
		break;
		case "WE":
		x1 = x1 - dxp;
		x2 = x2 - dxp;
		break;
		case "EA":
		x1 = x1 + dxp;
		x2 = x2 + dxp;
		break;
		case "NE":
		x1 = x1 + dxp;
		x2 = x2 + dxp;
		y1 = y1 + dyp;
		y2 = y2 + dyp;
		break;
		case "NW":
		x1 = x1 - dxp;
		x2 = x2 - dxp;
		y1 = y1 + dyp;
		y2 = y2 + dyp;
		break;
		case "SE":
		x1 = x1 + dxp;
		x2 = x2 + dxp;
		y1 = y1 - dyp;
		y2 = y2 - dyp;
		break;
		case "SW":
		x1 = x1 - dxp;
		x2 = x2 - dxp;
		y1 = y1 - dyp;
		y2 = y2 - dyp;
		break;
		case "ZI":
		xc = (x1+x2)/2.;
		yc = (y1+y2)/2.;
		dxp = dx*f/2.;
		dyp = dy*f/2.;
		x1 = xc - dxp;
		x2 = xc + dxp;
		y1 = yc - dyp;
		y2 = yc + dyp;
		break;
		case "ZO":
		xc = (x1+x2)/2.;
		yc = (y1+y2)/2.;
		dxp = dx/f/2.;
		dyp = dy/f/2.;
		x1 = xc - dxp;
		x2 = xc + dxp;
		y1 = yc - dyp;
		y2 = yc + dyp;
		break;
		default : 
	}

	params = params+"X1="+x1+"&";
	params = params+"Y1="+y1+"&";
	params = params+"X2="+x2+"&";
	params = params+"Y2="+y2+"&";
	location.href='canvas.php'+params;
	if(check && operation=="")
		parent.graphic.location.href='locate.php'+params;
}

<?php


function AbsPosButton($id,$text,$x,$y,$operation)
{
	global $OFFX,$OFFY;
	printf ("<div id=\"%s\" style=\"position:absolute;left:%4d;top:%4d;\">\n<a",$id,$OFFX+$x,$OFFY+$y);
	printf (" onClick=\"goNext('%s')\"",$operation);
	printf (">\n<img src=\"images/%s.gif\" alt=\"%s\" border=\"0\"></a></div>\n",$id,$text);
}

function AbsPos($id,$x,$y)
{
	global $OFFX,$OFFY;
	printf("<div nowrap id=\"%s\" STYLE=\"position:absolute;LEFT:%d;TOP:%d;\">",$id,$OFFX+$x,$OFFY+$y);
}

function AbsPosEnd()
{
		printf ("</div>\n");
}

// BEGIN
// Get browser information
	$dy = 15;
	$dx = 15;

	$OFFX=10;
	$OFFY=10;
	$TAM = "M";
	switch ($TAM)
	{
		case "G":
		$tamx = 900;
		$tamy = 600;
		$yu = 2;
		$yl = 1;
		$fx = 6./3.;
		$fy = 4./2.;
		break;
		
		case "P":
		$tamx = 450;
		$tamy = 300;
		$fx = 1.;
		$fy = 1.;
		break;

		case "M":
		default :
		$TAM = "M";
		$tamx = 600;
		$tamy = 450;
		$fx = 4./3.;
		$fy = 3./2.;
		break;
	}
?>
function getImageXY(e)
{
	var nav4 = window.Event ? true : false;
	if (nav4)
	{ // Navigator 4.0x
		mouseX = e.pageX;
		mouseY = e.pageY;
	}
	else
	{ // Internet Explorer 4.0x
		mouseX = event.clientX + document.body.scrollLeft;
		mouseY = event.clientY + document.body.scrollTop;
	}
	mouseX = mouseX - <?=$dx?> - <?=$OFFX?>;
	mouseY = mouseY - <?=$dy?> - <?=$OFFY?>;
}

function processEvent(e)
{
	var lat,lon;
	getImageXY(e);
	x1 = parseFloat(document.general.X1.value);
	y1 = parseFloat(document.general.Y1.value);
	x2 = parseFloat(document.general.X2.value);
	y2 = parseFloat(document.general.Y2.value);
	dx = x2-x1;
	dy = y2-y1;
	if (check)
	{
		xc = (x1+x2)/2.;
		yc = (y1+y2)/2.;
		nxc = x1 + dx*mouseX/(<?=$tamx?>-1);
		nyc = y2 - dy*mouseY/(<?=$tamy?>-1);
		document.general.X.value = nxc;
		document.general.Y.value = nyc;
		x1 += nxc - xc;
		x2 += nxc - xc;
		y1 += nyc - yc;
		y2 += nyc - yc;
		goNext ("");
//		goLocate(nxc,nyc);
	}
	else
	{
		if (zooming)
		{
			stopCursor(e);
			nx1 = x1 + dx*zleft/(<?=$tamx?>-1);
			nx2 = x1 + dx*zright/(<?=$tamx?>-1);
			ny2 = y2 - dy*ztop/(<?=$tamy?>-1);
			ny1 = y2 - dy*zbottom/(<?=$tamy?>-1);

			document.general.X1.value = nx1;
			document.general.X2.value = nx2;
			document.general.Y1.value = ny1;
			document.general.Y2.value = ny2;
			goNext ("");
		}
		else
			startCursor(e);
	}
}

function checkBox()
{
	if(check != true)
		{
		document.getElementById('imgCheck').src = "images/true.png"; //change the image
		document.getElementById('inputCheck').value = "true"; //change the field value
		check = true; //change the value for this checkbox in the array
		}
	else
		{
		document.getElementById('imgCheck').src = "images/false.png";
		document.getElementById('inputCheck').value = "false";
		check = false;
		}
}

function createLayer(name, inleft, intop, width, height, visible, content)
{
	var layer;
	document.writeln('<div id="' + name + '" style="position:absolute; overflow:hidden; left:' + inleft + 'px; top:' + intop + 'px; width:' + width + 'px; height:' + height + 'px;' + '; z-index:1; visibility:' + (visible ? 'visible;' : 'hidden;') +  '">');
	document.writeln(content);
	document.writeln('</div>');
}
function getLayer(name)
{
	layer = eval(document.getElementById(name).style);
	return(layer);
}
function showLayer(name)
{
	var layer = getLayer(name);
	layer.visibility = "visible";
}
function hideLayer(name)
{
  	var layer = getLayer(name);
  	layer.visibility = "hidden";
}
function setLayerBackgroundColor(name, color) {
  	var layer = getLayer(name);
   	layer.backgroundColor = color;
}

function startCursor(e)
{
	if ((mouseX<<?=$tamx?>) && (mouseY<<?=$tamy?>))
	{
		if (zooming)
		{
			stopCursor(e);
		}
		else
		{
			mx1=mouseX;
			my1=mouseY;
			mx2=mx1+1;
			my2=my1+1;
			zleft=mx1;
			ztop=my1;
			zbottom=my1;
			zright=mx1
			boxIt(mx1,my1,mx2,my2);
			zooming=true;
		}
	}
}

function stopCursor(e)
{
	  zooming=false;
	  hideLayer("cursorTop");
	  hideLayer("cursorLeft");
	  hideLayer("cursorRight");
	  hideLayer("cursorBottom");
}

function boxIt(theLeft,theTop,theRight,theBottom)
{
	clipLayer("cursorTop",theLeft,theTop,theRight,theTop+cursorWidth);
	clipLayer("cursorLeft",theLeft,theTop,theLeft+cursorWidth,theBottom);
	clipLayer("cursorRight",theRight-cursorWidth,theTop,theRight,theBottom);
	clipLayer("cursorBottom",theLeft,theBottom-cursorWidth,theRight,theBottom);
	showLayer("cursorTop");
	showLayer("cursorLeft");
	showLayer("cursorRight");
	showLayer("cursorBottom");
}

function clipLayer(name, clipleft, cliptop, clipright, clipbottom)
{
	  var layer = getLayer(name);
	  var newWidth = clipright - clipleft;
	  var newHeight = clipbottom - cliptop;
	  layer.height = newHeight;
	  layer.width	= newWidth;
	  layer.top	= <?=$dy?> + <?=$OFFY?> + cliptop  + "px";
	  layer.left	= <?=$dx?> + <?=$OFFX?> + clipleft + "px";
}

function getMouse(e)
{
	getImageXY(e);
	x1 = parseFloat(document.general.X1.value);
	y1 = parseFloat(document.general.Y1.value);
	x2 = parseFloat(document.general.X2.value);
	y2 = parseFloat(document.general.Y2.value);
	dx = x2-x1;
	dy = y2-y1;

	x = x1 + dx*mouseX/(<?=$tamx?>-1);
	y = y2 - dy*mouseY/(<?=$tamy?>-1);

	parent.status = "X:" + x + " Y:" + y;
	if (!zooming)
		return false;
		
	if ((mouseX><?=$tamx?>) || (mouseY><?=$tamy?>) || (mouseX<=0) ||(mouseY<=0) )
		stopCursor(e);
	else
	{
		mx2=mouseX;
		my2=mouseY;
		setClip();
	}
}

function setClip()
{
//	zleft = min(mx1,mx2);
	if (mx1>mx2) {
		zright=mx1;
		zleft=mx2;
	} else {
		zleft=mx1;
		zright=mx2;
	}
	if (my1>my2) {
		zbottom=my1;
		ztop=my2;
	} else {
		ztop=my1;
		zbottom=my2;
	}
	if ((mx1 != mx2) && (my1 != my2)) {
		boxIt(zleft,ztop,zright,zbottom);
	}
}

</script>
</head>
<body>
<script>
var cursorColor = '#ff0000';
var mouseX,mouseY;
var zooming=false;

var mx1=0;
var my1=0;
var mx2=0;
var my2=0;

var zleft=0;
var zright=0;
var zooming=false;
var ztop=0;
var zbottom=0;
var cursorWidth = 2;

content = '<img name="zoomImageTop" src="images/spacer.gif" width=1 height=1>';
createLayer("cursorTop",0,0,100,1,false,content);
createLayer("cursorLeft",0,0,1,100,false,content);
createLayer("cursorRight",0,0,1,100,false,content);
createLayer("cursorBottom",0,0,100,1,false,content);

setLayerBackgroundColor("cursorTop", cursorColor);
setLayerBackgroundColor("cursorLeft", cursorColor);
setLayerBackgroundColor("cursorRight", cursorColor);
setLayerBackgroundColor("cursorBottom", cursorColor);

</script>
<?php

// Start processing
/*
	printf ("<div id=\"moldura\" style=\"position:absolute;left:%d;top:%d;\">\n",$OFFX,$OFFY);
	printf ("<img src=\"images/moldura%s.png\"></div>\n",$TAM);
*/
	printf ("<div id=\"moldura\" style=\"position:absolute;left:%d;top:%d;\">\n<a",$OFFX,$OFFY);
	printf (" onmousedown=\"processEvent(event)\" onmousemove=\"getMouse(event)\"");
	printf (">\n<img src=\"images/moldura%s.png\"></div>\n",$TAM);
	$params = "?";
	$params .= "V=".$V."&";
	$params .= "CT=".$CT."&";
	if (strlen($T) > 0)
		$params .= "T=".$T."&";
	if (isset($TEXT))
		$params .= "TEXT=".$TEXT."&";
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

	printf ("<div id=\"imagemj\" style=\"position:absolute;left:%d;top:%d;\">\n",$dx+$OFFX,$dy+$OFFY);
	printf ("<img SRC=\"plotRaster.php%s\"></div>\n",$params);
	
	printf ("<div id=\"imagemp\" style=\"position:absolute;left:%d;top:%d;\">\n",$dx+$OFFX,$dy+$OFFY);
	printf ("<img SRC=\"plotVector.php%s\"></div>\n",$params);

	AbsPosButton ("menos","Reduzir", 8+75*$fx,  0,"ZO");
	AbsPosButton ("mais","Ampliar", 8+375*$fx,  0,"ZI");

	AbsPosButton ("noroeste","Veja mais a Noroeste",  -2,  -2,"NW");
	AbsPosButton ("norte",   "Veja mais a Norte",   8+225*$fx,  1,"NO");
	AbsPosButton ("nordeste","Veja mais a Nordeste",17+450*$fx,  -2,"NE");
	AbsPosButton ("oeste",   "Veja mais a Oeste",     0,8+150*$fy,"WE");
	AbsPosButton ("leste",   "Veja mais a Leste",   14+450*$fx,8+150*$fy,"EA");
	AbsPosButton ("sudoeste","Veja mais a Sudoeste",  -2,17+300*$fy,"SW");
	AbsPosButton ("sul",     "Veja mais a Sul",     8+225*$fx,14+300*$fy,"SO");
	AbsPosButton ("sudeste", "Veja mais a Sudeste", 17+450*$fx,17+300*$fy,"SE");

	evaluateTransformation($X1, $X2, $Y1, $Y2, $tamx, $tamy, true);
	echo "<form name=\"general\">\n";
	echo "<input type=\"hidden\" name=\"street\" value=\"".$street."\">\n";
	echo "<input type=\"hidden\" name=\"number\" value=\"".$number."\">\n";
	echo "<input type=\"hidden\" name=\"X\" value=\"".$X."\">\n";
	echo "<input type=\"hidden\" name=\"Y\" value=\"".$Y."\">\n";
	echo "<input type=\"hidden\" name=\"X1\" value=\"".$X1."\">\n";
	echo "<input type=\"hidden\" name=\"Y1\" value=\"".$Y1."\">\n";
	echo "<input type=\"hidden\" name=\"X2\" value=\"".$X2."\">\n";
	echo "<input type=\"hidden\" name=\"Y2\" value=\"".$Y2."\">\n";
	echo "<input type=\"hidden\" name=\"V\" value=\"".$V."\">\n";
	echo "<input type=\"hidden\" name=\"CT\" value=\"".$CT."\">\n";
	echo "<input type=\"hidden\" name=\"T\" value=\"".$T."\">\n";
	echo "</form>\n";
?>
<div id="check" style="position:absolute;left: <?=$OFFX+8+150*$fx?>;top:  <?=$OFFY?>;">
<img src="images/<?=$check?>.png" id="imgCheck" onclick="checkBox()" alt="checkbox" />
</div>
<input type="hidden" name="input_check" id="inputCheck" value="<?=$check?>">
</body>
</html>
