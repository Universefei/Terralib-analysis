<?
/*
	Gera o  mapa de ruas de taubate representado pelo eixos_taubate, desenha o texto da coluna "DENO" usando o campo "SPRAREA" como prioridade.
	Rever o seu campo: object_id_3
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
	
	$t->setTheme("eixos_taubate", 0);
	
	$result = $t->drawCurrentTheme();
	if($result == false)
	{
		echo($t->errorMessage() . "dddd<BR>");
		exit();
    }
	
	$t->setTextOutlineEnable(0);
	//$t->setTextOutlineColor(0, 0, 0);
	$t->setLabelField("object_id_3");
	$res = $t->setDefaultVisual(128, 0, 0, 0, 0, 12, "/mnt/win_c/WINDOWS/Fonts/arial.ttf");
	$t->setMinCollisionTolerance(0);
	$t->setConflictDetect(1);	
	$result = $t->drawText2();

	
	if($result == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
    }
	
	$imagemSaida = $t->getCanvasImage(0, 1);
	
	if($imagemSaida == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	
	header ("Content-type: image/png");
	echo($imagemSaida);
?>