<? 
  /*
   * Exemplo 9: Conecta com um banco de dados que possui um layer de municípios e outro de pontos,
   *            plotando o mapa de municipios agrupado pelo atributo LATITUDESE usando o modo de agrupamento
   *            passos iguais com 5 fatias em tons de vermelho
   *            Plota em amarelo os polígonos que interagem (intersects) com um dado box mostrado em verde
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

  $result = $t->setTheme("municipios", 0);
  $t->setActiveThemeRepresentation(1, 0);
  
  if($result == false)
  {
     echo($t->errorMessage() . "<BR>");

     exit();
  }

  $kout = $t->locateObjectsInBox(-58.003751, -14.778673, -49.718428, -9.807479, 64, 0);

  if($kout == false)
  {
    echo($t->errorMessage() . "<BR>");

    exit();
  }

  $rampColors = array("RED");

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

  $result = $t->setDefaultVisual(2, 0, 255, 0, 0, 3, "");

  if($result == false)
  {
     echo($t->errorMessage() . "<BR>");

     exit();
  }

  $result = $t->plotBox(-58.003751271716, -14.778673212769, -49.718428929108, -9.8074798072041, 0);

  if($result == false)
  {
     echo($t->errorMessage() . "<BR>");

     exit();
  }


  $line[0][0] = -58.003751271716;
  $line[0][1] = -14.778673212769;
  $line[1][0] = -49.718428929108;
  $line[1][1] = -9.8074798072041;

  $result = $t->plotLine($line);

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
