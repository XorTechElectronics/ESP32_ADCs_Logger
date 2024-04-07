<?php
	
	if(isset($_GET["SensorString"])) {
		$sensorstring = $_GET["SensorString"]; 


		$sql = "INSERT INTO SensorData SET sensorstring='".$sensorstring."'";
		//echo "<br><font color=BLUE>".$sql."</font>";
	
	 
		$query = @mysql_query($sql);
		if (!$query) {
			db_error("insert_data",$sql);
		} 
      
		mysql_close();
	} 
	else {
		echo "SensorString is not set in the HTTP request";
	}

?>
