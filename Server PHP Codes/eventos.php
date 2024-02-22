<?php
error_reporting(0);
$hostname = "localhost";
$username = "root";
$password = "";
$database = "frequencia_respiratoria";

$conn = mysqli_connect($hostname, $username, $password, $database);
if(!$conn){
    die("Conexão falhada: " . mysqli_connect_error());
}

$sql = "SELECT id, dataHora, nome FROM frequencia";
$result = mysqli_query($conn, $sql);

$events = array();
$i = 0;

while ($row = mysqli_fetch_assoc($result)) {
    $i++;
    $event = array(
        'id' => $row['id'],
        'title' => 'Leitura #' . $i,
        'start' => $row['dataHora'],
    );
    $events[] = $event;   
}
header('Content-Type: application/json');
echo json_encode($events);
?>