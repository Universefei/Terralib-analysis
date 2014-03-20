<?
/*
	Gera um cartograma de num_alunos x num_alunas para os distritos de Sao Paulo a partir de um SQL:
	- Cartograma de tortas proporcionais
	
	Obs.: substituir o atributo "object_id_1" pelo equivalente de seu banco de dados!
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
	
	$result = $t->drawCurrentTheme();
	
	if($result == false)
	{
		echo($t->errorMessage() . "<BR>");
		exit();
    }
	
	$fields[0] = "num_alunos";
	$fields[1] = "num_alunas";
	$fieldsFunc = "SUM";
	$colors[0] = array(0, 0, 255);
	$colors[1] = array(0, 255, 0);
	$cartParams[0] = $colors;
	$cartParams[1] = 10;	//min height
	$cartParams[2] = 40; 	//max height
	$cartParams[3] = 0;		// pie width => for proportionals = 0
	$cartParams[4] = 1;		// cartogram style = bar graph
	$proportionalToField = "tot_alunos";
	$fromClause = "( SELECT object_id_1, escolas.* FROM escolas, distritos_sp WHERE escolas.distrito = distritos_sp.deno) as r ";
	$linkAttr = "r.object_id_1";
	$restrictionExpression = "";	
	
	$t->setConflictDetect(1);
	$t->setMinCollisionTolerance(0);
	
	$result = $t->drawCartogram($fields, $fieldsFunc, $cartParams, $proportionalToField, $fromClause, $linkAttr, $restrictionExpression);
		
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