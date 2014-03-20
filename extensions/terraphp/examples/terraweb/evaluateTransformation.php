<?php
function evaluateTransformation(&$xmin, &$xmax, &$ymin, &$ymax, &$width, &$height, $extend)
{
	$dxw = $xmax - $xmin;
	$dyw = $ymax - $ymin;
					
	$fx = $width/$dxw;
	$fy = $height/$dyw;

	if ($fx > $fy)
	{
		$f = $fy;
		if ($extend)
		{
			$dxw = $width/$f;
			$xc = ($xmin+$xmax)/2;
			$xmin = $xc - $dxw/2;
			$xmax = $xc + $dxw/2;
		}
		else
			$width = round($f*$dxw);
	}
	else
	{
		$f = $fx;
		if ($extend)
		{
			$dyw = $height/$f;
			$yc = ($ymin+$ymax)/2;
			$ymin = $yc - $dyw/2;
			$ymax = $yc + $dyw/2;
		}
		else
			$height = round($f*$dyw);
	}
	return $f;
}
?>