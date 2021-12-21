  //Bibliotecas usadas
  #include <WiFi.h>
  #include <ESPAsyncWebServer.h>
  #include <SPIFFS.h>

  //Entrada analogica usada para ligar a placa de aquisição ao ESP32
  #define entradaAnalog 32

  //Nome e senha do wifi, na qual o ESP32 vai se conectar
  const char* ssid = "NOME_DO_WIFI";
  const char* password = "SENHA_DO_WIFI";

  //Cria o servidor para hospedar a página web
  AsyncWebServer server(80);

  // Criar a tarefa que será rodada no segundo núcleo de processamento do ESP32
  TaskHandle_t Task1;
  
  int valor; 
  String dado = "";
  int flag = 0;
  
  void setup() {
    Serial.begin(115200);

    //Inicializa o sistema SPIFFS
    //OBS: É necessario instalar o SPIFFS na IDE do Arduino para usá-lo
    if (!SPIFFS.begin()) {
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }

    //Conecta ao wifi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }

    //Imprimi no monitor serial o IP do ESP32
    //Esse IP é necessario para acessar a página web
    Serial.println(WiFi.localIP());

    //Envia para a página web todos os arquivos necessarios para o seu funcionamento
    //Esses arquivos estão presentes na pasta "data", que está no mesmo diretorio desse arquivo
    server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/index.html");
    });
    server.on("/data.js", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/data.js", "text/javascript");
    });
    server.on("/export-data.js", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/export-data.js", "text/javascript");
    });
    server.on("/exporting.js", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/exporting.js", "text/javascript");
    });
    server.on("/highcharts.js", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/highcharts.js", "text/javascript");
    });
    server.on("/play.png", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/play.png", "image/png");
    });
    server.on("/pause.png", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/pause.png", "image/png");
    });
    server.on("/restart.png", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/restart.png", "image/png");
    });

    //Tarefa responsavél por enviar os dados lidos pela porta analogica para a página web
    server.on("/EEG", HTTP_GET, [](AsyncWebServerRequest * request) {
      flag = 1;
      String DadoEnviado = dado;
      dado = "";
      request->send(200, "text/html", DadoEnviado.c_str());
    });
  
    server.begin();

    //Inicializa o segundo núcleo de processamento
    xTaskCreatePinnedToCore(
                      Task1code,
                      "Task1",
                      10000,
                      NULL,
                      1,
                      &Task1,
                      0);
    delay(500);
  }
  
  //Função que roda no segundo núcleo
  void Task1code( void * pvParameters ){
    Serial.print("Task1 running on core ");
    Serial.println(xPortGetCoreID());

    //Loop infinito
    for(;;){
      //Só entra nessa função se o ESP32 estiver conectado a um wifi e se a página web já tiver requisitado algum dado
      if(WiFi.status() == WL_CONNECTED && flag == 1){
        valor = analogRead(entradaAnalog);
        if (!isnan(valor)) {
          dado += (String)valor;
          dado += " ";
          delay(10);
        }
      }
    } 
  }
  
  //Não precisa de nenhum codigo no loop, pois as requisições da página web são processadas assincronamento no primeiro núcleo e
  //a aquisição de dados é feita na função acima que processa no segundo núcleo
  void loop() {
    
  }
