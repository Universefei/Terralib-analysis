<? 
  /*
   * Exemplo 2: Conecta-se com um banco de dados e lista para cada vista armazenada no banco,
   *            o seu retângulo envolvente e os temas contidos nela
   *
   */

  $t = terraweb();

  if(!$t->connect("localhost", "gribeiro", "queiroz", "gribeirodb", 3306, 1))
  {
    echo("Nao foi possivel conectar-se ao banco de dados<BR>");
    echo($t->errorMessage());
    exit();
  }
  
  $views = $t->getviews();

  if($views == false)
  {
    echo("Nao fo possivel recuperar as vistas para o usuário root <BR>");
    echo($t->errorMessage());
    exit();
  }

  for($j = 0; $j < count($views); ++$j)
  {
     echo("==> VISTA: " . $views[$j] . "<BR>");

     if($t->setCurrentView($views[$j]) == false)
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

     echo("BOX: X1 = " . $box[0] . " , Y1 = " . $box[1] . ", X2 = ". $box[2] . ", Y2 = " . $box[3] . "<BR>");

     $themes = $t->getthemes();

     if($themes == false)
     {
	echo($t->errorMessage() . "<BR>");

	exit();
     }

     for($i = 0; $i < count($themes); ++$i)
     {
        echo("    ---Tema: " . $themes[$i] . "<BR>");
     }

     echo("<BR>");
  }  
?>
