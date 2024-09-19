
#include "WiFi.h" // Blioteca para configurações WIFI 
#include "HTTPClient.h" // Blioteca para realizar requisições HTTP
#include <ArduinoJson.h> // Incluindo a biblioteca ArduinoJson
#include <vector> // Incluir vector dinâmico
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
  float x;
  float y;
};


float rx1 = 1;
float ry1 = 8;
float rx2 = 8;
float ry2 = 8;
float distanciaEntreRoteadores = 8;
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
Position calculatePosition(float x1 , float y1 , float x2, float y2, float d1, float d2) {
  /*
  Serial.println("Valores variavéis ");
  Serial.print("x1: ");
  Serial.print(x1);
  Serial.print(" y1: ");
  Serial.print(y1);
  Serial.print(" x2: ");
  Serial.print(x2);
  Serial.print(" y2: ");
  Serial.print(y2);
  Serial.print(" d1: ");
  Serial.print(d1);
  Serial.print(" d2: ");
  Serial.println(d2);
  */

  float A = 2 * (x2 - x1);
  float B = 2 * (y2 - y1);
  float C = pow(x1, 2) + pow(y1, 2) - pow(x2, 2) - pow(y2, 2) - pow(d1, 2) + pow(d2, 2);
  // As fórmulas para calcular x e y
  float x = (C * B) / (A * A + B * B);
  float y = (C * A) / (A * A + B * B);
  /*
  Serial.println("Valores Função Calculate Position ");
  Serial.print("A: ");
  Serial.print(A);
  Serial.print(" B: ");
  Serial.print(B);
  Serial.print(" C: ");
  Serial.print(C);
  Serial.print(" x: ");
  Serial.print(x);
  Serial.print(" y: ");
  Serial.print(y);
  */
  return Position{x, y};
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

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  getNetworkAps();

  //

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