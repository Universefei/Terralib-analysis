<html>
<head>
<title>Painel TerraWeb</title>

<?php 
require("css.php");
?>
<script type="text/javascript">

function getElById(idVal) {
  if (document.getElementById != null)
    return document.getElementById(idVal)
  if (document.all != null)
    return document.all[idVal]
  
  alert("Problem getting element by id")
  return null
}

function goOn(url)
{
	params = "?";
	V = document.general.VISTA.value;
	params = params+"V="+V+"&";
	if (url == 0)
	{
		location.href='paneladd.php'+params;
		return;
	}
	CT = document.general.TEMA.value;
	params = params+"CT="+CT+"&";
	if (url == 1)
	{
		location.href='paneladd.php'+params;
		return;
	}
	x1 = document.general.X1.value;
	y1 = document.general.Y1.value;
	x2 = document.general.X2.value;
	y2 = document.general.Y2.value;
	params = params+"X1="+x1+"&";
	params = params+"Y1="+y1+"&";
	params = params+"X2="+x2+"&";
	params = params+"Y2="+y2+"&";
	if (url >= 2)
	{
		nthemes = document.general.nthemes.value - 1;
		themes = "";
		j = 0;
		for (i=0;i<nthemes;i++)
		{
			var name = "VISIBLE"+i;
			if (getElById(name).checked)
			{
				if (j>0)
					themes += ";"
				themes += getElById(name).name;
				j++;
			}
		}
		params = params+"T="+themes+"&";
		if (url == 2)
			parent.canvas.location.href='canvas.php'+params;
		else if (url==3)
		{
			if (number.length == 0 || street.length == 0)
			{
				alert ("Entre com o nome da rua e numero");
				return;
			}
			parent.graphic.location.href='locateadd.php'+params;
		}
		else
			parent.canvas.location.href='slide.php'+params;
	}
}

</script>

<?php
import_request_variables("gp");
require("global.inc.php");
$views = $terraweb_->getviews();

if (!isset ($V))
	$V = $views[0];
$terraweb_->setcurrentview ($V);
$box = $terraweb_->getcurrentviewbox();

$themes = $terraweb_->getthemes();
if (!isset ($CT))
	$CT = $themes[0];
$nthemes = count($themes);

?>

</head>
<body leftmargin="0" rightmargin="0" topmargin="0">

<form name="general">
<input type="hidden" name="X1" value="<?=$box[0]?>">
<input type="hidden" name="Y1" value="<?=$box[1]?>">
<input type="hidden" name="X2" value="<?=$box[2]?>">
<input type="hidden" name="Y2" value="<?=$box[3]?>">
<input type="hidden" name="V" value="<?=$V?>">
<input type="hidden" name="CT" value="<?=$CT?>">
<input type="hidden" name="nthemes" value="<?=$nthemes?>">
<table width="100%" border="0" cellspacing="1">
	<tr>
		<td colspan="2" align="center" nowrap><b>Vista Corrente</b></td>
	</tr>
	<tr>
		<td colspan="2" align="center">
			<select name="VISTA" onChange="goOn(0)">
<?php
for ($j=0;$j<count($views);$j++)
{
	$selected = $views[$j]==$V ? "selected" : "";
	echo "<option value=\"".$views[$j]."\" $selected >".$views[$j]."</option>\n";
}
?>
			</select>
		</td>
	</tr>
	<tr>
		<td colspan="2" align="center" nowrap><b>Tema Corrente</b></td>
	</tr>
	<tr>
		<td colspan="2" align="center">
			<select name="TEMA" onChange="goOn(1)">
<?php
for ($j=0;$j<count($themes);$j++)
{
	$selected = $themes[$j]==$CT ? "selected" : "";
	echo "<option value=\"".$themes[$j]."\" $selected >".$themes[$j]."</option>\n";
}
?>
			</select>
		</td>
	</tr>
	<tr>
		<td align="center" colspan="2">
		<table width="100%" border="1" cellspacing="1">
		<tr><th>Tema</th><th>Vis&iacute;vel</th></tr>
<?php
$j = 0;
for ($i=0;$i<$nthemes;$i++)
{
	$theme = $themes[$i];
	if ($theme != $CT)
	{
	echo "<tr><td align=\"right\">$theme</td><td align=\"center\"><input type=\"checkbox\" name=\"$theme\" id=\"VISIBLE$j\"></td></tr>\n";
		$j++;
	}
}
?>
		</table>
		</td>
	</tr>
	<tr>
		<td align="center" colspan="2">
		<input type="button" value="Desenhar" onClick="goOn(2)"></td>
	</tr>
	<tr>
	<td align="center" colspan="2">
	<a href="http://terralib.dpi.inpe.br/" target="_blank">
	<img src="images/logo_terralib.gif" border="0">
	</a>
	</td>
	</tr>
</table>
</form>
</body>
</html>
