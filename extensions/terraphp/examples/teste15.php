<?
/*
	Desenha o mapa de trechos do tema Eixos de Rua.
 */
	$t = terraweb();
	
	if(!$t->connect("localhost", "postgres", "", "terraweb320", 5432, 2))
	{
		echo("Nao foi possivel conectar-se ao banco de dados<BR>");
		echo($t->errorMessage());
		exit();
	}
	
	if($t->setCurrentView("taubate") == false)
	{
		echo($t->errorMessage() . "vvvv<BR>");
		exit();
	}
	$t->setCanvasBackgroundColor(255, 255, 255);
	
	$box = $t->getcurrentviewbox();
	
	if($box == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	$x1 = $box[0];
	$y1 = $box[1];
	$x2 = $box[2];
	$y2 = $box[3];
	/*
	$x1 = 477833.18873559905;
	$y1 = 7481130.626617269;
	$x2 = 480170.0549485874;
	$y2 = 7482649.270461108;
	*/
	$t->setWorld($x1, $y1, $x2, $y2, 1024, 768);
	
	$t->setTheme("eixos_taubate", 0);
	
	//$result = $t->plotvector(); // 314
	$result = $t->drawCurrentTheme();
	
	if($result == false)
	{
		echo($t->errorMessage() . "dddd<BR>");
		exit();
    }
	
	//$imagemSaida = $t->getpng(); // 314
	$imagemSaida = $t->getCanvasImage(0, 1);
	
	if($imagemSaida == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	
	header ("Content-type: image/png");
	echo($imagemSaida);
?>
