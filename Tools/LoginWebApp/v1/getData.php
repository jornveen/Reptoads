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

$cards = array();
{
	$cardsResult = $mysqli->query("SELECT * FROM `cards`;");
	while($row = $cardsResult->fetch_assoc()) {
		$cards[] = $row;
	}
}

$cardRarity = array();
{
	$cardRarityResult = $mysqli->query("SELECT * FROM `cardRarity`;");
	while($row = $cardRarityResult->fetch_assoc()) {
		$cardRarity[] = $row;
	}
}

$cardTypes = array();
{
	$cardTypesResult = $mysqli->query("SELECT * FROM `cardTypes`;");
	while($row = $cardTypesResult->fetch_assoc()) {
		$cardTypes[] = $row;
	}
}

// ===========

$monsters = array();
{
	$monstersResult = $mysqli->query("SELECT * FROM `monsters`;");
	while($row = $monstersResult->fetch_assoc()) {
		$monsters[] = $row;
	}
}

$monsterRewards = array();
{
	$monsterRewardsResult = $mysqli->query("SELECT * FROM `monsterRewards`;");
	while($row = $monsterRewardsResult->fetch_assoc()) {
		$monsterRewards[] = $row;
	}
}

$monsterRewardsTypes = array();
{
	$monsterRewardsTypesResult = $mysqli->query("SELECT * FROM `monsterRewardsType`;");
	while($row = $monsterRewardsTypesResult->fetch_assoc()) {
		$monsterRewardsTypes[] = $row;
	}
}

// ===========

$monsterDecks = array();
{
	$monsterDecksResult = $mysqli->query("SELECT * FROM `monsterDecks`;");
	while($row = $monsterDecksResult->fetch_assoc()) {
		$row['monsters'] = array();
		$deckRelationResult = $mysqli->query("SELECT * FROM `monsterDeckRelation` WHERE `deckId`=" . $row['id']);
		while($relationRow = $deckRelationResult->fetch_assoc()) {
			$row['cards'][] = $relationRow['monsterId'];
		}
		$monsterDecks[] = $row;
	}
}

header("Content-Type: application/json");

exit(json_encode(array("valid" => true,
	"cards" => $cards,
	"cardRarity" => $cardRarity,
	"cardTypes" => $cardTypes,
	"monsters" => $monsters,
	"monsterRewards" => $monsterRewards,
	"monsterRewardsType" => $monsterRewardsTypes,
	"monsterDecks" => $monsterDecks
), JSON_NUMERIC_CHECK));
