//Bibliotecas
#include <Adafruit_BMP085.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <time.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SPI.h>
#include "OpenWeatherMapOneCall.h"
 
//Definição dos pinos
#define TFT_SCK     0  // Pino SCK ou CLK do display TFT no Pino D3 do NodeMCU
#define TFT_SDA     12 // Pino SDA do display TFT no Pino D6 do NodeMCU
#define TFT_A0      2  // Pino A0 ou RS do display TFT no Pino D4 do NodeMCU
#define TFT_RESET   15 // Pino RESET ou RST do display TFT no Pino D8 do NodeMCU
#define TFT_CS      14 // Pino CS do display TFT no Pino D5 do NodeMCU
#define DHT_PIN     13 // Pino DATA do sensor DHT11 no pino D7 do NodeMCU
 
//Definições WiFi
const char* WIFI_SSID     = "Archer C60";
const char* WIFI_PASSWORD = "02544946";
WiFiClient wifiClient;
 
// Definições Cliente NTP
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = -10800;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
 
//Definições Open Weather Map 
String OPEN_WEATHER_MAP_APP_ID = "Token da API Open Weather Map";
//Go to https://www.latlong.net/ 
float OPEN_WEATHER_MAP_LOCATTION_LAT = -3.732714;
float OPEN_WEATHER_MAP_LOCATTION_LON = -38.526997;
String OPEN_WEATHER_MAP_LANGUAGE = "pt";
boolean IS_METRIC = true;
 
OpenWeatherMapOneCallData openWeatherMapOneCallData;
 
// Definições Dias da Semana e Mês
const String WDAY_NAMES[] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};
const String MONTH_NAMES[] = {"Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul", "Ago", "Set", "Out", "Nov", "Dez"};
 
// Definições Display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_A0, TFT_SDA, TFT_SCK, TFT_RESET);
 
// Definições DHT11
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);
 
// Definições BMP180
Adafruit_BMP085 bmp;
 
// Definição de cores
#define PRETO     0x0000
#define BRANCO    0xFFFF
#define CHUMBO    0x4208
#define TEMP1     0xCE39
#define TEMP2     0xEBF0
#define UMID1     0x7DDD
#define UMID2     0xCF1F
#define PRES1     0x7DDD
#define PRES2     0xCF1F
#define SOL1      0xFC29
#define SOL2      0xFE0C
#define SEN1      0xFE8E
#define SEN2      0xD6FD
#define NUB1      0x8451
#define NUB2      0xD6FD
#define CHU1      0x43B3
#define CHU2      0x7DDD
#define TEM1      0xB659
#define TEM2      0xFE08
#define TEM3      0xA65F
 
// Caracteres externos
extern unsigned char temperatura1[];
extern unsigned char temperatura2[];
extern unsigned char temperatura3[];
extern unsigned char umidade1[];
extern unsigned char umidade2[];
extern unsigned char umidade3[];
extern unsigned char pressao1[];
extern unsigned char pressao2[];
extern unsigned char pressao3[];
extern unsigned char sol1[];
extern unsigned char sol2[];
extern unsigned char sol3[];
extern unsigned char solenuvem1[];
extern unsigned char solenuvem2[];
extern unsigned char solenuvem3[];
extern unsigned char nublado1[];
extern unsigned char nublado2[];
extern unsigned char nublado3[];
extern unsigned char chuva1[];
extern unsigned char chuva2[];
extern unsigned char chuva3[];
extern unsigned char tempestade1[];
extern unsigned char tempestade2[];
extern unsigned char tempestade3[];
extern unsigned char tempestade4[];
 
void setup() {
 
  // Inicialização comunicação serial
  Serial.begin(115200);
 
  // Inicialização WiFi
  connectWifi();
   
  // Inicialização cliente NTP
  timeClient.begin();
  timeClient.update();
 
  // Inicialização display
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  tft.fillScreen(CHUMBO);
  tft.setTextSize(2);
  tft.setTextColor(BRANCO);
  tft.setCursor(2,50);
  tft.print("Conectando...");
  delay(3000);
   
  // Inicialização DHT11
  dht.begin();
   
  // Inicialização BMP180
  bmp.begin();
}
 
