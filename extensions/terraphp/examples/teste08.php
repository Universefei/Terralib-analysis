<?
/*
	Desenha o mapa de distritos de Sao Paulo representado pelo tema distritos_sp e destaca um objeto em particular.
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
	
	$t->setTheme("distritos_sp", 0);
	
	$result = $t->drawCurrentTheme();
	
	if($result == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
    }
	
	$result = $t->setDefaultVisual(1, 210, 210, 0, 1, 0, "", 100, 100, 100, 0, 1);
	
	if($result == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
	}
	
	$kout[0] = "0";
	$result = $t->drawSelectedObjects($kout);
	
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