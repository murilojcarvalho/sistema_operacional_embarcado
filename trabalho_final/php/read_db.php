<?php
include('config_db.php');

	$content = ""; 

	/**
	 * Estabelece conexão com o banco de dados Mysql
	 */
	$conn = new mysqli($servername, $username, $password, $dbname);
	
	/**
	 * Verifica se a conexão ocorreu com sucesso.
	 */
	if ($conn->connect_error) {
	    die("Connection failed: " . $conn->connect_error);
	} 

	/**
	 * Prepara a query sql a fim de selecionar todo o conteúdo 
	 * da tabela sensores;
	 */
	$sql = "SELECT id, sensor_name, sensor_value, sensor_datetime FROM iot";
	$result = $conn->query($sql);
	if ($result->num_rows > 0) {
	
		$content =  "<div>
						<table class=\"w3-table-all\">
							<tr class=\"w3-blue\">
								<th>SENSOR</th>
								<th>VALOR</th>
								<th>DATA E HORA</th>
							</tr>";    
	    /**
	     * Cada iteração do loop será retornado um valor da tabela;
	     */
	    while($row = $result->fetch_assoc()) 
	    {        
	        $content .= "<tr>
	        			 	<td>" . $row["sensor_name"] . "</td> 
	        			 	<td>" . $row["sensor_value"] . "</td>
	        			 	<td>" . $row["sensor_datetime"] . "</td>
	        			  </tr>";
	    }
	    $content .= 	"</table>
	    			</div>";
	} else {
	    $content = "Resultado: 0";
	}
	$conn->close();

	echo $content;
?>