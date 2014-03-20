<? 
  /*
   * Exemplo 4: Conecta com um banco de dados e mostra o conteúdo da vista BRASIL
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

  $box = $t->getcurrentviewbox();

  if($box == false)
  {
     echo($t->errorMessage() . "<BR>");
     exit();
  }

  $t->setWorld($box[0], $box[1], $box[2], $box[3], 800, 600);

  $themes = $t->getthemes();

  if($themes == false)
  {
    echo($t->errorMessage() . "<BR>");

    exit();
  }

  for($i = 0; $i < count($themes); ++$i)
  {
     $t->setTheme($themes[$i], 0);

     $result = $t->plotvector();
 
     if($result == false)
     {
	echo($t->errorMessage() . "<BR>");

	exit();
     }	
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
