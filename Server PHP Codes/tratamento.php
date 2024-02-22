<?php
$hostname = "localhost";
$username = "root";
$password = "";
$database = "frequencia_respiratoria";

$filename = [];
$caracterInicio =">";
$caractereFim = "<";
$diretorioDeGravacao = 'C:/xampp/htdocs/teste/db/';


$conn = mysqli_connect($hostname, $username, $password, $database);
if (!$conn) {
    die("Conexão falhada: " . mysqli_connect_error());
}
echo "Conexão estabelecida à base de dados." . "<br>";

if (isset($_POST["valor"])) {
    $valor = $_POST["valor"];

    $caracterAcao = strpos($valor, $caracterInicio);
    if($caracterAcao !== false ){
        $valoresSemPont = rtrim($valor, ".");
        $valores = explode(".", $valoresSemPont);
        $primeirovalor = array_shift($valores);
        $sql = "INSERT INTO frequencia (valorADC) VALUES ($primeirovalor)";

        if (mysqli_query($conn, $sql)) {
            echo "Primeiro valor adicionado com sucesso à base de dados.";
            
            $sql = "SELECT id, valorADC, dataHora, nome FROM frequencia";
            $result = mysqli_query($conn, $sql);
            if ($result !== false) {
                $row = mysqli_fetch_assoc($result);
                $filename = $diretorioDeGravacao .  $row['id'];
            } else {
                echo "Erro ao buscar o id na base de dados: " . mysqli_error($conn);
            }
        } else {
            echo "Erro ao adicionar o primeiro valor à base de dados: " . mysqli_error($conn);
        }
        $file = fopen($filename . ".txt", "w");
        fwrite($file, $v . "\n");
        foreach ($valores as $v) {
            if($v === '>' || $v === '<'){
                break;
            }
            fwrite($file, $v . "\n");
        }
        fclose($file);
    }
    $caracterAcao = strpos($valor, $caractereFim);
    if ($caracterAcao !== false) {
        $sql = "SELECT id FROM frequencia ORDER BY id DESC LIMIT 1";
        $result = mysqli_query($conn, $sql);
        if ($result !== false) {
            $row = mysqli_fetch_assoc($result);
            $filename = $diretorioDeGravacao .  $row['id'];
        } else {
            echo "Erro ao buscar o id na base de dados: " . mysqli_error($conn);
        }
        $valoresSemPont = rtrim($valor, ".");
        $valores = explode(".", $valoresSemPont);
        $file = fopen($filename . ".txt", "a");
        foreach ($valores as $v) {
            if($v === '>' || $v === '<'){
                break;
            }
        fwrite($file, $v . "\n");
        }
        fclose($file);
        $flag = 1;
}
    else{
        $sql = "SELECT id FROM frequencia ORDER BY id DESC LIMIT 1";
        $result = mysqli_query($conn, $sql);
        if ($result !== false) {
            $row = mysqli_fetch_assoc($result);
            $filename = $diretorioDeGravacao .  $row['id'];
        } else {
            echo "Erro ao buscar o id na base de dados: " . mysqli_error($conn);
        }
        $valoresSemPont = rtrim($valor, ".");
        $valores = explode(".", $valoresSemPont);
        $file = fopen($filename . ".txt", "a");
        foreach ($valores as $v) {
            if($v === '>' || $v === '<'){
                break;
            }
            fwrite($file, $v . "\n");
        }
        fclose($file);
    }
} 

mysqli_close($conn);
?>
