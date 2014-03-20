<?
require("css.php");
if(!extension_loaded('terraweb')) {
	dl('terraweb.' . PHP_SHLIB_SUFFIX);
}
$module = 'terraweb';
$functions = get_extension_funcs($module);
echo "Functions available in the extension:<br>\n";
foreach($functions as $func) {
    echo $func."<br>\n";
}
echo "<br>\n";
$function = 'confirm_' . $module . '_compiled';
if (extension_loaded($module)) {
	$str = $function($module);
} else {
	$str = "Module $module is not compiled into PHP";
}
echo "$str <br>\n";

require("global.inc.php");

?>
