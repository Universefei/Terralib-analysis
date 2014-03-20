<html>
<head>
<?php
import_request_variables("gpc");
require("css.php");
require("global.inc.php");
?>
</head>
<body>
<?php

$terraweb_->setcurrentview($V);
$terraweb_->settheme($CT,0);
$data = $terraweb_->findaddress ($street,$number);
//var_dump($data);
?>
<input type="hidden" name="params" id="params" value="<?=$params?>">

<table>
<th>Rua</th><th>Bairro</th>
<?php
$params =  $_SERVER['QUERY_STRING'];
parse_str($params, $out);

$dx = ($X2 - $X1) / 200;
foreach ($data as $addresses)
{
	$X = $addresses[3];
	$Y = $addresses[4];
	$out["X1"] = $X - $dx;
	$out["X2"] = $X + $dx;
	$out["Y1"] = $Y - $dx;
	$out["Y2"] = $Y + $dx;
	$QSTRING = "?";
	
	foreach ($out as $key=> $val )
		$QSTRING .= $key.'='.urlencode($val).'&';

	$QSTRING .= "X=".$X."&Y=".$Y."&OBJS=".$addresses[5]."&TEXT=".$addresses[0].",".$number;
?>
<tr>
<td align="left" nowrap><a target="canvas" href="canvas.php<?=$QSTRING?>"><?=$addresses[0]?></a></td>
<td align="left" nowrap><?=$addresses[1]?>&nbsp</td>
</tr>
<?php
}
?>
</body>
</html>