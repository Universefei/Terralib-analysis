<?php
session_start();
import_request_variables("gpc");
if (isset ($N)) // check if one specific slide is requested (Nth slide)
{
	$slide = "slide".$N;
	if (isset($_SESSION["$slide"])) // check if this slide is stored as session variable
	{
		echo stripslashes($_SESSION["$slide"]);
		exit;
	}
}
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
$objs = array();
if (isset($OBJS))
	$objs = explode (";",$OBJS);
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

$i = 0;
for($i = 0; $i < count($themes); ++$i)
{
	$terraweb_->setTheme($themes[$i],0);
	$terraweb_->plotVector();
}

if ((isset($X) && isset($Y)) || isset($OBJS))
{
	$terraweb_->settheme($CT,0);
	$terraweb_->setdefaultvisual(1,255,255,255,0,2,"",100,100,100,0,1);
	$terraweb_->setdefaultvisual(2,255,0,255,0,2,"");
	$terraweb_->setdefaultvisual(4,255,0,0,2,10,"");
	$delta = ($X2-$X1)/100;
	if (!isset($OBJS))
	{
		$data = $terraweb_->locateobject($X,$Y,$delta,0);
		$objs[0] = $data["object_id"];
	}
	$terraweb_->plotselectedobjects($objs);
	$terraweb_->plotpoint($X,$Y);
	if (isset($TEXT))
	{
		$terraweb_->setdefaultvisual(128,0,255,0,0,0,"");
		$terraweb_->plottext($X,$Y,$TEXT,0.,0.,-1);
	}
}

$data = $terraweb_->getpng();
if (isset ($N)) // check if one specific slide is requested (Nth slide)
	$_SESSION["$slide"] = addslashes($data); // store image data as session variable

Header("Content-Type: image/png");
echo $data; 
flush(); 
?>
