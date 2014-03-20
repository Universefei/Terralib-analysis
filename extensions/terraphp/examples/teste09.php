<?
/*
	Desenha o mapa de distritos de Sao Paulo representado pelo tema distritos_sp,
	e desenha um poligono retângular com o box de um poligono de distrito.
	
	ajustes: parâmetros de conexao e nome da coluna de identificacao "object_id_1" no meu banco.
	
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
	
	$t->setCanvasBackgroundColor(255,255,255);
	
	$t->setWorld($box[0], $box[1], $box[2], $box[3], 800, 600);
	
	$t->setTheme("distritos_sp", 0);

	$result = $t->drawCurrentTheme();
	
	if($result == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
    }
    
    $box = $t->getthemebox(0, " object_id_1 = '1' ");
    
    if($box == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	$result = $t->setDefaultVisual(1, 255, 0, 0, 0, 0, "", 100, 100, 100, 0, 1);
	
	if($result == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	//$dx_ = $box[2]-$box[0];
	//$dy_ = $box[3]-$box[1];
	
	$x1 = $box[0];// - ($dx_ * 0.2);
	$y1 = $box[1];// - ($dy_ * 0.2);
	$x2 = $box[2];// + ($dx_ * 0.2);
	$y2 = $box[3];// + ($dy_ * 0.2);
	
	$result = $t->drawbox($x1, $y1, $x2, $y2);
	
	if($result == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}  
	
	$imagemSaida = $t->getCanvasImage(0, 0);
	
	if($imagemSaida == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	
	header ("Content-type: image/png");
	echo($imagemSaida);
?>