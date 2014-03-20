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
$delta = ($X2-$X1)/100;
$data = $terraweb_->locateobject($X,$Y,$delta,0);
$data = $terraweb_->fetchattributes($data["object_id"],0);
?>
<table>
<th>Campo</th><th>Valor</th>
<?php
foreach ($data as $key => $value)
{
?>
<tr>
<td align="right" nowrap><?=$key?>&nbsp</td>
<td align="left" nowrap><?=$value?>&nbsp</td>
</tr>
<?php
}
?>
</body>
</html>
