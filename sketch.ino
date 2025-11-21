
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* WIFI_SSID = "Wokwi-GUEST"; 
const char* WIFI_PASSWORD = "";          

const char* THINGSPEAK_API_KEY = "8JHTITC5CKIDA6P0";
const char* THINGSPEAK_SERVER = "http://api.thingspeak.com/update";

#define LED_PIN 2        
#define BUZZER_PIN 18    
#define BUTTON_PIN 4     
#define SDA_PIN 21       
#define SCL_PIN 22

const float LIMITE_BOA_POSTURA = 10.0;      
const float LIMITE_ATENCAO = 25.0;       

const unsigned long TEMPO_ALERTA_MS = 10000;  

const unsigned long INTERVALO_THINGSPEAK = 15000;

const int BUZZER_FREQUENCIA = 2000;  
const int BUZZER_DURACAO = 200;     


Adafruit_MPU6050 mpu;  

enum EstadoPostura {
  BOA_POSTURA = 0,
  ATENCAO = 1,
  MA_POSTURA = 2
};

EstadoPostura estadoAtual = BOA_POSTURA;
bool alertaAtivo = false;
unsigned long tempoInicioMaPostura = 0;
unsigned long ultimoEnvioThingSpeak = 0;
unsigned long ultimoBeep = 0;
float anguloAtual = 0.0;

bool botaoPressionadoAntes = false;


void setup() {
  Serial.begin(115200);
  Serial.println("\n=== DETECTOR DE POSTURA ERGONÔMICA ===");
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  
  
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  Wire.begin(SDA_PIN, SCL_PIN);
  
  Serial.println("Inicializando MPU6050...");
  if (!mpu.begin()) {
    Serial.println("ERRO: MPU6050 não encontrado!");
    while (1) {
      delay(100);  
    }
  }
  Serial.println("MPU6050 inicializado com sucesso!");
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  conectarWiFi();
  
  Serial.println("Sistema pronto!");
  Serial.println("===================================\n");
  delay(1000);
}


void loop() {
  lerSensorECalcularAngulo();
  
  classificarPostura();
  
  gerenciarAlertas();
  
  verificarBotao();
  
  enviarDadosThingSpeak();
  
  delay(100);
}

void conectarWiFi() {
  Serial.print("Conectando ao WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha ao conectar WiFi!");
  }
}
void lerSensorECalcularAngulo() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  Serial.print("Accel X: ");
  Serial.print(a.acceleration.x, 2);
  Serial.print(" | Y: ");
  Serial.print(a.acceleration.y, 2);
  Serial.print(" | Z: ");
  Serial.print(a.acceleration.z, 2);

  anguloAtual = atan2(a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180.0 / PI;
  
  Serial.print(" || Ângulo: ");
  Serial.print(anguloAtual, 1);
  Serial.print("° | Estado: ");
  
  switch(estadoAtual) {
    case BOA_POSTURA:
      Serial.print("BOA POSTURA");
      break;
    case ATENCAO:
      Serial.print("ATENÇÃO");
      break;
    case MA_POSTURA:
      Serial.print("MÁ POSTURA");
      break;
  }
  
  Serial.print(" | Alerta: ");
  Serial.println(alertaAtivo ? "ATIVO" : "Inativo");
}

void classificarPostura() {
  float anguloAbs = abs(anguloAtual);
  
  if (anguloAbs <= LIMITE_BOA_POSTURA) {
    estadoAtual = BOA_POSTURA;
    tempoInicioMaPostura = 0;
    
  } else if (anguloAbs <= LIMITE_ATENCAO) {
    estadoAtual = ATENCAO;
    tempoInicioMaPostura = 0;
    
  } else {
    estadoAtual = MA_POSTURA;
    
    if (tempoInicioMaPostura == 0) {
      tempoInicioMaPostura = millis();
    }
    
    if (!alertaAtivo && (millis() - tempoInicioMaPostura >= TEMPO_ALERTA_MS)) {
      alertaAtivo = true;
      Serial.println("\n>>> ALERTA ATIVADO: Má postura detectada! <<<\n");
    }
  }
}

void gerenciarAlertas() {
  if (alertaAtivo) {
    // Liga o LED vermelho
    digitalWrite(LED_PIN, HIGH);
    
    if (millis() - ultimoBeep >= 1000) {
      tone(BUZZER_PIN, BUZZER_FREQUENCIA, BUZZER_DURACAO);
      ultimoBeep = millis();
    }
  } else {
    // Desliga LED e buzzer
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }
}


void verificarBotao() {
  bool botaoPressionado = (digitalRead(BUTTON_PIN) == LOW);
  
  if (botaoPressionado && !botaoPressionadoAntes) {
    Serial.println("\n>>> BOTÃO PRESSIONADO: Alerta resetado <<<\n");
    
    // Reseta o estado de alerta
    alertaAtivo = false;
    tempoInicioMaPostura = 0;
    
    // Desliga LED e buzzer imediatamente
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
    
    // Pequeno delay para debounce
    delay(200);
  }
  
  // Atualiza o estado anterior do botão
  botaoPressionadoAntes = botaoPressionado;
}


void enviarDadosThingSpeak() {
  if (millis() - ultimoEnvioThingSpeak < INTERVALO_THINGSPEAK) {
    return;  
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado! Tentando reconectar...");
    conectarWiFi();
    return;
  }
  
  String url = String(THINGSPEAK_SERVER) + "?api_key=" + THINGSPEAK_API_KEY;
  url += "&field1=" + String(anguloAtual, 2);        
  url += "&field2=" + String((int)estadoAtual);     
  url += "&field3=" + String(alertaAtivo ? 1 : 0);  
  
  HTTPClient http;
  http.begin(url);
  
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    Serial.println("Dados enviados para ThingSpeak com sucesso!");
    Serial.print("Resposta: ");
    Serial.println(http.getString());
  } else {
    Serial.print("Erro ao enviar dados. Código: ");
    Serial.println(httpCode);
  }
  
  http.end();
  
  ultimoEnvioThingSpeak = millis();
}
