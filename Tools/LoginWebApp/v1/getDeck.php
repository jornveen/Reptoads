<?php

include_once $_SERVER['DOCUMENT_ROOT'] . "/includes/connect.php";

if(!isset($_POST['projectId'])) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "projectId not specified"), JSON_NUMERIC_CHECK));
}
$projectId = intval($mysqli->real_escape_string($_POST['projectId']));

$projectResult = $mysqli->query("SELECT * FROM `projects` WHERE `id`=$projectId;");
if($projectResult->num_rows <= 0) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "Invalid project"), JSON_NUMERIC_CHECK));
}

if(!isset($_POST['token'])) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "token not specified"), JSON_NUMERIC_CHECK));
}
$token = $mysqli->real_escape_string($_POST['token']);

$sessionResult = $mysqli->query("SELECT * FROM `sessions` WHERE `token`='$token';");
if($sessionResult->num_rows <= 0) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "Invalid session"), JSON_NUMERIC_CHECK));
}

if(!isset($_POST['profileId'])) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "profileId not specified"), JSON_NUMERIC_CHECK));
}
$profileId = intval($mysqli->real_escape_string($_POST['profileId']));

$profileResult = $mysqli->query("SELECT * FROM `profiles` WHERE `id`=$profileId;");
if($profileResult->num_rows <= 0) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "Invalid profile"), JSON_NUMERIC_CHECK));
}
$profile = $profileResult->fetch_assoc();

$decks = array();
{
	$decksResult = $mysqli->query("SELECT * FROM `decks` WHERE `projectId`=$projectId AND `profileId`=" . $profile['id'] . ";");
	while($row = $decksResult->fetch_assoc()) {
		$row['cards'] = array();
		$deckRelationResult = $mysqli->query("SELECT * FROM `deckCardRelation` WHERE `deckId`=" . $row['id']);
		while($relationRow = $deckRelationResult->fetch_assoc()) {
			$row['cards'][] = $relationRow['cardId'];
		}
		$decks[] = $row;
	}
}

header("Content-Type: application/json");
exit(json_encode(array("valid" => true, "data" => $decks), JSON_NUMERIC_CHECK));
