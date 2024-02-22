#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <String.h>
#include <Wire.h>
//--------------------------------------------------Variaveis globais ----------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

ESP8266WebServer server(80);
const char* ssidAP = "Frequencia-Respiratoria-MM";
const char* passwordAP = "123456789";

const String URL = "http://192.168.1.231:80/teste/tratamento.php";              // Ligação ao Servidor Apache e base de dados mySQL
const String paginaInicial = "http://192.168.1.231:80/teste/calendario.php";

const int port = 80;

const int ledPin = 2;
int flagLigar, cont, flagContador, flagPrimeiro;

String buffer; 
String bufferComp = "valor=";

WiFiClient client;
//------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600); // iniciar a comunicação serial

  WiFi.persistent(true); //Grava na memoria Flash os dados SSID e Password da rede a qual ligou
  
  WiFi.begin(); //Inicia ligação Wi-Fi
  WiFi.softAP(ssidAP,passwordAP); //Inicia Acess Point

  IPAddress ip = WiFi.softAPIP();
  Serial.print("Endereço IP do Medidor: ");
  Serial.println(ip);

  server.on("/", handleRoot);
  server.on("/connect", handleConnect);
  

  server.begin();

  flagLigar = flagPrimeiro = cont = flagContador = 0;
  buffer = "valor=";

  pinMode(ledPin, OUTPUT);
}
//--------------------------------------------------------------------------------------------------------------------------
//-------------------------- Função Referente à pagina HTML da Introução ---------------------------------------------------
void handleRoot() {
  Serial.print("}");
  String html = "<html><body>";
  if (WiFi.status() == WL_CONNECTED) {
    String ssid = WiFi.SSID();
    html += "<h1>Conectado a: " + ssid + "</h1>";
    html += "<h2>Conectar a outra rede Wi-Fi</h2>";
    html += "<form method='post' action='/connect'>";
    html += "<label for='ssid'>SSID:</label>";
    html += "<select name='ssid' id='ssid'>";
  }
  else{
    html += "<h2>Conectar a rede Wi-Fi</h2>";
    html += "<form method='post' action='/connect'>";
    html += "<label for='ssid'>SSID:</label>";
    html += "<select name='ssid' id='ssid'>";
  }
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    html += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
  }
  html += "</select><br>";
  html += "<label for='password'>Senha:</label>";
  html += "<input type='password' name='password' id='password'><br>";
  html += "<input type='submit' value='Conectar'>";
  html += "</form>";
  html += "<button onclick=\"location.href='" + paginaInicial + "'\">Ir para a p&aacute;gina</button>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------- Função Referente à pagina HTML de resolução da ligação à Internet ---------------------------------------------------

void handleConnect() {
  Serial.print("}");
  String ssid;
  String password;
  if (server.hasArg("ssid")) {
    ssid = server.arg("ssid");
  }
  if (server.hasArg("password")) {
    password = server.arg("password");
  }

  if (ssid.length() > 0 && password.length() > 0) {
    WiFi.begin(ssid, password);
    for(int i=0;i<=15;i++){
      delay(1000);
      if(WiFi.status() == WL_CONNECTED){
      String html = "<html><body><h1>Conectado ao Wi-Fi com sucesso.</h1><script>window.location.href='" + paginaInicial + "';alert(\"Conectado ao Wi-Fi com sucesso.\");</script></body></html>";
      server.send(200, "text/html", html);
      return;
      }
    }
    String html = "<html><body><h1>Erro: SSID e/ou senha inv&aacute;lidos.</h1><button onclick=\"goBack()\">Retroceder</button><script>function goBack(){window.history.back();} </script></body></html>";
    server.send(400, "text/html", html);
    }
  if(ssid.length() <= 0 || password.length() <= 0){
    String html = "<html><body><h1>Erro: SSID e/ou senha inv&aacute;lidos.</h1><button onclick=\"goBack()\">Retroceder</button><script>function goBack(){window.history.back();} </script></body></html>";
    server.send(400, "text/html", html);
  }
}
//------------------------------------------------- funcao que liga e desliga LED a indicar a ligação a WIFI ----------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
void ledLigacaoWIFI(){
  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(ledPin, HIGH); // Liga o LED
    flagLigar = 1;
    Serial.print("{");
    delay(100);
  }
  else{
    digitalWrite(ledPin, LOW); // Desliga o LED
    flagLigar = 0;
    Serial.print("}");
    delay(100); // Pequeno atraso para tornar a indicação do LED visível 
    }
}

int receberSerial(void){
  if (Serial.available() > 0) { // Verifica se há dados disponíveis na porta serial
    char receivedChar = Serial.read(); // Lê o próximo caractere disponível na porta serial
    String data = ""; // Inicializa a string para armazenar os dados recebidos

    while (receivedChar != '\0') {
      data += receivedChar; // Adiciona o caractere lido à string
      receivedChar = Serial.read(); // Lê o próximo caractere disponível na porta serial
    }
    int var = atoi(data.c_str()); 
    return var;
  }
  return 0;
}
void enviarWIFI(){
  HTTPClient http;
  WiFiClient client;
  http.begin(client,URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(buffer);
  String payload = http.getString();
}
//------------------------------------------------- MAIN -----------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
void loop() {
  int valor = 0;
  server.handleClient();
  ledLigacaoWIFI();
  valor = receberSerial();
  if(valor > 30){
    cont++;
    buffer += String(valor) + ".";
    if (cont >= 10){
        if(flagPrimeiro == 0){
           buffer += ">";
           flagPrimeiro = 1;
        }
        enviarWIFI();
        Serial.println(buffer);
        cont = 0;
        buffer = "valor=";
    }
  }
  if(valor <= 30 && buffer != "valor=") {
    buffer += "<";
    while (Serial.available() > 0){
      Serial.read();
    }
    enviarWIFI();
    Serial.println(buffer);
    buffer = "valor=";
    flagPrimeiro = 0;
  }  
}
