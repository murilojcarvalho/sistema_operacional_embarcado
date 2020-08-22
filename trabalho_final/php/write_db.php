<?php
	include('config_db.php');

	$device = $_GET['device'];
	$status = $_GET['status'];

	echo "device: " . $device;
	echo "<br>";
	echo "light: " . $status;
	echo "<br>";

	// Create connection
	$conn = new mysqli($servername, $username, $password, $dbname);
	// Check connection
	if ($conn->connect_error) {
	  die("Connection failed: " . $conn->connect_error);
	}

	$sql = "DELETE FROM atuadores WHERE sensor_name='$device'";

	if ($conn->query($sql) === TRUE) {
	  echo "Record deleted successfully";
	} else {
	  echo "Error deleting record: " . $conn->error;
	}

	echo "<br>";

	$mydate=localtime(time(),true);
	$mydate[tm_year] = $mydate[tm_year] + 1900;
	$mydate[tm_mon] = $mydate[tm_mon] + 1;

	$sql = "INSERT INTO atuadores (sensor_name, sensor_value, sensor_datetime)
	VALUES ('$device', '$status', '$mydate[tm_year]-$mydate[tm_mon]-$mydate[tm_mday] $mydate[tm_hour]:$mydate[tm_min]:$mydate[tm_sec]')";

	if ($conn->query($sql) === TRUE) {
	  echo "New record created successfully";
	} else {
	  echo "Error: " . $sql . "<br>" . $conn->error;
	}

	$conn->close();

	header('Location: index.html');
	exit;

?>