
#include "WiFi.h" // Blioteca para configurações WIFI 
#include "HTTPClient.h" // Blioteca para realizar requisições HTTP
#include <ArduinoJson.h> // Incluindo a biblioteca ArduinoJson
#include <vector> // Incluir vector dinâmico
#include <ArduinoEigen.h> // fazer cálculos
/**
* Collect RTLS training data
* using WiFi
*/
#include <eloquent_rtls.h>
#include <eloquent_rtls/wifi.h>
#include "Classifier.h"
#include "FeaturesConverter.h"


using eloq::rtls::wifiScanner;
using eloq::rtls::FeaturesConverter;

Classifier classifier;
FeaturesConverter converter(wifiScanner, classifier);



// Estrutura para armazenar informações sobre um ponto de acesso
struct AccessPoint {
    String SSID;
    float RSSI;
    String BSSID;
    int DISTANCE;
};

struct Position {
  double x, y;
};

float distance;
float rssi0;
float pathloss = 2.1;

// Calibrar o path loss (n) de acordo com o ambiente de forma automática
float calculatePathLoss(float distance, float rssi, float rssi0) {
  if (distance <= 0) {
    return 0;
  }
  float pathLoss = (rssi0 - rssi) / (10 * log10(distance));
  Serial.print(pathLoss);
}

// Calibrar o RSSI0 de acordo com o ambiente de forma automática
float calibrateRSSI0() {
    float rssiSum = 0;
    float rssiCount = 0;

    // Perform multiple scans to get an average RSSI value
    for (int i = 0; i < 5; i++) {
        int n = WiFi.scanNetworks();
        for (int j = 0; j < n; j++) {
            rssiSum += WiFi.RSSI(j);
            rssiCount++;
        }
        delay(100);
    }

    // Calculate the average RSSI value
    float rssiAverage = rssiSum / rssiCount;

    // Return the calibrated RSSI0 value
    return rssiAverage;
}

// Calcular Distância através do RSSI
float calculateDistance(float RSSI, float RSSI0 = -56, float n=2.1) {
    return pow(10, (RSSI0 - RSSI) / (10 * n));
}
//calcular a posição do esp a partir da das coordenadas do roteador 1 (x1, y1) e roteador 2 (x2, y2), e das distâncias encontradas através do RSSI
/*
Variávies :
- Distância do ESP ao ponto de acesso:
  double d1 -> distância do ESP ao ponto 1
  double d2 -> distância do ESP ao ponto 1
  double d3 -> distância do ESP ao ponto 1
  double d4 -> distância do ESP ao ponto 1

- Coordenadas dos pontos de acesso:
              x   y 
  Position r1 = {1, 14};
  Position r2 = {14, 14};
  Position r3 = {1, 1};
  Position r4 = {14, 1};
*/
Position calculatePosition(double d1, double d2, double d3, double d4, Position r1, Position r2, Position r3, Position r4) {
  // Matriz A -> coordenadas dos pontos de acesso
  Eigen::MatrixXd A(4,2);

  A(0, 0) = 2 * (r2.x - r1.x);
  A(0, 1) = 2 * (r2.y - r1.y);

  A(1, 0) = 2 * (r3.x - r1.x);
  A(1, 1) = 2 * (r3.y - r1.y);

  A(2, 0) = 2 * (r4.x - r1.x);
  A(2, 1) = 2 * (r4.y - r1.y);

  A(3, 0) = 2 * (r4.x - r2.x);
  A(3, 1) = 2 * (r4.y - r2.y);
  
  // Vetor B -> termos independentes das equações
  Eigen::VectorXd B(4);
  B(0) = d1 * d1 - d2 * d2 - r1.x * r1.x - r1.y * r1.y + r2.x * r2.x + r2.y * r2.y;
  B(1) = d1 * d1 - d3 * d3 - r1.x * r1.x - r1.y * r1.y + r3.x * r3.x + r3.y * r3.y;
  B(2) = d1 * d1 - d4 * d4 - r1.x * r1.x - r1.y * r1.y + r4.x * r4.x + r4.y * r4.y;
  B(3) = d2 * d2 - d4 * d4 - r2.x * r2.x - r2.y * r2.y + r4.x * r4.x + r4.y * r4.y;

  // Resolver o sistema usando mínimos quadrados
  Eigen::VectorXd resultado = (A.transpose() * A).inverse() * A.transpose() * B;

  // O vetor 'resultado' contém as coordenadas (x, y)
  Position pos_esp;
  pos_esp.x = resultado(0);
  pos_esp.y = resultado(1);
  
  return pos_esp;
}


const char* apiEndpointGET = "https://deerego-back.onrender.com/user"; // URL GET -> Buscar informações dos usuários
const char* apiEndpointPOST = "https://deerego-back.onrender.com/rebocador/entrega/carrinho"; // URL POST -> adicionar novas informações
const char* apiEndpointPATCH = "https://deerego-back.onrender.com/rebocador/entrega/carrinho/66d843f5abc2283e65640a90"; // URL PATCH -> atualizar informações
const char* ssidLucas = "A30 de Ronaldo";
const char* passwordLucas = "24012006";
String local;

