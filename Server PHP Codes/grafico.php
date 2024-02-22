<?php
error_reporting(0);
$hostname = "localhost";
$username = "root";
$password = "";
$database = "frequencia_respiratoria";

$diretorioDeGravacao = 'C:/xampp/htdocs/teste/db/';
$filename = [];

$conn = mysqli_connect($hostname, $username, $password, $database);
if(!$conn){
    die("Conexão falhada: " . mysqli_connect_error());
}

if (isset($_GET['nome']) && isset($_GET['infoID'])) {
    $nome = $_GET['nome'];
    $infoID = $_GET['infoID'];
    $sql = "UPDATE frequencia SET nome = '$nome' WHERE id = $infoID";
    if (mysqli_query($conn, $sql)) {
        echo "Nome adicionado com sucesso à base de dados.";
    } else {
        echo "Erro ao adicionar o nome à base de dados: " . mysqli_error($conn);
    }
}

if (isset($_GET['id'])) {
    $inicial = $_GET['id'];
    $filename = $diretorioDeGravacao .  $inicial;
}

$valores = []; // Array para armazenar os valores

$sql = "SELECT id, valorADC, dataHora, nome FROM frequencia WHERE id = '$inicial'";
$result = mysqli_query($conn, $sql);

while ($row = mysqli_fetch_assoc($result)) {
    $nomebase = $row['nome'];
    if (file_exists($filename . ".txt")) {
        $file = fopen($filename . ".txt", "r");
        if($file){
            while (($line = fgets($file)) !== false) {
                $valores[] = $line;
            }
            fclose($file);
        } else {
            echo "Erro ao abrir o arquivo: " . $filename;
        }
    } else {
        echo "Erro ao abrir o arquivo: " . $filename;
    }
}
?>

<!DOCTYPE html>
    <html lang="pt-PT">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>Frequencia Respiratoria</title>
            <link href="https://cdnjs.cloudflare.com/ajax/libs/chartjs/3.7.0/chart.min.css" rel="stylesheet">
            <script src="https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.29.1/moment.min.js"></script>
            <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.7.0/chart.min.js"></script>
            <script src="https://cdnjs.cloudflare.com/ajax/libs/chartjs-plugin-zoom/1.1.1/chartjs-plugin-zoom.min.js"></script>
            <button onclick="retroceder()">Retroceder</button>
            <form id="nameForm">
            <label for="nome">Nome:</label>
            <input type="text" id="nome" name="nome">
            <input type="hidden" id="infoID" name="infoID">
            <button type="button" onclick="submeter()">Adicionar Nome</button>
    </form>
    </form>
        </head>
        <body>
            <canvas id="myChart"></canvas>     
            <script>
                function submeter() {
                    var nome = document.getElementById("nome").value;
                    var urlAtual = window.location.href;
                    var id = new URLSearchParams(urlAtual.split('?')[1]).get("id");
                    var url = window.location.href.split('?')[0] + "?nome=" + encodeURIComponent(nome) + "&infoID=" + <?php echo $inicial?> + "&id=" + <?php echo $inicial?>;
                    window.location.href = url;
                }
                function retroceder() {
                    window.history.back(); // Go back in the browser history
                }
                var numDataPoints = <?php echo count($valores); ?>;
                var samplingInterval = 0.1; // Intervalo de amostragem em segundos

                var timeLabels = [];
                for (var i = 0; i < numDataPoints; i++) {
                    timeLabels.push((i * samplingInterval).toFixed(1));
                }
                var data = {
                    labels: timeLabels,
                    datasets: [{
                    label: '<?php echo $nomebase ?>',
                    data: <?php echo json_encode($valores); ?>,
                    borderColor: 'rgba(54, 162, 235, 1)',
                    borderWidth: 2,
                    fill: false
                    }]
                };

                var options = {
                    plugins: {
                        zoom: {
                            zoom: {
                                wheel: {
                                    enabled: true, // Habilita o zoom com a roda do mouse
                                },
                                pinch: {
                                    enabled: true // Habilita o zoom com gestos de pinça em dispositivos touch
                                },
                                 mode: 'x' // Zoom apenas no eixo x
                            },
                            pan: {
                                enabled: true,
                                mode: 'x' // Pan apenas no eixo x
                            }
                        }
                    },
                    scales: {
                        x: {
                            title: {
                                display: true,
                                text: 'Tempo (s)' // Título do eixo x
                            }
                        },
                        y: {
                            beginAtZero: true,
                            title: {
                            display: true,
                            text: 'ValorADC' // Título do eixo y
                            }
                        }
                    }
                };

                var ctx = document.getElementById('myChart').getContext('2d');
                var myChart = new Chart(ctx, {
                    type: 'line',
                    data: data,
                    options: options
                });

            </script>
        </body>
    </html>