#include "WiFi.h"
#include "HTTPClient.h"
#include "stdlib.h"

#define PIN_FALANTE 13
#define PIN_LDR 33
#define LED 2
int LDR;


const char* rede_wifi = "Dom Firmino_2.4";
const char* senha_wifi = "mikaelfirmino";

int sensor_laser;

void conectarWifi(){
  if(WiFi.status() == WL_CONNECTED){
    return;
  }
  WiFi.begin(rede_wifi, senha_wifi);
  Serial.println();
  Serial.print("Conectando a rede Wifi: " + String(rede_wifi));
  int tentativas = 0;
  while(WiFi.status() != WL_CONNECTED && tentativas < 20){
    delay(1000);
    Serial.print(".");
    tentativas++;
  }
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("\nFalha ao conectar ao Wifi. Revise os dados e tente novamente");
  }else {
    Serial.println("\nConectado com sucesso ao Wifi!!");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_FALANTE, OUTPUT);
  pinMode(PIN_LDR, INPUT);
  pinMode(LED, OUTPUT);
  Serial.println("O ESP32 foi iniciado...");
  conectarWifi();
}

void loop() {
  Serial.println();
  Serial.println(tempo_alarme());
  StatusESP32(true);
  reiniciar();
  int vezes = tempo_alarme();
  sensor_laser = laser_stts();
  String dadosParaEnviar = "{\"Sensor_Laser\":" + String(sensor_laser) + "}";
  enviardados(dadosParaEnviar);
  bool alarme = valarme();
  if(alarme == true){
    Serial.println("Alarme acionado com sucesso");
    tocar_alarme(vezes);
    alarme = false;
  }
  delay(30000);
}

void reiniciar(){
  HTTPClient http;  
  http.begin("http://15.229.233.85:8080/reiniciaresp");
  http.addHeader("Content-Type", "application/json");
  int respostaHTTP = http.GET();  
  if(respostaHTTP > 0){
    Serial.println("Codigo de resposta: " + String(respostaHTTP));
    String resposta = http.getString();
    Serial.println(resposta);
    if(resposta == "true"){
      Serial.println();
      Serial.println("Reiniciando o ESP32.....");
      ESP.restart();
    }
  }else{
    Serial.println("Falha na conexao HTTP");
  }
  http.end();
}

void enviardados(String dados){
  HTTPClient http;
  http.begin("http://15.229.233.85:8080/dados-do-esp32");
  http.addHeader("Content-Type", "application/json");
  int respostaHTTP = http.POST(dados);
  if(respostaHTTP > 0){
    Serial.println("Codigo de resposta: " + String(respostaHTTP));
    String resposta = http.getString();
    Serial.println(resposta);
  }else{
    Serial.println("Falha na conexao HTTP");
  }
  http.end();
}

bool laser_stts(){
  LDR = analogRead(PIN_LDR);
  if(LDR < 1000){
    return true;
  }else{
    return false;
  }
}

bool valarme(){
  HTTPClient http;
  http.begin("http://15.229.233.85:8080/acionar-alarme");
  http.addHeader("Content-Type", "application/json");
  int respostaHTTP = http.GET();
  if (respostaHTTP >= 200 && respostaHTTP < 300) {
    Serial.println("Requisição bem-sucedida");
    String alrmar = http.getString();
    if(alrmar == "true"){
        return true;
    }else{
        return false;
    }
  } else {
    Serial.println();
    Serial.print("Falha na requisição. Código de resposta: ");
    Serial.println(respostaHTTP);
    Serial.println();
    return false;
  }

  http.end();
}

void StatusESP32(bool online) {
  HTTPClient http;
  http.begin("http://15.229.233.85:8080/atualizar-status");
  http.addHeader("Content-Type", "application/json");
  String dados = "{\"status\":";
  dados += online ? "true" : "false";
  dados += "}";

  int respostaHTTP = http.POST(dados);
  if(respostaHTTP > 0){
    Serial.println("Codigo de resposta: " + String(respostaHTTP));
    String resposta = http.getString();
    Serial.println(resposta);
  }else{
    Serial.println("Falha na conexao HTTP");
  }
  http.end();
}

int tempo_alarme(){
  int vtemp;
  HTTPClient http;
  http.begin("http://15.229.233.85:8080/tempo-alarmar");
  int respostaHTTP = http.GET();
  String tempo_alarmar;
  if (respostaHTTP >= 200 && respostaHTTP < 300) {
    tempo_alarmar = http.getString();
    vtemp = tempo_alarmar.toInt();
    Serial.println();
    Serial.println(vtemp);
    Serial.println();
    return vtemp;
  } else {
    Serial.println();
    Serial.print("Falha na requisição. Código de resposta: ");
    Serial.println(respostaHTTP);
    Serial.println();
    return 0;
  }
  http.end();
}

void tocar_alarme(int t){
  int i = 0;
  while(i <= t){
    tone(PIN_FALANTE, 1350);
    delay(500);
    tone(PIN_FALANTE, 1100);
    delay(1000);
    i++;
  }
  noTone(PIN_FALANTE);
}