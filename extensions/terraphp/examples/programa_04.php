<? 
  /*
   * Exemplo 3: Conecta com um banco de dados e plota um layer de imagem
   * 
   * 
   */


  $t = terraweb();

  if(!$t->connect("localhost", "gribeiro", "queiroz", "gribeirodb", 3306, 1))
  {
    echo("Nao foi possivel conectar-se ao banco de dados<BR>");
    echo($t->errorMessage());
    exit();
  }
  
  if($t->setCurrentView("NATIVIDADE") == false)
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

     $result = $t->plotraster();
 
     if($result == false)
     {
	echo($t->errorMessage() . "<BR>");

	exit();
     }	
  }

  $imagemSaida = $t->getjpg();
  header ("Content-type: image/jpg");
  echo($imagemSaida);
?>
