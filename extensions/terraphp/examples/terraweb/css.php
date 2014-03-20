<?php
require("browser.php");

function css_site() {
$site_fonts='Arial, Verdana, Helvetica, sans-serif';

	//determine font for this platform
	if (browser_is_windows() && browser_is_ie()) {

		//ie needs smaller fonts
		$font_size='x-small';
		$font_smaller='xx-small';
		$font_smallest='7pt';
	} else if (browser_is_windows()) {

		//netscape on wintel
		$font_size='small';
       	$font_smaller='7pt';
		$font_smallest='x-small';
	} else if (browser_is_mac()){

		//mac users need bigger fonts
		$font_size='medium';
		$font_smaller='small';
		$font_smallest='x-small';
	} else if (browser_is_mozilla_linux()){

		//mozilla on linux users need bigger fonts
		$font_size='small';
		$font_smaller='small';
		$font_smallest='x-small';
	} else {

		//linux and other users
		$font_size='small';
		$font_smaller='x-small';
		$font_smallest='x-small';
	}
	?>
<STYLE TYPE="text/css">
<!--
BODY, OL, UL, LI { font-family: <?=$site_fonts; ?>; font-size: <?=$font_smaller; ?>; background-color: #EEEEEE;}
form { font-family: <?php echo $site_fonts; ?>; font-size: <?php echo $font_size; ?>}
CAPTION { font-family: <?php echo $site_fonts; ?>; font-size: <?php echo $font_size; ?>;font-weight: bold; background-color: #BBBBBB; color : #004477 }
TH { font-family: <?php echo $site_fonts; ?>; font-size: <?php echo $font_size; ?>;font-weight: bold; background-color: #BBBBBB; color : #004477 }
TD { font-family: <?php echo $site_fonts; ?>; color : #006699; font-size: <?php echo $font_smaller; ?>;font-weight: bold; background-color: #DDDDDD; height: 10px;align : "right";}
H1 { font-size: 175%; font-family: <?php echo $site_fonts; ?>; color : #006699}
H2 { font-size: 150%; font-family: <?php echo $site_fonts; ?>; color : #006699 }
H3 { font-size: 125%; font-family: <?php echo $site_fonts; ?>; color : #006699 }
H4 { font-size: 100%; font-family: <?php echo $site_fonts; ?>; color : #006699 }
H5 { font-size: 75%; font-family: <?php echo $site_fonts; ?>; color : #006699 }
H6 { font-size: 50%; font-family: <?php echo $site_fonts; ?>; color : #006699 }
PRE, TT, CODE { font-family: courier, sans-serif; font-size: <?php echo $font_size; ?>; }
select {font-size:<?=$font_smallest; ?>;font-family:<?=$site_fonts; ?>;background-color:#F7F7F7;border:#944100;border-style:solid;border-top-width:1px;border-right-width:1px;border-bottom-width:1px;border-left-width:1px}
input {font-size:<?=$font_smallest; ?>;font-family:<?=$site_fonts; ?>;background-color:#F7F7F7;border:#944100;border-style:solid;border-top-width:1px;border-right-width:1px;border-bottom-width:1px;border-left-width:1px}
A:hover { text-decoration: none; color: #FF6666; font-size: <?php echo $font_smaller; ?>; }
A.menus { color: #FF6666; text-decoration: none; font-size: <?php echo $font_smaller; ?>; }
A.menus:visited { color: #FF6666; text-decoration: none; font-size: <?php echo $font_smaller; ?>; }
A.menus:hover { text-decoration: none; color: #FF6666; background: #ffa; font-size: <?php echo $font_smaller; ?>; }
A.menussel { color: #FF6666; text-decoration: none; background: #ffa; font-size: <?php echo $font_smaller; ?>; }
A.menussel:visited { color: #FF6666; text-decoration: none; background: #ffa; font-size: <?php echo $font_smaller; ?>; }
A.menussel:hover { text-decoration: none; color: #FF6666; background: #ffa; font-size: <?php echo $font_smaller; ?>; }
A.menusxxs { color: #FF6666; text-decoration: none; font-size: <?php echo $font_smallest; ?>; }
A.menusxxs:visited { color: #FF6666; text-decoration: none; font-size: <?php echo $font_smallest; ?>; }
A.menusxxs:hover { text-decoration: none; color: #FF6666; background: #ffa; font-size: <?php echo $font_smallest; ?>; }
.azul {font-size : <?=$font_size?>; color : #006699; font-family : Arial; }
.vermelho {color : #FF0000;}
.verde {color : #00FF00;}
.ciano {color : #00FFFF;}
.azul {color : #0000FF;}
.preto {color : #000000;}
.black {font-size : <?=$font_smallest?>; color : #000000; font-family : Arial; }
-->
</STYLE>
<?php
}
css_site();
?>
