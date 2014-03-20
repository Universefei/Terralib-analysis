<? 
  /*
   * Exemplo 2: Conecta-se a um banco de dados e lista todas as vistas armazenadas neste banco
   *
   */

  $t = terraweb();

  if(!$t->connect("localhost", "gribeiro", "queiroz", "gribeirodb", 3306, 1))
  {
    echo("Nao foi possivel conectar-se ao banco de dados<BR>");
    echo($t->errorMessage());
    exit();
  }
  
  $views = $t->getviews("gribeiro");

  if($views == false)
  {
    echo("Nao fo possivel recuperar as vistas para o usuário root <BR>");
    echo($t->errorMessage());
    exit();
  }

  for($j = 0; $j < count($views); ++$j)
     echo($views[$j] . "<BR>");
  
?>
