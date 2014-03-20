<? 
  /*
   * Exemplo 8: Conecta com um banco de dados que possui um layer de municípios e outro de pontos,
   *            plotando o mapa de municipios agrupado pelo atributo LATITUDESE usando o modo de agrupamento
   *            passos iguais com 5 fatias, e o plota o layer de pontos de forma normal.
   *            restringindo o box de visualização.
   *            Plota em amarelo o polígono que contém o objeto (ponto) 1210
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

  $t->setWorld(-59.912893594101, -13.598146710832, -40.977835330074, -2.7220339762508, 800, 600);

  $result = $t->setTheme("desmat2004", 0);
  $t->setActiveThemeRepresentation(4, 0);
  
  if($result == false)
  {
     echo($t->errorMessage() . "<BR>");

     exit();
  }

  $result = $t->setTheme("municipios", 1);
  $t->setActiveThemeRepresentation(1, 1);
  
  if($result == false)
  {
     echo($t->errorMessage() . "<BR>");

     exit();
  }

  $kout = $t->locateObjectsUsingRefTheme("64", 32);

  if($kout == false)
  {
    echo($t->errorMessage() . "<BR>");

    exit();
  }

  $result = $t->setTheme("municipios", 0);
  
  if($result == false)
  {
     echo($t->errorMessage() . "<BR>");

     exit();
  }

  $rampColors = array("RED", "GREEN", "BLUE");

  $result = $t->plotvector("", "LATITUDESE", "", 5, 0, $rampColors, 3, 0);
 
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

  $result = $t->plotSelectedObjects($kout);

  if($result == false)
  {
     echo($t->errorMessage() . "<BR>");

     exit();
  }

  $result = $t->setTheme("desmat2004", 0);
  
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

  $result = $t->setDefaultVisual(4, 0, 255, 0, 4, 3, "");

  if($result == false)
  {
     echo($t->errorMessage() . "<BR>");

     exit();
  }

  $kout2[0] = 64;

  $result = $t->plotSelectedObjects($kout2);

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
