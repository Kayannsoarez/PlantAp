//--------Bibliotecas utilizadas no código--------

#include <SPI.h>
#include <Wire.h>
#include <BH1750.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <PubSubClient.h>
#include "EspMQTTClient.h"
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ST7735.h>

//--------Configurações iniciais do display LCD TFT--------

//Definição dos pinos
#define TFT_SCK     0  // Pino SCK ou CLK do display TFT no Pino D3 do NodeMCU
#define TFT_SDA     12 // Pino SDA do display TFT no Pino D6 do NodeMCU
#define TFT_A0      2  // Pino A0 ou RS do display TFT no Pino D4 do NodeMCU
#define TFT_RESET   15 // Pino RESET ou RST do display TFT no Pino D8 do NodeMCU
#define TFT_CS      14 // Pino CS do display TFT no Pino D5 do NodeMCU

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_A0, TFT_SDA, TFT_SCK, TFT_RESET);

//----------Definição das cores----------
#define PRETO     0x0000
#define AZUL      0x0C3A
#define VERDE     0x07E0
#define AMARELO   0xFFE0 
#define CINZA     0xD6BA
#define MAGENTA   0xF81F
#define VERMELHO  0xF800
#define BRANCO    0xFFFF

//----------Caracteres externos----------
extern unsigned char feliz_icon[];
extern unsigned char sede_icon[];
extern unsigned char enjoada_icon[];
extern unsigned char calor_icon[];
extern unsigned char frio_icon[];
extern unsigned char vampiro_icon[];
extern unsigned char oculos_icon[];

//----------Variáveis globais utilizados no código----------
int var_feliz   = 0;
int var_sede    = 0;
int var_enjoada = 0;
int var_calor   = 0;
int var_frio    = 0;
int var_vampiro = 0;
int var_oculos  = 0;
int Feeling = 0;

//-------Informações sobre a planta--------

// Valores mínimos e máximos de umidade do solo, temperatura e luminosidade para CACTOS
int u_min = 55;
int u_max = 75;
int t_min = 22;
int t_max = 35;
int l_min = 100;
int l_max = 5000;

// Valores mínimos e máximos de umidade do solo, temperatura e luminosidade para SUCULENTAS
//int u_min = 45;
//int u_max = 65;
//int t_min = 22;
//int t_max = 35;
//int l_min = 100;
//int l_max = 5000;

// Valores mínimos e máximos de umidade do solo, temperatura e luminosidade para ESPADA DE SÂO JORGE
//int u_min = 20;
//int u_max = 50;
//int t_min = 15;
//int t_max = 30;
//int l_min = 300;
//int l_max = 10000;

//----------Variáveis para Json----------

char lumi[100];
char temp[100];
char humi[100];
char feel[100];

//----------Configurações da conexão MQTT----------
EspMQTTClient client
(
  "Archer C60", //Nome da sua rede Wi-Fi
  "02544946", //Senha da sua rede Wi-Fi
  "mqtt.tago.io",  // MQTT Broker server ip padrão da tago
  "Default",   // Username
  "634c9b71-674c-4998-a191-b59d61300ff2",   // Código do Token
  "TestClient",      // Nome do cliente que identifica exclusivamente seu device
  1883              // A porta MQTT, padrão 1883.
);

//--------Configurações iniciais do sensor de umidade do solo--------

int AOUT;
float Umidade_Solo;

//--------Configurações iniciais do sensor de temperatura--------

#define BMP280_I2C_ADDRESS  0x76
Adafruit_BMP280 bmp280;

//--------Configurações iniciais do sensor de luminosidade--------
BH1750 lightMeter;

//--------Setup--------

