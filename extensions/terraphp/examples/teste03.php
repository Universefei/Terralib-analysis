<?
/*
	Gera o  mapa de Sao Paulo representado pelo tema distritos_sp, desenha o texto da coluna "DENO" usando o campo "SPRAREA" como prioridade.
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
	
	$t->setWorld($box[0], $box[1], $box[2], $box[3], 800, 600);
	
	$t->setTheme("distritos_sp", 0);
	
	$t->setTextOutlineEnable(1);
	$t->setTextOutlineColor(255, 255, 255);
	$t->setLabelField("deno");
	$t->setPriorityField("sprarea");
	$res = $t->setDefaultVisual(128, 100, 100, 100, 0, 2, "");
	$t->setMinCollisionTolerance(10);
	$t->setConflictDetect(1);	
	
	$result = $t->drawCurrentTheme();
	
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