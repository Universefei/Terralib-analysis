<? 
  /*
   * Exemplo 12: Conecta com um banco de dados que possui um layer de municípios
   *             plotando do mapa de municipios apenas os municípios com o atributo AREA_1 >= 38162.317
   *		 Ajusta a área do canvas para o box dos objetos do tema corremte com a restrição da área
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

  $result = $t->setTheme("municipios", 0);
  $t->setActiveThemeRepresentation(1, 0);
  
  if($result == false)
  {
     echo($t->errorMessage() . "<BR>");

     exit();
  }

  $box = $t->getThemeBox(0, "AREA_1 >= 38162.317");

  if($box == false)
  {
     echo($t->errorMessage() . "<BR>");
     exit();
  }

  $t->setWorld($box[0], $box[1], $box[2], $box[3], 800, 600);

  $result = $t->plotvector("AREA_1 >= 38162.317");
 
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
