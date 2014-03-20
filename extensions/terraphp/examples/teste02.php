<?
/*
	Gera o Image Map do mapa de Sao Paulo representado pelo tema distritos_sp.
	Obs.: Se voce chamar esta pagina no seu navegador, nada devera ser apresentado. No entanto,
	se voce tentar ver o código fonte da pagina, vera o mapa de imagem.
	Este suporte eh interessante quando integrado com Ajax.
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
	$t->setImagemapProperties("distritos_sp", "distritos_sp");
	$t->closeImagemap(1);
	$t->setAreaProperty("onClick", "alert(%s);", "object_id_1");
	$t->setAreaProperty("title", "%s", "deno");	
	$t->setGeneralizedPixels(30);
	$result = $t->getImagemap("", "", "");
	
	if($result == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
    }
	
	echo($result);
	
	$t->setImagemapProperties("distritos_sp2", "distritos_sp2");
	$t->closeImagemap(1);
	$t->setAreaProperty("onClick", "alert(%s);", "object_id_1");
	$t->setAreaProperty("title", "%s", "deno");	
	$result = $t->getImagemap("", "", " DENO = 'MARSILAC' ");
	
	if($result == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
    }
	
	echo($result);
	echo("Funcionou! Veja o codigo fonte desta pagina para visualizar o resultado do script!");
?>