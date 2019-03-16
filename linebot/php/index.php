<?php
/* Maxboard */
 error_reporting(0);
 
// Line access toke
$access_token = "84ah/gtpG/bWLWtDU3COD+6YDnfEy7of4Zy/7QuFCrY/fNyEfYPprP4lsOMK0hXULqsXcqXztXcMDsfFEwrx5jX3KelVvz1iwFvj+CXzMtiGlVUmbYaP+PuAJyva2KpDdM9ZoNXgom7x0PyMB6p8YQdB04t89/1O/w1cDnyilFU=
Issue";


$filelist = "./boards.txt";

// Get POST body content
$content = file_get_contents('php://input');
// Parse JSON
$events = json_decode($content, true);
// Validate parsed JSON data
if (!is_null($events['events'])) {
	// Loop through each event
	foreach ($events['events'] as $event) {
		// Reply only when message sent is in 'text' format
		if ($event['type'] == 'message' && $event['message']['type'] == 'text') {
			// Get text sent
			$text = $event['message']['text'];
            $userId = $event['source']['userId'];
            
			if (preg_match("/^@/", $text)) {
				list($_,$boardname) = explode("@",$text);
				$lines = file($filelist);
				foreach ($lines as $line_num => $line) {
					$line = str_replace(array("\n", "\r"), '', $line);
					list($uid,$boardid) = explode(",",$line);
					$board[$uid]=$boardid;
				}
				$board[$userId]=$boardname;
				$fh = fopen($filelist, 'w') or die("can't open file");
				foreach ($board as $id => $name) {
					if ($id) {
						$stringData = $id.",".$name."\n";
						fwrite($fh, $stringData);
					}
				}
				fclose($fh);         
			}
			else if ($text == "?"){
				$text = "Help:\n";
				$text .= "@<boardname> = define board\n";
				$text .= "#<time ms> = delay scroll text\n";
				$text .= "!<text> = no scroll text\n";
				$text .= "><text> = pacman\n";
				$text .= "clr = clear\n";
				$text .= "? = Help\n";
								
			}
			else {
				$lines = file($filelist);
				foreach ($lines as $line_num => $line) {
					$line = str_replace(array("\n", "\r"), '', $line);
					list($uid,$boardid) = explode(",",$line);
					$board[$uid]=$boardid;
					//if (!file_exists($boardid.".html")){
						//unlink($boardid.".html");
					//}
				}

				$filename = $board[$userId].".html";
				$myfile = fopen($filename, "w") or die("Unable to open file!");
				fwrite($myfile, $text);
				fclose($myfile);

				$text = "@".$board[$userId].":".$text;
			}			
			
			// Get replyToken
			$replyToken = $event['replyToken'];

			// Build message to reply back
			$messages = [
				'type' => 'text',
				'text' => $text
			];

			// Make a POST Request to Messaging API to reply to sender
			$url = 'https://api.line.me/v2/bot/message/reply';
			$data = [
				'replyToken' => $replyToken,
				'messages' => [$messages],
			];
			$post = json_encode($data);
			$headers = array('Content-Type: application/json', 'Authorization: Bearer ' . $access_token);

			$ch = curl_init($url);
			curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");
			curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
			curl_setopt($ch, CURLOPT_POSTFIELDS, $post);
			curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
			curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
			$result = curl_exec($ch);
			curl_close($ch);

			echo $result . "\r\n";
		}
	}
}
echo "OK";