void setup() {

  // Inicializa a comunicação serial
  Serial.begin(9600);

  // Inicializa o sensor de temperatura
  if (!bmp280.begin(BMP280_I2C_ADDRESS))
  {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  // Inicializa o sensor de luminosidade
  lightMeter.begin();

  // Inicializa o display LCD TFT
  tft.initR(INITR_BLACKTAB);
  tft.fillRect( 0, 0, 128, 160, PRETO);

}

// Esta função é chamada assim que tudo estiver conectado (Wifi e MQTT)
void onConnectionEstablished()
{}

//--------Loop--------

void loop() {

  // Verifica a luminosidade ambiente
  uint16_t Luminosidade = lightMeter.readLightLevel();
  Serial.print(F("Luminosidade= "));
  Serial.println(Luminosidade);

  // Verifica a temperatura ambiente
  float Temperatura = bmp280.readTemperature();
  Serial.print(F("Temperatura = "));
  Serial.println(Temperatura);

  // Verifica a umidade do solo
  AOUT = analogRead(A0);
  Umidade_Solo = 100 * ((978 - (float)AOUT) / 978);
  Serial.print(F("Umidade do solo = "));
  Serial.println(Umidade_Solo);

  // Se a umidade do solo for muito baixa, a plantinha ficará com sede
  if (Umidade_Solo < u_min) {
    if (var_sede == 0) {
      limpar();
      var_sede = 1;
      Feeling = 2;
    }
    sede();
  }

  // Se a umidade do solo for muito alta, a plantinha ficará enjoada
  else if (Umidade_Solo > u_max) {
    if (var_enjoada == 0) {
      limpar();
      var_enjoada = 1;
      Feeling = 3;
    }
    enjoada();
  }

  // Se a temperatura for muito baixa, a plantinha ficará com frio
  else if (Temperatura < t_min) {
    if (var_frio == 0) {
      limpar();
      var_frio = 1;
      Feeling = 5;
    }
    frio();
  }

  // Se a temperatura for muito alta, a plantinha ficará com calor
  else if (Temperatura > t_max) {
    if (var_calor == 0) {
      limpar();
      var_calor = 1;
      Feeling = 4;
    }
    calor();
  }

  // Se a luminosidade for muito baixa, a plantinha virará uma vampira
  else if (Luminosidade < l_min) {
    if (var_vampiro == 0) {
      limpar();
      var_vampiro = 1;
      Feeling = 6;
    }
    vampiro();
  }

  // Se a luminosidade for muito alta, a plantinha ficará com óculos escuros
  else if (Luminosidade > l_max) {
    if (var_oculos == 0) {
      limpar();
      var_oculos = 1;
      Feeling = 7;
    }
    oculos();
  }

  // Se todos os parâmetros estiverem dentro dos valores ideais, a plantinha ficará feliz
  else {
    if (var_feliz == 0) {
      limpar();
      var_feliz = 1;
      Feeling = 1;
    }
    feliz();
  }

  delay(5000);

    //----------Arquivo Json----------

  StaticJsonDocument<300> jlumi;

  jlumi["variable"] = "lumi";
  jlumi["value"] = Luminosidade;

  serializeJson(jlumi, lumi);

  StaticJsonDocument<300> jtemp;

  jtemp["variable"] = "temp";
  jtemp["value"] = Temperatura;

  serializeJson(jtemp, temp);

  StaticJsonDocument<300> jhumi;

  jhumi["variable"] = "humi";
  jhumi["value"] = Umidade_Solo;

  serializeJson(jhumi, humi);

  StaticJsonDocument<300> jfeel;

  jfeel["variable"] = "feel";
  jfeel["value"] = Feeling;

  serializeJson(jfeel, feel);

  //----------Envio de dados----------

  client.publish("info/lumi", lumi);
  client.publish("info/temp", temp);
  client.publish("info/humi", humi);
  client.publish("info/feel", feel);
  delay(1000);

  client.loop();
  
}

//--------Função limpar display--------

void limpar()
{
  delay(500);
  tft.fillRect(0, 0, 128, 160, PRETO);
  delay(100);
}

//--------Função plantinha com sede--------

void sede()
{
  tft.drawBitmap(0, 0, sede_icon, 128, 160, BRANCO);

  tft.setTextColor(BRANCO);
  tft.setCursor(20, 140);
  tft.println(F("Que seeeeeeede!"));
  tft.setCursor(30, 150);;
  tft.print(F("Preciso de agua."));

  var_feliz   = 0;
  var_enjoada = 0;
  var_calor   = 0;
  var_frio    = 0;
  var_vampiro = 0;
  var_oculos  = 0;
}

//--------Função plantinha enjoada--------

void enjoada()
{
  tft.drawBitmap(0, 0, enjoada_icon, 128, 160, VERDE);
  
  tft.setTextColor(BRANCO);
  tft.setCursor(5, 140);
  tft.println(F("Estou enjoada!"));
  tft.setCursor(7, 150);
  tft.print(F("Nao quero mais agua."));

  var_feliz   = 0;
  var_sede    = 0;
  var_calor   = 0;
  var_frio    = 0;
  var_vampiro = 0;
  var_oculos  = 0;
}

//--------Função plantinha com frio--------

void frio()
{
  tft.drawBitmap(0, 0, frio_icon, 128, 160, AZUL);
  
  tft.setTextColor(BRANCO);
  tft.setCursor(50, 140);
  tft.println(F("Que frio!"));
  tft.setCursor(10, 150);
  tft.print(F("Esta muito gelado."));

  var_feliz   = 0;
  var_sede    = 0;
  var_enjoada = 0;
  var_calor   = 0;
  var_vampiro = 0;
  var_oculos  = 0;
}

//--------Função plantinha com calor--------

void calor()
{
  tft.drawBitmap(0, 0, calor_icon, 128, 160, VERMELHO);

  tft.setTextColor(BRANCO);
  tft.setCursor(25, 140);
  tft.println(F("Que calor!"));
  tft.setCursor(15, 150);
  tft.print(F("Esta muito quente."));

  var_feliz   = 0;
  var_sede    = 0;
  var_enjoada = 0;
  var_frio    = 0;
  var_vampiro = 0;
  var_oculos  = 0;
}

//--------Função plantinha vampira--------

void vampiro()
{
  tft.drawBitmap(0, 0, vampiro_icon, 128, 160, MAGENTA);
  
  tft.setTextColor(BRANCO);
  tft.setCursor(4, 140);
  tft.println(F("Cuidado! Sem luz vou"));
  tft.setCursor(15, 150);
  tft.print(F("virar uma vampira."));

  var_feliz   = 0;
  var_sede    = 0;
  var_enjoada = 0;
  var_calor   = 0;
  var_frio    = 0;
  var_oculos  = 0;
}

//--------Função plantinha com óculos--------

void oculos()
{
  tft.drawBitmap(0, 0, oculos_icon, 128, 160, CINZA);

  tft.setTextColor(BRANCO);
  tft.setCursor(7, 140);
  tft.println(F("Olha esse Sol! Cade"));
  tft.setCursor(13, 150);
  tft.print(F("o protetor solar?"));

  var_feliz   = 0;
  var_sede    = 0;
  var_enjoada = 0;
  var_calor   = 0;
  var_frio    = 0;
  var_vampiro = 0;
}

//--------Função plantinha feliz--------

void feliz()
{
  tft.drawBitmap(0, 0, feliz_icon, 128, 160, AMARELO);

  tft.setTextColor(BRANCO);
  tft.setCursor(20, 140);
  tft.println(F("Hum! Esta tudo"));
  tft.setCursor(10, 150);
  tft.print(F("perfeito. Obrigada!"));

  var_sede    = 0;
  var_enjoada = 0;
  var_calor   = 0;
  var_frio    = 0;
  var_vampiro = 0;
  var_oculos  = 0;
}