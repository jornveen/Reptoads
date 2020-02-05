<?php

if(!isset($_POST['username']) || !isset($_POST['password'])) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "Some values not provided.")));
}

include_once $_SERVER['DOCUMENT_ROOT'] . "/includes/connect.php";

$username = $mysqli->real_escape_string($_POST['username']);
$password = hash('sha256', $_POST['password']);

$result = $mysqli->query("SELECT * FROM `profiles` WHERE `username`='$username' AND `password`='$password' AND `enabled`=1 LIMIT 1;");

if($result->num_rows <= 0) {
	header("Content-Type: application/json");
	exit(json_encode(array("valid" => false, "error" => "Invalid username or password")));
}

$profile = $result->fetch_assoc();

$token = hash("sha256", $profile['id'] . "-" . microtime());

$mysqli->query("INSERT INTO `sessions` (`profileId`, `token`, `created`) VALUES (" . $profile['id'] . ", '$token', NOW());");

header("Content-Type: application/json");
exit(json_encode(array("valid" => true, "token" => $token, "profileId" => $profile['id']), JSON_NUMERIC_CHECK));
