<?php
/*
=======================================================================
		This file contains 
		
		Class CONFIGFILE V 0.1.2.
		-------------------------
Utility and use:
	This class is used to edit a config file containing variables.
	Used to shorten developpement time of admin interface.

Functions :
	* ConfigFile($fname)									- constructor of the class and accepts the name of 
																the config file
	* Initialize()											- Initialize the class, oepn the file and fill the array
	* Auto()												- Automatically handles the form
	* OutputForm()											- Output the editing form
	* SaveToFile()											- save the variables in the file
	* SaveForm()											- Takes all the new values from the POST of our form


-------------------------------
=	PROGRAMMER:
=		Marc Charbonneau
=		marccharbonneau001@yahoo.com
=		22-04-2002  01:25
=		License : GNU General Public License (GPL)
=				see http://www.opensource.org/licenses/gpl-license.html
================================================================================
* 
* Usage sample (html)
* -------------------
* <html>
*  <head><title>configfile class test</title></head>
* <body>
* <?php
*   require ("configfile.class.php");
*   $fconfig = new ConfigFile("vars.inc.php");
* ?>
* </body>
* </html>
* 
* Notes : the file must exist. It should be in the format like below
* <?php
* 	$DBHOST = "127.0.0.1";	// address of the database host
* 	$DBNAME = "sample";	// name of the database
* ?>
*/

class ConfigFile
{

  var $fname;  				// name of the config file
  var $lasterrormessage; 	// contains the last error message
  var $varlist;				// an array of the variables available in the file
  var $varlistcomment;		// an array of the comments on variables in the file
  var $varcount;			// number of variables
  var $candelvar;			// if user can delete a variable from the file
  var $canaddvar;			// if user can add a variable to the file
  var $caneditcomment;		// if user can edit the comment of the variable

  /******************************************************************
  * constructor of the class, accept the name of the config file    *
  * $CanDelVar		= if user can delete a variable from the file   *
  * $CanAddVar		= if user can add a variable to the file        *
  * $CanEditComment	= if user can edit the comment of the variable  *
  *******************************************************************/
  function ConfigFile ($name,$CanDelVar = false,$CanAddVar = false,$CanEditComment = false)
  {
	$this->lasterrormessage = "";
	$this->fname = $name;
	$this->candelvar = $CanDelVar;
	$this->canaddvar = $CanAddVar;
	$this->caneditcomment = $CanEditComment;
	$this->varcount = 0;
	if(!$this->Initialize())
	{
	  echo "Error initializing : " . $this->lasterrormessage . "<br>\n";
	  exit;
	}
	$this->Auto();
  }

  /******************************************************************
  * read the file and build the array                               *
  *******************************************************************/
  function Initialize ()
  {
    // check that file exist
    if(!file_exists($this->fname))
	{
	  $this->lasterrormessage = "File does not exist : " . $this->fname;
	  return false;
	}
	// open file in read mode
	$fp = fopen ($this->fname, "r");
	if(!$fp)
	{
	  $this->lasterrormessage = "Unable to open file";
	  return false;
	}
	// create the arrays
	$this->varlist = array();
	$this->varlistcomment = array();
	
	// parse the file line by line
	while (!feof ($fp)) 
	{
      $tmpline = fgets($fp, 4096);  //read a line, 4k max
      $posbegin = strpos($tmpline, "$");
      if (!$posbegin === false) // needs PHP 4.0b3 and newer: note three equal signs
	  { // this line may have a value
	    // check if we find the "=" sign at a higher position than "$"
		$posmiddle = strpos($tmpline, "=");
		if(($posmiddle) and ($posmiddle > $posbegin))
		{
		  // check if we find the ";" sign at a higher position than "="
		  $posend = strpos($tmpline, ";");
		  if(($posend) and ($posend > $posmiddle))
		  {
		    // everything looks ok, grab the variable and value
			$varname = substr($tmpline, $posbegin, $posmiddle - $posbegin);
			$varvalue = substr($tmpline, $posmiddle + 1, $posend - $posmiddle - 1);
			// removes any "
			$varvalue = ereg_replace("\"", "", $varvalue);
			// trim any blank char before or after variable name and value
			$varname = trim($varname);
			$varvalue = trim($varvalue);
			// add it to the array
			$this->varlist[$varname] = $varvalue;
			// check if there is a comment after the variable
			$poscomment = strpos($tmpline, "//");
            if (!$poscomment === false) // needs PHP 4.0b3 and newer: note three equal signs
			{ // there is one, grab it and put it in the "varlistcomment" array
			  $this->varlistcomment[$varname] = trim(substr($tmpline, $poscomment+2));
			}
		  }
		}
	    
	  }
	}
	// close the file
	fclose($fp);
	// update number of variables
	$this->varcount = count($this->varlist);
	//re-order the array, using key
	ksort ($this->varlist);
	return true;
  }
  
