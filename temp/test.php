<?php
$filelist = "./list.txt";
$lineid = "11112253345";

$text = $argv[1];

if (preg_match("/^@/", $text)) {
    list($_,$boardname) = explode("@",$text);
    $lines = file($filelist);
    foreach ($lines as $line_num => $line) {
        $line = str_replace(array("\n", "\r"), '', $line);
        list($uid,$boardid) = explode(",",$line);
        $board[$uid]=$boardid;
    }
    $board[$lineid]=$boardname;
    $fh = fopen($filelist, 'w') or die("can't open file");
    foreach ($board as $id => $name) {
        $stringData = $id.",".$name."\n";
        fwrite($fh, $stringData);
    }
    fclose($fh);         
}

$lines = file($filelist);
foreach ($lines as $line_num => $line) {
    $line = str_replace(array("\n", "\r"), '', $line);
    list($uid,$boardid) = explode(",",$line);
    $board[$uid]=$boardid;
}
$id = "22134234";
echo $board[$lineid];
