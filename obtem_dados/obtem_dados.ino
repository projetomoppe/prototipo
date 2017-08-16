#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <NewPing.h>
#include <TinyGPS++.h>

// definindo as constantes do programa
const int S1 = 22; // sensores
const int S2 = 23;

const int ECHO = 12; // ultrassonico
const int TRIG = 13;

const float REF = 35.0; // referencia de altura de instalacao do sensor ultrassonico
static const uint32_t GPSB = 9600; // definindo a velocidade de comunicação do módulo GPS
const unsigned long RATE = 3000; // intervalo de log de dados

float nivel = 0.0;
int estado;
unsigned long ultimo_envio = 0; // armazena o tempo do ultimo log realizado

const int CE_pin = 49; // modulo RF24
const int CS_pin = 48;

// inicialização dos objetos gps e us
TinyGPSPlus gps;
NewPing us(TRIG, ECHO);

RF24 radio(CE_pin, CS_pin);
const uint64_t pipe = 0xE8E8F0F0E1LL;

// estrutura de dados a ser enviada
struct comRF {
  int est;
  float nivel;
  double longitude;
  double latitude;
  uint32_t data;
  uint32_t hora;
  unsigned long packectID;
};

void setup()
{ 
  Serial.begin(115200);
  Serial3.begin(GPSB); // modulo GPS
  radio.begin();
  radio.openWritingPipe(pipe);
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);

  Serial.println(F("----------- PICJr - MOPPE -----------"));
  Serial.println(F("------- Programa inicializado -------"));
  Serial.println();  
}

void loop()
{
  estado = obtem_estado(); // obtem estado baseado na leitura dos sensores ICOS
  nivel  = dados_su();     // obtem a leitura do sensor ultrassonico

  // realiza o log dos dados
  if((ultimo_envio + RATE) <= millis())
  {   
    if ((gps.location.isValid()) && (gps.date.isValid()) && (gps.time.isValid()))
    {      
      struct comRF {
        int est = estado;
        float nivel = nv;
        double longitude = gps.location.lng();
        double latitude = gps.location.lat();
        uint32_t data = gps.date.value();
        uint32_t hora = gps.time.value();
      } dadosEnvio;
      
      Serial.println(sizeof(dadosEnvio));
      
      bool state = radio.write(&dadosEnvio, sizeof(dadosEnvio));

      if(state)
      {
        Serial.println(F("Dados enviados!"));
        Serial.println();
        ultimo_envio = millis();
      }
      else
        Serial.println(F("Falha no envio dos dados!"));
      
//      if(dados_envia(estado, nv))
//      {
//        Serial.println(F("Dados enviados."));
//        Serial.println();
//        ultimo_envio = millis();
//      }
//      else
//      {
//        Serial.println(F("Falha ao fazer log de dados."));
//      }
    }
    else{
      Serial.print(F("Sem dados do GPS. Satelites: "));
      Serial.println(gps.satellites.value());
    }
  }
  dados_gps(); // alimenta o objeto gps com dados
}

// obtendo dados do sensor ultrassonico
float dados_su()
{
// DATA SMOOTHING
//  const int num_leituras = 10;
//  float leituras[num_leituras];
//  int index = 0;
//  float total = 0.0;
//
//  for(int essa_leitura = 0; essa_leitura < num_leituras; essa_leitura++)
//    leituras[essa_leitura] = 0.0;
//
//  bool i = true;
//  while(i){
//    total = total - leituras[index];
//    leituras[index] = us.ping_cm();
//    total = total + leituras[index];
//    index++;
//  
//    if(index >= num_leituras)
//    {
//      index = 0;
//      i = false;
//    } 
//  }
  
//  long microsec = us.ping_median();
//  float dist = us.convert_cm(microsec);
  float dist = us.ping_cm();
  float nivel = REF - dist;
  
  return nivel;
}

// alimentando o objeto gps com dados do módulo GPS
void dados_gps()
{
  while(Serial3.available())
    gps.encode(Serial3.read());
}

// armazena o estado do sistema -> 0 - Normal | 1 - Nivel de alerta | 2 - Nivel de emergencia
int obtem_estado()
{
  // define o estado 1
  if((digitalRead(S1) == HIGH) && (digitalRead(S2) == LOW))
    estado = 0;

  // define o estado 1
  if((digitalRead(S1) == LOW) && (digitalRead(S2) == LOW))
    estado = 1;

  // define o estado 2
  if((digitalRead(S1) == LOW) && (digitalRead(S2) == HIGH))
    estado = 2;

  return estado;
}

// funcao log de dados
//byte dados_envia(int estado, float nivel)
//{   
//  // trata estado
//  if(estado == 0)
//    Serial.println("NIVEL NORMAL");
//  if(estado == 1)
//    Serial.println("NIVEL DE ALERTA");
//  if(estado == 2)
//    Serial.println("NIVEL DE EMERGENCIA");
//
//  Serial.print(F("LEITURA ATUAl: "));
//  Serial.print(nivel);
//  Serial.println("cm");
//  
//  Serial.print(F("Localizacao: "));
//  Serial.print(gps.location.lng(), 6);
//  Serial.print(",");
//  Serial.println(gps.location.lat(), 6);
//
//  Serial.print(F("Data/Hora: "));
//  Serial.print(gps.date.day());
//  Serial.print("/");
//  Serial.print(gps.date.month());
//  Serial.print("/");
//  Serial.print(gps.date.year());
//  Serial.print(" | ");
//  Serial.print(gps.time.hour());
//  Serial.print(":");
//  Serial.print(gps.time.minute());
//  Serial.print(":");
//  Serial.print(gps.time.second());
//  Serial.println();

//  return 1;
//}