  /******************************************************************
  * Output the editing form                                         *
  *******************************************************************/
  function OutputForm ()
  {
//    echo "<center><font size=4>Fichier de configuration : " . $this->fname . "</font></center><br>\n";
//    echo "<center><font size=4>Nombre de variables dans le fichier : " . $this->varcount . "</font></center><br>\n<hr>\n";
	echo "<form action=\"" . $GLOBALS["PHP_SELF"] . "\" method=post>\n";
	echo "<table align=center border=1 width=\"600\">\n<tbody>\n";
	echo "<tr align=center>\n";
	echo "<th width=\"20%\">Vari&aacute;vel</TH>\n";
	echo "<th width=\"37%\">Valor</TH>\n";
	echo "<th width=\"37%\">Coment&aacute;rio</TH>\n";
	if($this->candelvar)
	{
	  echo "<th width=\"6%\">Del</TH>\n";
	}
	echo "</tr>\n";
	
	// loop that produce the editing rows
	reset ($this->varlist);
	foreach ($this->varlist as $key => $value)
	{
	  echo "<tr>\n<td align=\"right\" vAlign=\"bottom\"><b>" . $key . " : </b></td>\n";
	  echo "<td vAlign=\"bottom\"><input maxLength=\"50\" name=\"" . $key . "\" size=\"50\" value=\"$value\" class=\"inputbox\"></td>\n";
	  echo "<td vAlign=\"bottom\">";
	  if($this->caneditcomment)
	  {
	    echo "<input maxLength=\"50\" name=\"" . $key . "comment\"" . " size=\"50\" value=\"" . $this->varlistcomment[$key] . "\" class=\"inputbox\">";
	  }
	  else
	  {
	    echo "<nobr>" . $this->varlistcomment[$key] . "</nobr>\n"
		   . "<input type=\"hidden\" name=\"" . $key ."comment\" value=\"" . $this->varlistcomment[$key] . "\"";
	  }
	  
	  echo "</td>\n";
 	  if($this->candelvar)
	  {
	    echo "<td vAlign=bottom width=\"6%\">\n<a href=\"" . $GLOBALS["PHP_SELF"] . "?action=del&vname=" . $key . "\">X</a></td>\n</tr>\n";
	  }
	}
	echo "<tr><td align=center vAlign=bottom colspan=4><input type=\"submit\" name=\"submit\" value=\"Update\" class=\"inputbox\"></td></tr>";
	echo "</tbody>\n</table>\n";
	echo "</form>\n<hr>\n";

	if($this->canaddvar)
	{
	  // adding a variable
	  echo "<center><font size=3><b>New variable</b></font></center>";
	  echo "<form action=\"" . $GLOBALS["PHP_SELF"] . "?action=add\" method=post>";	
	  echo "<table align=center border=1 width=\"600\">\n<tbody>\n";
	  echo "<tr align=center>\n";
	  echo "<th width=\"20%\">Name</TH>\n";
	  echo "<th width=\"40%\">Value</TH>\n";
	  echo "<th width=\"40%\">Comment</TH>\n";
	  echo "</tr>\n";
      echo "<tr>\n<td align=right vAlign=bottom width=\"20%\"><input maxLength=\"50\" name=\"NewVarName\" size=\"30\" value=\"\" class=\"inputbox\"></td>\n";
	  echo "<td vAlign=bottom width=\"40%\"><input maxLength=\"50\" name=\"NewVarValue\" size=\"50\" value=\"\" class=\"inputbox\"></td>\n";
	  echo "<td vAlign=bottom width=\"40%\"><input maxLength=\"50\" name=\"NewVarComment\" size=\"50\" value=\"\" class=\"inputbox\"></td>\n</tr>\n";
	  echo "<tr><td align=center vAlign=bottom colspan=3><input type=\"submit\" name=\"submit\" value=\"Add\" class=\"inputbox\"></td></tr>";
	  echo "</tbody>\n</table>\n";
	  echo "</form>\n<hr>\n";
	}
	return true;
  }
  
  /******************************************************************
  * Save the values posted                                          *
  *******************************************************************/
  function SaveForm ()
  {
	// position to the beginning of the list
	reset($this->varlist);
	while (list($key, $value) = each ($this->varlist))
	{
	  $this->varlist[$key] = $GLOBALS[$key];
	  $this->varlistcomment[$key] = $GLOBALS[$key."comment"];
	}
    // save the file
	if(!$this->SaveToFile())
	{
	  return false;
	}
    return true;
  }
  
