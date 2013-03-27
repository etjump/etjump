<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>.: 999 Map-list :.</title>
<style type="text/css" title="currentStyle">
	@import "media/css/demo_page.css";
	@import "media/css/demo_table_jui.css";
	@import "media/examples_support/themes/smoothness/jquery-ui-1.8.4.custom.css";
</style>
<script type="text/javascript" language="javascript" src="media/js/jquery.js"></script>
<script type="text/javascript" language="javascript" src="media/js/jquery.dataTables.js"></script>
<script type="text/javascript" charset="utf-8">
$.fn.dataTableExt.oApi.fnFilterClear  = function ( oSettings )
{
    oSettings.oPreviousSearch.sSearch = "";
    if ( typeof oSettings.aanFeatures.f != 'undefined' )
    {
        var n = oSettings.aanFeatures.f;
        for ( var i=0, iLen=n.length ; i<iLen ; i++ )
        {
            $('input', n[i]).val( '' );
        }
    }
    for ( var i=0, iLen=oSettings.aoPreSearchCols.length ; i<iLen ; i++ )
    {
        oSettings.aoPreSearchCols[i].sSearch = "";
    }
    oSettings.oApi._fnReDraw( oSettings );
}

$(document).ready(function() {
    oTable = $('#example').dataTable({
        "bJQueryUI": true,
        "sPaginationType": "full_numbers",
		"iDisplayLength": 15,
		"aoColumnDefs": [
            { "bSortable": false, "aTargets": [ 5 ] }
        ]
    });
} );
</script>
<!--
Automatic map listing system made by HeX|Fate
Contact me at:
	Xfire: fmcfate 
	MSN: fmcfate[at]hotmail.com
Visit my web-page at: www.hex.ee

Back-end coded in PHP
Front-end is Jquery DataTables

Valid XHTML 1.0 Transitional: 
http://validator.w3.org/check?uri=http%3A%2F%2Fwww.etjump.com%2F999%2Fmaplist%2F
-->
</head>
<?php
function getsize($size, $precision = 2)
{
    $base = log($size) / log(1024);
    $suffixes = array('', 'KB', 'MB', 'GB', 'TB');
	$color = array('', '#736F6E', '#000000', 'GB', 'TB');   
    return round(pow(1024, $base - floor($base)), $precision) . " " . $suffixes[floor($base)];
}
?>

<body id="dt_example">
<div style="width:950px;margin:10px auto;padding:0">
<div id="demo">
<?php
function removecolor ($string){
	return preg_replace("/\^./","",$string);
}
$host = "213.108.31.54"; 
$port = 27010;
$length = 2048;
$packet = "\377\377\377\377";
$players = array();
$params = array();
$socket = socket_create (AF_INET, SOCK_DGRAM, getprotobyname ("udp"));
if ($socket) {
	socket_connect ($socket, $host, $port );
	socket_write ($socket, $packet . "getstatus\n");
	$read = array ($socket);
	
	$out = "";
	$out = socket_read ($socket, $length, PHP_BINARY_READ);
	socket_close ($socket);
	
	$out = preg_replace ("/" . $packet . "print\n/", "", $out);
	$out = preg_replace ("/" . $packet . "statusResponse\n/", "", $out);
	
	$all = explode( "\n", $out );
	$params = explode( "\\", $all[0] );
	
	array_shift( $params );
	$temp = count($params);
	for( $i = 0; $i < $temp; $i++ )
	{
		$params[ strtolower($params[$i]) ] = $params[++$i];
	}
	for( $i = 1; $i < count($all) - 1; $i++ )
	{
		$pos = strpos( $all[$i], " " );
		$score = substr( $all[$i], 0, $pos );
		$pos2 = strpos( $all[$i], " ", $pos + 1 );
		$ping = substr( $all[$i], $pos + 1, $pos2 - $pos - 1 );
		$name = substr( $all[$i], $pos2 + 2 );
		$name = substr( $name, 0, strlen( $name ) - 1);
	
		$player = array( $name, $score, $ping );
		$players[] = $player;
	}
}
else 
{
	echo "The server is DOWN!";
}
?>
<div style="width:500px">
<h1>999 Server monitor.</h1>
<div class="dataTables_wrapper" id="example_wrapper">
<div class="fg-toolbar ui-toolbar ui-widget-header ui-corner-tl ui-corner-tr ui-helper-clearfix">Server info</div>
<table cellspacing="0" cellpadding="0" border="0" class="display">
<thead>
    <tr>
    <th class="ui-state-default" rowspan="1" colspan="1">Key</th>
    <th class="ui-state-default" rowspan="1" colspan="1">Value</th>
    </tr>
