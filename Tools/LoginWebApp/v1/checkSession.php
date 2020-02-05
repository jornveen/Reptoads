<?php

include_once $_SERVER['DOCUMENT_ROOT'] . "/includes/connect.php";

if(!isset($_POST['token'])) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "token not specified"), JSON_NUMERIC_CHECK));
}
$token = $mysqli->real_escape_string($_POST['token']);

$sessionResult = $mysqli->query("SELECT * FROM `sessions` WHERE `token`='$token';");
if($sessionResult->num_rows <= 0) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "Session expired"), JSON_NUMERIC_CHECK));
}

header("Content-Type: application/json");
exit(json_encode(array("valid" => true), JSON_NUMERIC_CHECK));
