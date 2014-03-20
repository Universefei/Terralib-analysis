<?php
import_request_variables("gpc");
require("global.inc.php");

if (!isset($V))
{
	$filename = "images/discworld.jpg";
	$handle = fopen($filename, "r");
	$data = fread($handle, filesize($filename));
	fclose($handle);
	Header("Content-Type: image/png");
	echo $data;
	exit;
}

$terraweb_->setcurrentview($V);
$themes = array();
if (isset($T))
	$themes = explode (";",$T);
if (isset($CT))
	$themes[] = $CT;
$box = $terraweb_->getcurrentviewbox();
if (!isset($X1))
	$X1 = $box[0];
if (!isset($Y1))
	$Y1 = $box[1];
if (!isset($X2))
	$X2 = $box[2];
if (!isset($Y2))
	$Y2 = $box[3];
$box = $terraweb_->setworld($X1,$Y1,$X2,$Y2,$TX,$TY);
//var_dump ($themes);
$i = 0;
for($i = 0; $i < count($themes); ++$i)
{
	$terraweb_->setTheme($themes[$i]);

        $terraweb_->plotraster();
}
$data = $terraweb_->getjpg();

if($data == false)
{
	$filename = "images/blank.jpg";
	$handle = fopen($filename, "r");
        $data = fread($handle, filesize($filename));
	fclose($handle);
        Header("Content-Type: image/png");
	echo $data;
						
}
else
{
	Header("Content-Type: image/jpeg");
	echo $data; 
	flush();
}
?>
