<?php

$defaultDeckProfileId = 17;

if(!isset($_POST['username']) || !isset($_POST['password']) || !isset($_POST['email'])) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "Some values not provided.")));
}

include_once $_SERVER['DOCUMENT_ROOT'] . "/includes/connect.php";

$username = $mysqli->real_escape_string($_POST['username']);
$password = hash('sha256', $_POST['password']);
$email = $mysqli->real_escape_string($_POST['email']);

$existingResult = $mysqli->query("SELECT * FROM `profiles` WHERE `username`='$username';");
if($existingResult->num_rows > 0) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "Username in use", "field" => "username")));
}

$existingResult = $mysqli->query("SELECT * FROM `profiles` WHERE `email`='$email';");
if($existingResult->num_rows > 0) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "Email in use", "field" => "email")));
}

$mysqli->query("INSERT INTO `profiles` (`username`, `password`, `email`) VALUES ('$username', '$password', '$email');");
$profileId = $mysqli->insert_id;

$mysqli->query("INSERT INTO `profileCardRelation` SELECT * FROM `profileCardRelation` WHERE `profileId`=$defaultDeckProfileId;");

$decksResult = $mysqli->query("SELECT * FROM `decks` WHERE `profileId`=$defaultDeckProfileId;");
while($deck = $decksResult->fetch_assoc()) {
	$mysqli->query("INSERT INTO `decks` (`profileId`, `projectId`, `name`) VALUES ($profileId, " . $deck['projectId'] . ", '" . $deck['name'] . "');");
	$deckId = $mysqli->insert_id;
	$mysqli->query("INSERT INTO `deckCardRelation` (`deckId`, `cardId`) SELECT $deckId, `cardId` FROM `deckCardRelation` WHERE `deckId`=$deckId;");
}

exit(json_encode(array("valid" => true)));