</thead>
<tbody>
	<tr class="odd">
    	<td>Server IP</td>
        <td><? echo $host.":".$port; ?> (Join using <a href="xfire:join?game=wet&server=<? echo $host.":".$port; ?>">Xfire</a> or <a href="hlsw://<? echo $host.":".$port; ?>">HLSW</a>)</td>
  	</tr>
	<tr class="even">
    	<td>Server name</td>
        <td><? echo removecolor($params['sv_hostname']); ?></td>
  	</tr>
   	<tr class="odd">
    	<td>Server location</td>
        <td><? echo $params['location']; ?></td>
  	</tr>
    <tr class="even">
    	<td>Map name</td>
        <!--  (<a href="#" onclick="javascript: oTable.fnFilter('^.*$', 2, true);">Clear search</a>) -->
        <td><a href="#" onclick="javascript: oTable.fnFilter('^<? echo $params['mapname']; ?>$', 2, true);"><? echo $params['mapname']; ?></a></td>
  	</tr>
    <tr class="odd">
    	<td>Mod name</td>
        <td><? echo removecolor($params['gamename']); ?></td>
  	</tr>
    <tr class="even">
    	<td>Server capacity</td>
        <td><? echo count($players) . " / " . $params['sv_maxclients'] . " (" . $params['sv_privateclients'] . ")"; ?></td>
  	</tr>
    <tr class="odd">
    	<td>Game version</td>
        <td><? echo $params['version']; ?></td>
  	</tr>
</tbody>
</table>
<table cellspacing="0" cellpadding="0" border="0" class="display">
<thead>
    <tr>
    <th class="ui-state-default" rowspan="1" colspan="1">Player name</th>
    <th class="ui-state-default" rowspan="1" colspan="1">Ping</th>
    </tr>
</thead>
<tbody>
<?php
foreach($players as $key => $p){
	?>
    	<tr class="<?  echo ($key % 2 == 0) ? "odd" : "even"; ?>">
    	<td><? echo removecolor($p[0]); ?></td>
        <td><? echo $p[2]; ?></td>
  		</tr>
    <?
}
?>
</tbody>
</table>
<div class="fg-toolbar ui-toolbar ui-widget-header ui-corner-bl ui-corner-br ui-helper-clearfix">
<br />
</div></div></div>
<h1>999 Server auto-updating maplist.</h1>
<table cellpadding="0" cellspacing="0" border="0" class="display" id="example">
<thead>
    <tr>
        <th>#</th>
        <th>Download link</th>
        <th>Callvote name</th>
        <th>Release date</th>
        <th>Uploaded</th>
        <th>Size</th>
    </tr>
</thead>
<tfoot>
    <tr>
        <th>#</th>
        <th>Download link</th>
        <th>Callvote name</th>
        <th>Release date</th>
        <th>Uploaded</th>
        <th>Size</th>
    </tr>
</tfoot>
<tbody>
<?php
$counter = 1;
$pk3log = file("pk3.txt");
asort($pk3log);
foreach($pk3log as $line)
{
	$t = explode(";", $line);
	?>
    <tr>
    	<td><? echo $counter; ?></td>
        <td><a href="http://maps.server1.uk.ycn-hosting.com/213.108.31.54-27010/etmain/<? echo $t[1]; ?>"><? echo ucfirst($t[0]); ?></a></td>
        <td><? echo $t[2]; ?></td>
        <td><? echo $t[3]; ?></td>
        <td><? echo $t[4]; ?></td>
        <td><? echo getsize((int)$t[5]); ?></td>
  	</tr>
    <?
	$counter++;
}
?>
</tbody>
</table>
</div></div>
</body>
</html>