void enviarDados(float espX, float espY, String apiEndpoint) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(apiEndpoint);
        http.addHeader("Content-Type", "application/json");

        // Criar payload JSON com os dados a serem enviados
        String payload = "{";
        payload += "\"IdCarrinho\": ";
        payload += 69;
        payload += ", ";
        payload += "\"Peças\": \"Veloso, Felipe\", ";
        payload += "\"PosX\": ";
        payload += espX;
        payload += ", ";
        payload += "\"PosY\": ";
        payload += espY;
        payload += ", ";
        payload += "\"Local\": \"Setor C\", ";
        payload += "\"StatusManutenção\":\"Operando\", ";
        payload += "\"NomeCarrinho\": \"Veloso CPX\", ";
        payload += "\"StatusCapacidade\": \"Cheio de Rola\"";
        payload += "}";
        
        
        int httpResponseCode = http.POST(payload);
        Serial.println("ResponseCode");
        Serial.print(httpResponseCode);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Response: " + response);
        } else {
            Serial.println("Error on sending POST");
        }

        http.end();
    } else {
        Serial.println("Error: WiFi not connected");
    }
}
void atualizarDados(float espX, float espY, String apiEndpoint, String local) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(apiEndpoint);
        http.addHeader("Content-Type", "application/json");

        // Criar payload JSON com os dados a serem enviados
        // Criar payload JSON com os dados a serem enviados
        String payload = "{";
        payload += "\"PosX\": " + String(espX) + ", ";
        payload += "\"PosY\": " + String(espY) + ", ";
        payload += "\"Local\": \"" + local + "\"";
        payload += "}";
        
        
        int httpResponseCode = http.PATCH(payload);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Response: " + response);
        } else {
            Serial.println("Error on sending PATCH");
        }

        http.end();
    } else {
        Serial.println("Error: WiFi not connected");
    }
}

void getAPI(String apiEndpoint){
  if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(apiEndpoint);
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Response: " + response); // retorno do json
            delay(500);
            Serial.println(httpResponseCode);

            // Fazendo o parsing do JSON
            StaticJsonDocument<2048> doc;
            DeserializationError error = deserializeJson(doc, response);

            if (!error){
              // Iterar sobre o array de objetos
              JsonArray arr = doc.as<JsonArray>();
              for (JsonObject obj : arr){
                const char* Nome = obj["Nome"];
                const char* Email = obj["Email"];
                const char* Role = obj["Role"];
                const char* Fabrica = obj["Fabrica"];
                long telefone = obj["Telefone"];
                bool status = obj["Status"];

                Serial.println("Nome: " + String(Nome));
                Serial.println("Email: " + String(Email));
                Serial.println("Role: " + String(Role));
                Serial.println("Fábrica: " + String(Fabrica));
                Serial.println("Telefone: " + String(telefone));
                Serial.println("Status: " + String(status ? "Ativo" : "Inativo"));

                // Acessando o array 'Rebocadores' se existir
                if (obj.containsKey("rebocadores")){
                  JsonArray rebocadores = obj["rebocadores"];
                  for (JsonObject rebocador : rebocadores){
                    const char* id = rebocador["_id"];
                    int tempoTotal = rebocador["TempoTotal"];
                    int totalCarrinhos = rebocador["TotalCarrinhos"];
                    const char* statusRebocador = rebocador["StatusRebocador"];

                    Serial.println("id: " + String(id));
                    Serial.println("Tempo Toal: " + String(tempoTotal));
                    Serial.println("Total de Carrinhos: " + String(totalCarrinhos));
                    Serial.println("Status do Rebocador: " + String(statusRebocador));

                    // Acessando o array 'Carrinhos' se existir
                    if (obj.containsKey("carrinhos")){
                      JsonArray carrinhos = rebocador["carrinhos"];
                      for (JsonObject carrinho : carrinhos) {
                        const char* nomeCarrinho = carrinho["NomeCarrinho"];
                        const char* pecas = carrinho["Peças"];

                        Serial.println("Nome do Carrinho: " + String(nomeCarrinho));
                        Serial.println("Peças: " + String(pecas));
                      }
                    }
                  }
                }
              }
            } else {
              Serial.println("Falha ao ler o json");
            }
        } else {
            Serial.println("Error on getting Json");
        }
        http.end();
    } else {
        Serial.println("Error: WiFi not connected");
    }
}

