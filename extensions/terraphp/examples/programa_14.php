<? 
  /*
   * Exemplo 13: Conecta com um banco de dados que possui um layer de municípios
   *             e escreve sobre o mapa   
   */

  $t = terraweb();

  if(!$t->connect("localhost", "gribeiro", "queiroz", "gribeirodb", 3306, 1))
  {
    echo("Nao foi possivel conectar-se ao banco de dados<BR>");
    echo($t->errorMessage());
    exit();
  }
  
  if($t->setCurrentView("BRASIL") == false)
  {
    echo($t->errorMessage() . "<BR>");
    exit();
  }  

  $newWorldBox = $t->setWorld(-59.912893594101, -13.598146710832, -40.977835330074, -2.7220339762508, 800, 600);

  $result = $t->setTheme("municipios", 0);
  
  if($result == false)
  {
     echo($t->errorMessage() . "<BR>");

     exit();
  } 

  $result = $t->plotvector();
 
  if($result == false)
  {
    echo($t->errorMessage() . "<BR>");

    exit();
  }

  $x = $newWorldBox[0];
  $y = $newWorldBox[1] + (($newWorldBox[3] - $newWorldBox[1]) / 8);
	
  $res = $t->setDefaultVisual(128, 0, 0, 0, 0, 0, "");

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $res = $t->plotText($x, $y, "ESCREVENDO SOBRE O CANVAS - FONTE DEFAULT TAMANHO 0", 0.0, 0.5, 0.5);

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $y = $newWorldBox[1] + 2 * (($newWorldBox[3] - $newWorldBox[1]) / 8);
	
  $res = $t->setDefaultVisual(128, 0, 0, 0, 0, 1, "");

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $res = $t->plotText($x, $y, "ESCREVENDO SOBRE O CANVAS - FONTE DEFAULT TAMANHO 1", 0.0, 0.5, 0.5);

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $y = $newWorldBox[1] + 3 * (($newWorldBox[3] - $newWorldBox[1]) / 8);
	
  $res = $t->setDefaultVisual(128, 0, 0, 0, 0, 2, "");

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $res = $t->plotText($x, $y, "ESCREVENDO SOBRE O CANVAS - FONTE DEFAULT TAMANHO 2", 0.0, 0.5, 0.5);

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $y = $newWorldBox[1] + 4 * (($newWorldBox[3] - $newWorldBox[1]) / 8);
	
  $res = $t->setDefaultVisual(128, 0, 0, 0, 0, 3, "");

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $res = $t->plotText($x, $y, "ESCREVENDO SOBRE O CANVAS - FONTE DEFAULT TAMANHO 3", 0.0, 0.5, 0.5);

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $y = $newWorldBox[1] + 5 * (($newWorldBox[3] - $newWorldBox[1]) / 8);
	
  $res = $t->setDefaultVisual(128, 0, 0, 0, 0, 4, "");

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $res = $t->plotText($x, $y, "ESCREVENDO SOBRE O CANVAS - FONTE DEFAULT TAMANHO 4", 0.0, 0.5, 0.5);

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $y = $newWorldBox[1] + 6 * (($newWorldBox[3] - $newWorldBox[1]) / 8);
	
  $res = $t->setDefaultVisual(128, 0, 0, 0, 0, 24, "/mnt/windows/windows/fonts/trebucbi.ttf");

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $res = $t->plotText($x, $y, "ESCREVENDO SOBRE O CANVAS - FONTE TRUE TYPE", 0.0, 0.5, 0.5);

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $imagemSaida = $t->getpng();

  if($imagemSaida == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  header ("Content-type: image/png");
  echo($imagemSaida);
?>
