<html>
<head><title>configfile class test</title>
<?php
  require ("css.php");
?>
</head>
<body>
<?php
import_request_variables("gpc");
require ("configfile.class.php");
// give full authority (can add/delete vars and edit comments)
//$fconfig = new ConfigFile("vars.inc.php",true,true,true);
//this would let only change the value of the variables
$fconfig = new ConfigFile("vars.inc.php");
?>
</body>
</html>
