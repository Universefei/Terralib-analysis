<? 
  /*
   * Exemplo 6: Conecta com um banco de dados que possui um layer de municípios (poligonos), 
   *            um de desmatamento (pontos) e outros de hidrografia (linhas),
   *            plotando o mapa de municipios agrupado pelo atributo LATITUDESE usando o modo de agrupamento
   *            passos iguais com 5 fatias, plota o layer de pontos e linhas de forma normal
   *             e então plota a legenda. Os objetos plotados serão restritos a um determinado box.
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
  
  $legenda["municipios - Passos Iguais (LATITUDESE)"] = $result;

  $result = $t->setTheme("RiosPermanentesBrasil", 0);
  
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
  
  $legenda["Rios Permanentes Brasil"] = $result;

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

  $legenda["desmatamento 2004"] = $result;

  $res = $t->setDefaultVisual(128, 0, 0, 0, 0, 2, "");

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $res = $t->plotLegend($legenda, 350);

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }
 
  $imagemSaida = $t->getlegendpng();

  if($imagemSaida == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  header ("Content-type: image/png");
  echo($imagemSaida);
?>
