<? 
  /*
   * Exemplo 10: Conecta com um banco de dados que possui um layer de municípios
   *             plotando o mapa de municipios
   *             restringindo o box de visualização.
   *             Plota em vermelho uma linha contendo o buffer do objeto (polígono) 1151
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

  $t->setWorld(-59.912893594101, -13.598146710832, -40.977835330074, -2.7220339762508, 800, 600);

  $result = $t->setTheme("municipios", 0);
  $t->setActiveThemeRepresentation(1, 0);
  
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

  $result = $t->setDefaultVisual(2, 255, 0, 0, 0, 1, "");

  if($result == false)
  {
     echo($t->errorMessage() . "<BR>");

     exit();
  }

  $result = $t->plotBufferZone("1151", 0.2, 0);

  if($result == false)
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