void loop() {
 
  // Rotina para imprimir a temperatura local no display
  tft.fillScreen(CHUMBO);
  data();
  horario();
  temperatura();
  delay(10000);
 
  // Rotina para imprimir a umidade relativa do ar local no display
  tft.fillScreen(CHUMBO);
  data();
  horario(); 
  umidade();
  delay(10000);
 
  // Rotina para imprimir a pressão atmosférica local no display
  tft.fillScreen(CHUMBO);
  data();
  horario(); 
  pressao();
  delay(10000);
 
  // Rotina para imprimir a previsão do tempo nos próximos 3 dias no display
  tft.fillScreen(CHUMBO);
  data();
  horario();
  previsao();
  delay(30000);
}
 
// Função para conexão com a rede WiFi
void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando...");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conectado!");
  Serial.println(WiFi.localIP());
  Serial.println();
  delay(7000);
}
 
// Função para obter a data (dia, mês e ano)
void data(){
   
  tft.setTextSize(1);
  OpenWeatherMapOneCall *oneCallClient = new OpenWeatherMapOneCall();
  oneCallClient->setMetric(IS_METRIC);
  oneCallClient->setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
 
  long executionStart = millis();
  oneCallClient->update(&openWeatherMapOneCallData, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATTION_LAT, OPEN_WEATHER_MAP_LOCATTION_LON);
  delete oneCallClient;
  oneCallClient = nullptr;
 
  time_t time;
 
  time_t observationTimestamp0 = openWeatherMapOneCallData.daily[0].dt;
  struct tm* timeInfo0;
  timeInfo0 = localtime(&observationTimestamp0);
  tft.setCursor(35,2);
  tft.print(WDAY_NAMES[timeInfo0->tm_wday]);
  tft.print(", ");
  tft.print(timeInfo0->tm_mday);
  tft.print(" ");
  tft.print(MONTH_NAMES[timeInfo0->tm_mon]);
  tft.print(" ");
  tft.print(timeInfo0->tm_year+1900);
   
}
 
// Função para obter o horário (horas e minutos)
void horario(){
   
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  tft.setCursor(50,15);
  tft.setTextSize(2); 
  if (currentHour < 10){
    tft.print("0");
    tft.print(currentHour);
  }
  else {
    tft.print(currentHour); 
  } 
  tft.print(":");
  if (currentMinute < 10){
    tft.print("0");
    tft.print(currentMinute);
  }
  else {
    tft.print(currentMinute);
  } 
}
 
// Função para monitorar a temperatura local
void temperatura(){
 
  tft.drawBitmap(0,32,temperatura1,96,96,TEMP1);
  tft.drawBitmap(0,32,temperatura2,96,96,TEMP2);
  tft.drawBitmap(0,32,temperatura3,96,96,PRETO);
 
  float t = bmp.readTemperature();
  tft.setTextColor(BRANCO);
  tft.setCursor(85,50);
  tft.setTextSize(1);
  tft.print("Temperatura");
  tft.setCursor(90,65);
  tft.setTextSize(2);
  tft.print(t);
  tft.setCursor(110,90);
  tft.print("C");
  tft.setCursor(100,90);
  tft.setTextSize(1);
  tft.print("o");
}
 
// Função para monitorar a umidade relativa do ar local
void umidade(){
 
  tft.drawBitmap(0,32,umidade1,96,96,UMID1);
  tft.drawBitmap(0,32,umidade2,96,96,UMID2);
  tft.drawBitmap(0,32,umidade3,96,96,PRETO);
   
  float h = dht.readHumidity();
  tft.setCursor (105,50);
  tft.setTextSize(1);
  tft.print("Umidade");
  tft.setCursor(93,65);
  tft.setTextSize(2);
  tft.print(h);
  tft.setCursor(115,90);
  tft.print("%");
}
 
// Função para monitorar a pressão atmosférica local
void pressao(){
 
  tft.drawBitmap(0,32,pressao1,96,96,PRES1);
  tft.drawBitmap(0,32,pressao2,96,96,PRES2);
  tft.drawBitmap(0,32,pressao3,96,96,PRETO);
   
  int t = bmp.readPressure();
  tft.setCursor (105,50);
  tft.setTextSize(1);
  tft.print("Pressao");
  tft.setCursor(100,65);
  tft.setTextSize(2);
  tft.print(t/100);
  tft.setCursor(107,90);
  tft.print("hPa"); 
}
 
