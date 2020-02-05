<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

$config = array();

if(file_exists("..\..\..\Projects\\froggy\Server\config.json")) {
	$config = json_decode(file_get_contents(__DIR__ . "\..\..\..\Projects\\froggy\Server\config.json"), true);
} else {
	$config = json_decode(file_get_contents(__DIR__ . "/config.json"), true);
}

$host = $config['database']['host'];
$port = intval($config['database']['port']);
$username = $config['database']['username'];
$password = $config['database']['password'];
$database = $config['database']['database'];

$mysqli = mysqli_connect($host, $username, $password, $database);
if (!$mysqli) {
	echo "Error: Unable to connect to MySQL." . PHP_EOL;
	echo "Debugging errno: " . mysqli_connect_errno() . PHP_EOL;
	echo "Debugging error: " . mysqli_connect_error() . PHP_EOL;
	exit;
}
