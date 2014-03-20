<?
/*
	Desenha a legenda de um mapa.
 */
	$t = terraweb();
	
	if(!$t->connect("localhost", "postgres", "", "terraweb320", 5432, 2))
	{
		echo("Nao foi possivel conectar-se ao banco de dados<BR>");
		echo($t->errorMessage());
		exit();
	}
	
	if($t->setCurrentView("sp") == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	
	$box = $t->getcurrentviewbox();
	
	if($box == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	
	$t->setCanvasBackgroundColor(255, 255, 255);
	$t->setWorld($box[0], $box[1], $box[2], $box[3], 800, 600);
	
	$themes = $t->getthemes();
	
	if($themes == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	
	$legenda;
	
	for($i = 0; $i < count($themes); ++$i)
	{
		$t->setTheme($themes[$i], 0);
		
		$result = $t->drawCurrentTheme();
		
		if($result == false)
		{
			echo($t->errorMessage() . "<BR>");
			exit();
		}
		
		$legenda[$themes[$i] ] = $result;
	}
	
	$res = $t->setDefaultVisual(128, 0, 0, 0, 0, 2, "");
	
	if($res == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	
	$res = $t->drawLegend($legenda, 350);
	
	if($res == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	
	$imagemSaida = $t->getLegendImage(0, 1);
	
	if($imagemSaida == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	
	header ("Content-type: image/png");
	echo($imagemSaida);
?>