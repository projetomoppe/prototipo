#include <SPI.h>
//#include "nRF24L01.h"
//#include "RF24.h"
#include <NewPing.h>
#include <TinyGPS++.h>
#include <Time.h>
#include <TimeLib.h>

// definindo as constantes do programa
const int S1 = 22; // sensores
const int S2 = 23;

const int ECHO = 24; // ultrassonico
const int TRIG = 25;

const float REF = 35.0; // referencia de altura de instalacao do sensor ultrassonico
const int GPSB = 9600; // definindo a velocidade de comunicação do módulo GPS
const unsigned long RATE = 3000; // intervalo de log de dados

int estado; // armazena o estado do sistema -> 0 - Normal | 1 - Nivel de alerta | 2 - Nivel de emergencia
unsigned long ultimo_log = 0; // armazena o tempo do ultimo log realizado

// inicialização dos objetos gps e us
TinyGPSPlus gps;
NewPing us(TRIG, ECHO);
//RF24 radio(9,10);
//const uint64_t pipe = 0xE8E8F0F0E1LL;

void setup()
{ 
  Serial.begin(115200);
  Serial3.begin(GPSB);
  radio.begin();
  radio.openWritingPipe(pipe);
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  Serial.println(F("Moppe"));
  Serial.println(F("Obtendo Dados de Localizacao"));
  Serial.println();  
}

void loop()
{
  float nv = dados_su(); // obtem a leitura do sensor ultrassonico

  // define o estado 0
  if((digitalRead(S1) == HIGH) && (digitalRead(S2) == LOW))
  {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    estado = 0;
  }

  // define o estado 1
  if((digitalRead(S1) == LOW) && (digitalRead(S2) == LOW))
  {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
    estado = 1;
  }

  // define o estado 2
  if((digitalRead(S1) == LOW) && (digitalRead(S2) == HIGH))
  {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    estado = 2;
  }

  // realiza o log dos dados
  if((ultimo_log + RATE) <= millis())
  {
    if ((gps.location.isValid() && gps.location.isUpdated()) && (gps.date.isValid() && gps.date.isUpdated()) && (gps.time.isValid() && gps.time.isUpdated()))
    {
      if(dados_log(estado, nv))
      {
        Serial.println(F("Dados logados."));
        Serial.println();
        ultimo_log = millis();
      }
      else
      {
        Serial.println(F("Falha ao fazer log de dados."));
      }
    }
    else{
      Serial.print(F("Sem dados do GPS. Satelites: "));
      Serial.println(gps.satellites.value());
    }
  }

  dados_gps(); // alimenta o objeto gps com dados
  
  delay(1); // estabiliza loop
}

// obtendo dados do sensor ultrassonico
float dados_su()
{
  long microsec = sonar.ping_median();
  float dist = us.convert_cm(microsec);
  float nivel = REF - dist;
  
  return nivel;
}

// alimentando o objeto gps com dados do módulo GPS
void dados_gps()
{
  while(Serial3.available())
    gps.encode(Serial3.read());
}

// funcao log de dados
byte dados_log(int estado, float nivel)
{
  // ajusta hora para GMT -3
  setTime(gps.time.hour(),gps.time.minute(),gps.time.second(),gps.date.day(),gps.date.month(),gps.date.year());
  adjustTime(-3 * SECS_PER_HOUR);
  
  // trata estado
  if(estado == 0)
    Serial.println("NIVEL NORMAL");
  if(estado == 1)
    Serial.println("NIVEL DE ALERTA");
  if(estado == 2)
    Serial.println("NIVEL DE EMERGENCIA");

  Serial.print(F("LEITURA ATUAl: "));
  Serial.print(nivel);
  Serial.println("cm");
  
  Serial.print(F("Localizacao: "));
  Serial.print(gps.location.lng(), 6);
  Serial.print(",");
  Serial.println(gps.location.lat(), 6);

  Serial.print(F("Data/Hora: "));
  Serial.print(gps.date.day());
  Serial.print("/");
  Serial.print(gps.date.month());
  Serial.print("/");
  Serial.print(gps.date.year());
  Serial.print(" | ");
  Serial.print(hour());
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.print(second());
  Serial.println();

  return 1;
}
