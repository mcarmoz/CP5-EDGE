#include <WiFi.h>         // Biblioteca para utilizar o Wi-Fi no ESP32.
#include <PubSubClient.h> // Biblioteca para utilizar o protocolo MQTT.
#include <DHT.h>
// Configurações - variáveis editáveis
const char* default_SSID = "Wokwi-GUEST"; // Nome da rede Wi-Fi.
const char* default_PASSWORD = "";        // Senha da rede Wi-Fi.
const char* default_BROKER_MQTT = "20.201.117.178";     // Endereço IP do Broker MQTT.
const int default_BROKER_PORT = 1883;     // Porta de conexão do Broker MQTT (1883 é o padrão).
const char* default_TOPICO_SUBSCRIBE = "/TEF/devin/cmd";   // Tópico para receber comandos.
const char* default_TOPICO_PUBLISH_1 = "/TEF/devin/attrs/t"; // Tópico para enviar dados.
const char* default_TOPICO_PUBLISH_2 = "/TEF/devin/attrs/h"; // Tópico para enviar dados de luminosidade.
const char* default_TOPICO_PUBLISH_3 = "/TEF/devin/attrs/l";
const char* default_ID_MQTT = "fiware_001"; // ID único do cliente MQTT.
const int default_D4 = 2; // Pino do LED onboard no ESP32.

// Declaração da variável para o prefixo do tópico
const char* topicPrefix = "devin"; // Prefixo para formar os tópicos de comando.
DHT dht(15, DHT22);
long tempo = 0;
// Variáveis para configurações editáveis
// Essas variáveis podem ser alteradas ao longo do programa.
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int BROKER_PORT = default_BROKER_PORT;
char* TOPICO_SUBSCRIBE = const_cast<char*>(default_TOPICO_SUBSCRIBE);
char* TOPICO_PUBLISH_1 = const_cast<char*>(default_TOPICO_PUBLISH_1);
char* TOPICO_PUBLISH_2 = const_cast<char*>(default_TOPICO_PUBLISH_2);
char* TOPICO_PUBLISH_3 = const_cast<char*>(default_TOPICO_PUBLISH_3);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);
int D4 = default_D4; // Pino do LED onboard.

WiFiClient espClient; // Cria um cliente Wi-Fi.
PubSubClient MQTT(espClient); // Cria um cliente MQTT usando o cliente Wi-Fi.
char EstadoSaida = '0'; // Variável para armazenar o estado do LED (ligado/desligado).

// Função para iniciar a comunicação serial.
void initSerial() {
    Serial.begin(115200);
}

// Função para conectar ao Wi-Fi.
void initWiFi() {
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconectWiFi(); // Chama a função que reconecta ao Wi-Fi.
}

// Função para configurar o MQTT (Broker e callback).
void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT); // Configura o servidor MQTT.
    MQTT.setCallback(mqtt_callback); // Configura a função de callback para receber mensagens.
}

// Função de setup do ESP32.
void setup() {
    dht.begin();
    
    initSerial();  // Inicia a comunicação serial.
    initWiFi();    // Conecta ao Wi-Fi.
    initMQTT();    // Configura o MQTT.
    delay(5000);   // Espera 5 segundos antes de enviar a primeira mensagem.
}

// Função principal que será executada repetidamente.
void loop() {
    
    VerificaConexoesWiFIEMQTT(); // Verifica a conexão com Wi-Fi e MQTT.
    MQTT.loop();
    char msgBuffer[4];                 // Mantém o cliente MQTT ativo.
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    if(isnan(humidity) || isnan(temperature)){
      Serial.println("falha na leitura do dht 22");
    }
    Serial.print("Umidade: ");
    Serial.print(humidity);
    Serial.print(F(" % Temperatura: "));
    Serial.print(temperature);
    Serial.println(F(" °C "));


    String msgH = String(humidity);
    MQTT.publish(TOPICO_PUBLISH_2,msgH.c_str());

    String msgT = String(temperature);
    MQTT.publish(TOPICO_PUBLISH_1,msgT.c_str());

    Lumin();

    delay(2000); 
}

// Função para reconectar ao Wi-Fi.
void reconectWiFi() {
    if (WiFi.status() == WL_CONNECTED)
        return; // Se já estiver conectado, sai da função.

    WiFi.begin(SSID, PASSWORD); // Conecta ao Wi-Fi.
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP()); // Mostra o IP obtido.

    // Garante que o LED onboard inicie desligado.
    digitalWrite(D4, LOW);
}

// Função chamada ao receber mensagens MQTT.
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (int i = 0; i < length; i++) {
        char c = (char)payload[i];
        msg += c; // Converte o payload em uma string.
    }
    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);

    // Forma o padrão de tópico para comparar com as mensagens recebidas.
    String onTopic = String(topicPrefix) + "@on|";
    String offTopic = String(topicPrefix) + "@off|";

    // Liga o LED caso a mensagem seja de ligar.
    if (msg.equals(onTopic)) {
        digitalWrite(D4, HIGH);
        EstadoSaida = '1';
    }

    // Desliga o LED caso a mensagem seja de desligar.
    if (msg.equals(offTopic)) {
        digitalWrite(D4, LOW);
        EstadoSaida = '0';
    }
}

// Verifica se o Wi-Fi e o MQTT estão conectados.
void VerificaConexoesWiFIEMQTT() {
    if (!MQTT.connected())
        reconnectMQTT(); // Se o MQTT não estiver conectado, tenta reconectar.
    reconectWiFi();       // Verifica a conexão Wi-Fi.
}





// Função para reconectar ao Broker MQTT.
void reconnectMQTT() {
    while (!MQTT.connected()) { // Tenta conectar enquanto não estiver conectado.
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) { // Conecta ao broker MQTT.
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); // Se conecta ao tópico de comandos.
        } else {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Haverá nova tentativa de conexão em 2s");
            delay(2000); // Espera 2 segundos antes de tentar novamente.
        }
    }
}

void Lumin() {
    const int potPin = 34;
    int sensorValue = analogRead(potPin);
    int luminosity = map(sensorValue, 0, 4095, 0, 100);
    String mensagem = String(luminosity);
    Serial.print("Valor da luminosidade: ");
    Serial.println(mensagem.c_str());
    MQTT.publish(TOPICO_PUBLISH_3, mensagem.c_str());
}