  /******************************
  *  Save the vars in the file  *
  ******************************/
  function SaveToFile ()
  {
	// Open the file in write mode
	$fp = fopen ("temp.conf", "w");
	if(!$fp)
	{
	  $this->lasterrormessage = "Unable to create temp file";
	  return false;
	}
	// write the beginning of the file
	fwrite($fp, "<?\n", 4);
	//re-order the array, sorting by key
	ksort ($this->varlist);
	// position to the beginning of the list
	reset($this->varlist);
	// loop writing variables with value and comment
	while (list($key, $value) = each ($this->varlist))
	{
	  if(is_numeric($value))
	  {  // it's a numeric value, don't put " around-it
	    $tmpstr = "\t" . $key . " = " . $value . ";\t//" . $this->varlistcomment[$key] . "\n";
	  }
	  else
	  {
	    $tmpstr = "\t" . $key . " = \"" . $value . "\";\t//" . $this->varlistcomment[$key] . "\n";
	  }
	  fwrite($fp, $tmpstr, strlen($tmpstr));
	}
	// write the end of the file
	fwrite($fp, "?>\n", 4);
	// now rename the file over the real one (means basically no delay without config file)
	if(!rename("temp.conf", $this->fname))
	{
	  $this->lasterrormessage = "Unable to rename temp file";
	  return false;
	}
	return true;
  }
  
  /*********************************
  *  Handle the form automaticaly  *
  *********************************/
function Auto ()
{
//if(isset($GLOBALS["REQUEST_METHOD"]))
//echo "<center>Auto ".$GLOBALS["REQUEST_METHOD"]." XXX ".$GLOBALS["action"]."</center><br><hr>\n";
	if((isset($GLOBALS["REQUEST_METHOD"])) && ($GLOBALS["REQUEST_METHOD"] == "POST"))
	{
		if(isset($GLOBALS["action"]))
		{  //action defined
			if($GLOBALS["action"] == "add")
			{
				$varname = $GLOBALS["NewVarName"];
				$varvalue = $GLOBALS["NewVarValue"];
				$varcomment = $GLOBALS["NewVarComment"];
				// removes any " in the value
				$varvalue = ereg_replace("\"", "", $varvalue);
				// trim any blank char before or after variable name, value and comment
				$varname = trim($varname);
				$varvalue = trim($varvalue);
				$varcomment = trim($varcomment);
				// make shure the name start with a dollar sign
				if(!(substr($varname,0,1) == "$"))
				{
				$varname = "$" . $varname;
				}
				// add it to the array
				$this->varlist[$varname] = $varvalue;
				$this->varlistcomment[$varname] = $varcomment;
				// update number of variables
				$this->varcount = count($this->varlist);
				// save the new variable in the file
				if(!$this->SaveToFile())
				{
					echo "<center>Error : " . $this->lasterrormessage . "</center><br><hr>\n";
				}
				else
				{
					echo "<center>Added \"" . $varname . " = " . $varvalue . ";  //" . $varcomment . "\" to the file</center><br><hr>\n";
				}
			}
			else
			{
				echo "<center>Unknown action, nothing done</center><br>\n";
			}
		}
		else
		{//just an update of the values in the file
			if(!$this->SaveForm())
			{
				echo "<center>Error : " . $this->lasterrormessage . "</center><br><hr>\n";
			}
			else
			{ // output action performed
				echo "<center>Updated the values in the file</center><br><hr>\n";
			}
		}
	}
	elseif((isset($GLOBALS["action"])) && ($GLOBALS["action"] == "del"))
	{
		// build an array with the one we want to delete
		$tmpkey = $GLOBALS["vname"];
		$tmpval = $this->varlist[$tmpkey];
		$delarray = array($tmpkey=>$tmpval);
		// make a diff with the existing one
		$this->varlist = array_diff ($this->varlist, $delarray);
		// output action performed
		echo "<center>Deleted \"" . $tmpkey . "\" from the file</center><br><hr>\n";
		//now save the file
		if(!$this->SaveToFile())
		{
			echo "<center>Error : " . $this->lasterrormessage . "</center><br><hr>\n";
		}
	}
	if(!$this->OutputForm())
	{
		echo "Unable to produce the form<br>\n";
		echo "Error : " . $this->lasterrormessage . "<br>\n";
		exit;
	}
}
}

?>
