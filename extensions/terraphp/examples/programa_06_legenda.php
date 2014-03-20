<? 
  /*
   * Exemplo 5: Conecta com um banco de dados e mostra a legenda do tema municípios
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

  $res = $t->setDefaultVisual(128, 0, 0, 0, 0, 2, "");

  if($res == false)
  {
	echo($t->errorMessage() . "<BR>");

	exit();
  }

  $legenda["municipios(Passos Iguais - LATITUDESE)"] = $result;

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