// Função para buscar os pontos de acesso com base nos SSIDs definidos por uma lista
std::vector<AccessPoint> getApBySSID(const std::vector<AccessPoint>& accessPoints, const std::vector<String>& ssidList){
  std::vector<AccessPoint> foundAPs;

  // Iterar através dos SSID da lista
  for (const auto& ssid : ssidList) {
    for (const auto& ap : accessPoints) {
      if (ap.SSID == ssid) {
        // se o ssid da rede for encontrado, adicione à lista
        foundAPs.push_back(ap);
        break;
      }
    }
  }

  return foundAPs; // Retornar a lista dos pontos de acesso encontrados
}

void getNetworkAps(){
  std::vector<AccessPoint> strongestAPs; // vetor dinâmico para armazenar as informações (ssid, rssi e distância) dos roteadores
  
  // WiFi.scanNetworks retorna o número de redes encontradas
  int n = WiFi.scanNetworks();
  Serial.println("Scan Feito");
  if (n == 0){
    Serial.println("Nenhuma Rede Encontrada");
  } else {
    Serial.println(n + "- Redes Encontradas");
  }
  
  // Loop para armazenar as informações dos roteadores no vector
  for (int i = 0; i < n; ++i) {
    // Obter SSID e RSSI da rede Wi-Fi
    float rssi = WiFi.RSSI(i);
    String ssid = WiFi.SSID(i);
    String bssid = WiFi.BSSIDstr(i);

    // Criar uma estrutura apartir da AccessPoint para as informações do ponto de acesso que está sendo guardado
    AccessPoint ap;
    ap.RSSI = rssi;
    ap.SSID = ssid;
    ap.BSSID =  bssid;
    ap.DISTANCE = calculateDistance(rssi);

    // Adicionar o ponto de acesso ao vetor
    strongestAPs.push_back(ap);
  }
  
  // Ordenar o vector (do mais forte para o mais fraco)
  std::sort(strongestAPs.begin(), strongestAPs.end(), [](const AccessPoint &a, const AccessPoint &b){
    return a.RSSI > b.RSSI; // ordena em ordem decrescente de RSSI
  });

  //SSID LIST
  std::vector<String> ssidList = {"Guto Rapido", "11B", "AleDessa", "CAMARGO"};

  //Buscar as informações com base na SSID LIST
  std::vector<AccessPoint> selectedAPs = getApBySSID(strongestAPs, ssidList);

  // Exibir Lista Geral
  Serial.println("Lista de pontos de acesso (GERAL): ");
  for (const auto& ap : strongestAPs) {
    Serial.print("SSID: " + String(ap.SSID));
    Serial.print(" | RSSID: " + String(ap.RSSI));
    Serial.print(" | BSSID: " + String(ap.BSSID));
    Serial.println(" | DISTANCE: " + String(ap.DISTANCE));
  }
  // Exibir
  Serial.println("Lista de pontos de acesso (SSIDLIST): ");
  for (const auto& ap : selectedAPs) {
    Serial.print("SSID: " + String(ap.SSID));
    Serial.print(" | RSSID: " + String(ap.RSSI));
    Serial.print(" | BSSID: " + String(ap.BSSID));
    Serial.println(" | DISTANCE: " + String(ap.DISTANCE));
  }
}

void setup() {
  Serial.begin(115200);

  // Teste localizar esp
  Position r1 = {0,0};
  Position r2 = {14,0};
  Position r3 = {0,14};
  Position r4 = {14,14};

  double d1 = 5.0;
  double d2 = 6.0;
  double d3 = 7.0;
  double d4 = 8.0;

  // Calcular a posição
  Position pos_esp = calculatePosition(d1, d2, d3, d4, r1, r2, r3, r4);

  // Exibidr a posição
  Serial.print("Posição do ESP: x: ");
  Serial.print(pos_esp.x);
  Serial.print(" , y: ");
  Serial.println(pos_esp.y);

  /* Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  getNetworkAps();
  */

  /*
  // Wifi scan ML
  wifiScanner.identifyBySSID();
  // set lower bound for RSSI
  wifiScanner.discardWeakerThan(-85);
  // print feature vector before predictions
  converter.verbose();
  */
  Serial.println("Setup done");
}

void loop() {
  //WiFi.begin(ssidLucas, passwordLucas);
  
  /*
  // scan & predict
  String local = converter.predict();
  Serial.println(local);

  // calcular as coordenadas X e Y
  Position pos = calculatePosition(rx1, ry1, rx2, ry2, strongestAPs[0].DISTANCE, strongestAPs[1].DISTANCE);
  Serial.print("ESP Position: (");
  Serial.print(pos.x);
  Serial.print(", ");
  Serial.print(pos.y);
  Serial.println(")");
  
  // Conectar o ESP ao Wifi para enviar os dados ao banco de dados
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssidLucas, passwordLucas);
  while (WiFi.status()!= WL_CONNECTED){
    delay(500);
    Serial.println("not Connected");
  }
  // atualizar status e posição do carrinho
  atualizarDados(pos.x, pos.y, apiEndpointPATCH, local);

  // delay para escanear denovo
  delay(5000);
  */
}