<?
/*
	Salva o  mapa de distritos de Sao Paulo em um shape file.
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
	
	$result = $t->saveThemeToFile("/usr/local/apache/htdocs/tmp/meu_shape");
	
	if($result == false)
	{
		echo($t->errorMessage() . "sssssssssss<BR>");
		exit();
    }
    
	echo("Gravou o shape: /usr/local/apache/htdocs/tmp/meu_shape");
?>