// Função para obter a previsão do tempo nos próximos 3 dias
void previsao() {
  tft.setTextSize(1);
   
  OpenWeatherMapOneCall *oneCallClient = new OpenWeatherMapOneCall();
  oneCallClient->setMetric(IS_METRIC);
  oneCallClient->setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
 
  long executionStart = millis();
  oneCallClient->update(&openWeatherMapOneCallData, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATTION_LAT, OPEN_WEATHER_MAP_LOCATTION_LON);
  delete oneCallClient;
  oneCallClient = nullptr;
 
  time_t time;
 
  time_t observationTimestamp1 = openWeatherMapOneCallData.daily[1].dt;
  struct tm* timeInfo1;
  timeInfo1 = localtime(&observationTimestamp1);
  tft.setCursor(2,35);
  tft.print(WDAY_NAMES[timeInfo1->tm_wday]);
  tft.setCursor(2,45);
  tft.print("Min:");
  int tmin1 = openWeatherMapOneCallData.daily[0].tempMin;
  tft.print(tmin1);
  tft.setCursor(2,55);
  tft.print("Max:");
  int tmax1 = openWeatherMapOneCallData.daily[1].tempMax;
  tft.print(tmax1);
  String cond1 = openWeatherMapOneCallData.daily[1].weatherMain;
  if (cond1 == "Thunderstorm"){
    tft.setCursor(2,65);
    tft.print("Tempestade");
    tft.drawBitmap(2,80,tempestade1,48,48,TEM1);
    tft.drawBitmap(2,80,tempestade2,48,48,TEM2);
    tft.drawBitmap(2,80,tempestade3,48,48,TEM3);
    tft.drawBitmap(2,80,tempestade4,48,48,PRETO);
  }
  else if (cond1 == "Rain"){
    tft.setCursor(2,65);
    tft.print("Chuva");
    tft.drawBitmap(2,80,chuva1,48,48,CHU1);
    tft.drawBitmap(2,80,chuva2,48,48,CHU2);
    tft.drawBitmap(2,80,chuva3,48,48,PRETO);
  }
  else if(cond1 == "Drizzle"){
    tft.setCursor(2,65);
    tft.print("Pouca Chuva");
    tft.drawBitmap(2,80,chuva1,48,48,CHU1);
    tft.drawBitmap(2,80,chuva2,48,48,CHU2);
    tft.drawBitmap(2,80,chuva3,48,48,PRETO);
  }
  else if(cond1 == "Clouds"){
    tft.setCursor(2,65);
    tft.print("Nublado");
    tft.drawBitmap(2,80,nublado1,48,48,NUB1);
    tft.drawBitmap(2,80,nublado2,48,48,NUB2);
    tft.drawBitmap(2,80,nublado3,48,48,PRETO);
  }
  else if(cond1 == "Clear"){
    tft.setCursor(2,65);
    tft.print("Sol");
    tft.drawBitmap(2,80,sol1,48,48,SOL1);
    tft.drawBitmap(2,80,sol2,48,48,SOL2);
    tft.drawBitmap(2,80,sol3,48,48,PRETO);
  }
   
  time_t observationTimestamp2 = openWeatherMapOneCallData.daily[2].dt;
  struct tm* timeInfo2;
  timeInfo2 = localtime(&observationTimestamp2);
  tft.setCursor(57,35);
  tft.print(WDAY_NAMES[timeInfo2->tm_wday]);
  tft.setCursor(57,45);
  tft.print("Min:");
  int tmin2 = openWeatherMapOneCallData.daily[2].tempMin;
  tft.print(tmin2);
  tft.setCursor(57,55);
  tft.print("Max:");
  int tmax2 = openWeatherMapOneCallData.daily[2].tempMax;
  tft.print(tmax2);
  String cond2 = openWeatherMapOneCallData.daily[2].weatherMain;
  if (cond2 == "Thunderstorm"){
    tft.setCursor(57,65);
    tft.print("Tempestade");
    tft.drawBitmap(57,80,tempestade1,48,48,TEM1);
    tft.drawBitmap(57,80,tempestade2,48,48,TEM2);
    tft.drawBitmap(57,80,tempestade3,48,48,TEM3);
    tft.drawBitmap(57,80,tempestade4,48,48,PRETO);
  }
  else if (cond2 == "Rain"){
    tft.setCursor(57,65);
    tft.print("Chuva");
    tft.drawBitmap(57,80,chuva1,48,48,CHU1);
    tft.drawBitmap(57,80,chuva2,48,48,CHU2);
    tft.drawBitmap(57,80,chuva3,48,48,PRETO);
  }
  else if(cond2 == "Drizzle"){
    tft.setCursor(57,65);
    tft.print("Pouca Chuva");
    tft.drawBitmap(57,80,chuva1,48,48,CHU1);
    tft.drawBitmap(57,80,chuva2,48,48,CHU2);
    tft.drawBitmap(57,80,chuva3,48,48,PRETO);
  }
  else if(cond2 == "Clouds"){
    tft.setCursor(57,65);
    tft.print("Nublado");
    tft.drawBitmap(57,80,nublado1,48,48,NUB1);
    tft.drawBitmap(57,80,nublado2,48,48,NUB2);
    tft.drawBitmap(57,80,nublado3,48,48,PRETO);
  }
  else if(cond2 == "Clear"){
    tft.setCursor(57,65);
    tft.print("Sol");
    tft.drawBitmap(57,80,sol1,48,48,SOL1);
    tft.drawBitmap(57,80,sol2,48,48,SOL2);
    tft.drawBitmap(57,80,sol3,48,48,PRETO);
  }
 
  time_t observationTimestamp3 = openWeatherMapOneCallData.daily[3].dt;
  struct tm* timeInfo3;
  timeInfo3 = localtime(&observationTimestamp3);
  tft.setTextSize(1);
  tft.setCursor(110,35);
  tft.print(WDAY_NAMES[timeInfo3->tm_wday]);
  tft.setCursor(110,45);
  tft.print("Min:");
  int tmin3 = openWeatherMapOneCallData.daily[3].tempMin;
  tft.print(tmin3);
  tft.setCursor(110,55);
  tft.print("Max:");
  int tmax3 = openWeatherMapOneCallData.daily[3].tempMax;
  tft.print(tmax3);
    String cond3 = openWeatherMapOneCallData.daily[3].weatherMain;
  if (cond3 == "Thunderstorm"){
    tft.setCursor(110,65);
    tft.print("Tempestade");
    tft.drawBitmap(110,80,tempestade1,48,48,TEM1);
    tft.drawBitmap(110,80,tempestade2,48,48,TEM2);
    tft.drawBitmap(110,80,tempestade3,48,48,TEM3);
    tft.drawBitmap(110,80,tempestade4,48,48,PRETO);
  }
  else if (cond3 == "Rain"){
    tft.setCursor(110,65);
    tft.print("Chuva");
    tft.drawBitmap(110,80,chuva1,48,48,CHU1);
    tft.drawBitmap(110,80,chuva2,48,48,CHU2);
    tft.drawBitmap(110,80,chuva3,48,48,PRETO);
  }
  else if(cond3 == "Drizzle"){
    tft.setCursor(110,65);
    tft.print("Pouca Chuva");
    tft.drawBitmap(110,80,chuva1,48,48,CHU1);
    tft.drawBitmap(110,80,chuva2,48,48,CHU2);
    tft.drawBitmap(110,80,chuva3,48,48,PRETO);
  }
  else if(cond3 == "Clouds"){
    tft.setCursor(110,65);
    tft.print("Nublado");
    tft.drawBitmap(110,80,nublado1,48,48,NUB1);
    tft.drawBitmap(110,80,nublado2,48,48,NUB2);
    tft.drawBitmap(110,80,nublado3,48,48,PRETO);
  }
  else if(cond3 == "Clear"){
    tft.setCursor(110,65);
    tft.print("Sol");
    tft.drawBitmap(110,80,sol1,48,48,SOL1);
    tft.drawBitmap(110,80,sol2,48,48,SOL2);
    tft.drawBitmap(110,80,sol3,48,48,PRETO);
  }
}
