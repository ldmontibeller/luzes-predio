//Definiçao da quantidade do incremento dos fades, ver função sequencia();
#define QTD_1 10
#define QTD_2 10

#include <SoftwareSerial.h>
#include <TimerFive.h>

SoftwareSerial bluetooth(51, 53); // TX, RX ( of the Bluetooth, respectively)

//Variáveis para funcionamento dos fades;
volatile unsigned long tempoAnterior1;
volatile unsigned long tempoAnterior2;
volatile unsigned long tempoAnterior3;
volatile unsigned long tempoAtual;
const unsigned long periodo1 = 5;
const unsigned long periodo2 = 50;
const unsigned long periodo3 = 10;


volatile int brilhoFadePisca1 = 0;
volatile int brilhoFadePisca2 = 0;
volatile int quantidadeFadePisca1 = QTD_1;
volatile int quantidadeFadePisca2 = QTD_2;

volatile boolean piscaRodando = false;

String inData = "";
unsigned const int tempoFadeIn = 75;
unsigned const int tempoFadeOut = 50;

void setup() {
  bluetooth.begin(9600);

  Serial.begin(9600);

  for(uint8_t pin = 2; pin <10; pin++)
  {
    pinMode(pin,OUTPUT);
    digitalWrite(pin, LOW);
  }

  tempoAnterior1 = millis();
  tempoAnterior2 = millis();
  tempoAnterior3 = millis();

  Timer5.initialize(1000);
}

void loop() {
  while(bluetooth.available() > 0) 
  {  
    //Recebe um caractére
    char receivedChar = bluetooth.read();
    
    // Adiciona caracteres a string até receber o terminador
    inData += receivedChar;
    
    // Verifica se tem o tamanho máximo de um comando
    if(inData.length() > 11)
    { 
      Serial.println("Erro: maior que 11 caracteres.");
      Serial.println(inData);
      inData = "";
      break;
    }
    //Verifica se possui o terminador \n
    if(receivedChar == '\n')
    { 
      //Serial.println(inData);

      //Verifica se é um comando de 11 caracteres
      if (inData.length() == 11) 
      {
        setBrightness();
      }
      else if (inData.length() == 7)
      {
        fade();
      }
      else if (inData.length() == 4)
      {
        todos();
        pisca();
        encerramento();
      }
      else {
        Serial.print("Não é um comando válido: ");
        Serial.println(inData);
        inData = "";
        break;
      }
    }
  }
}

void setBrightness() 
{
  while(true)
  {  
    String command = inData.substring(0, 2);
    if(command != "SB") {
      Serial.println("Erro: não é SB.");
      Serial.println(inData);
      inData = "";
      break;
    }
    
    String ledNumber = inData.substring(3, 5);  
    int _ledNumber = ledNumber.toInt();
    
    if (_ledNumber == 0) {
      Serial.println("Erro: número do led retornou zero.");
      Serial.println(ledNumber);
      inData = "";
      break;
    }
    
    String pwmValue = inData.substring(6, 9); 
    int _pwmValue = pwmValue.toInt();
    
    if (_pwmValue > 255) 
    {
      Serial.print("Erro: PWM maior que 255.");
      Serial.println(pwmValue);
      inData = "";
      break;
    }
    
    analogWrite(_ledNumber, _pwmValue);
    break;
  }
}

void fade()
{
  while(true)
  {
    String command = inData.substring(0, 2);
    if((command != "FI")&&(command != "FO")) {
      Serial.println("Erro: não é fade.");
      Serial.println(inData);
      inData = "";
      break;
    }
    
    String ledNumber = inData.substring(3, 5);  
    int _ledNumber = ledNumber.toInt();
    
    if (_ledNumber == 0) {
      Serial.println("Erro: número do led retornou zero.");
      Serial.println(ledNumber);
      inData = "";
      break;
    }
    
  if(command == "FI")
    {  
      //Função de fade in
      for(uint8_t i = 0; i < 255; i+=5)
        {
          analogWrite(_ledNumber, i);
          delay(tempoFadeIn); 
        }
      analogWrite(_ledNumber, 255);
      inData = "";
      break;
    }

     if(command == "FO")
    {  
      //Função de fade out
      for(uint8_t i = 255; i > 0; i-=5)
        {
          analogWrite(_ledNumber, i);
          delay(tempoFadeOut); 
        }
      analogWrite(_ledNumber, 0);
      inData = "";
      break;
    }

    Serial.println(inData);
    inData = "";
    break;   
  }
}

void todos()
{
  while(true)
  {
    String command = inData.substring(0, 2);

    if(command == "LT")
    {  
      //Função de ligar todos
      for(uint8_t pin = 2; pin < 10; pin++)
      {
        digitalWrite(pin, HIGH);
      }
      inData = "";
      break;
    }

    if(command == "DT")
    {  
      //Função de desligar todos
      for(uint8_t pin = 2; pin < 10; pin++)
      {
        digitalWrite(pin, LOW);
      }
      inData = "";
      break;
    }
    break;
  }
}

void pisca()
{
  while (true)
  { 
    String comando = inData.substring(0, 2);
    if((comando == "PL")&&(piscaRodando == false))
    { 
      //Inicialização do programa
      piscaRodando = true;

      //Reseta todos os LEDs
      for(uint8_t pin = 2; pin < 10; pin++)
      {
        digitalWrite(pin, LOW);
      }

      //Roda a sequencia programada
      Timer5.attachInterrupt(sequencia);

      //Serial.println(inData);
      inData = "";
      break;
    } 
    else if((comando == "PD")&&(piscaRodando == true))
    { 
      Timer5.detachInterrupt();
      //Termina com todos acesos
      for(uint8_t pin = 2; pin < 10; pin++)
      {
        digitalWrite(pin, HIGH);
      }
      
      //Reseta os valores de brilho da função
      brilhoFadePisca1 = 0;
      brilhoFadePisca2 = 0;
      quantidadeFadePisca1 = QTD_1;
      quantidadeFadePisca2 = QTD_2;

      //Reseta a flag de uso do comando;
      piscaRodando = false;

      //Serial.println(inData);
      inData = "";
      break;
    }else if(((comando == "PL")&&(piscaRodando == true))||((comando == "PD")&&(piscaRodando == false)))
    {
      Serial.println("O comando PL foi enviado enquanto andamento ou PD sem ser após PL");
      inData = "";
      break;
    }
    break;
  }
}

void encerramento()
{
  while (true)
  {
    String command = inData.substring(0, 2);
    if (command == "EN")
    { 
      //Garantir que todos estavam ligados
      for(uint8_t pin = 2; pin < 10; pin++)
      {
        digitalWrite(pin, HIGH);
      }

      for(uint8_t i = 255; i > 0; i-=5)
      {
        analogWrite(5, i);
        delay(tempoFadeOut); 
      }
      digitalWrite(5, LOW);
      for(uint8_t i = 255; i > 0; i-=5)
      {
        analogWrite(8, i);
        delay(tempoFadeOut); 
      }
      digitalWrite(8, LOW);
      for(uint8_t i = 255; i > 0; i-=5)
      {
        analogWrite(2, i);
        delay(tempoFadeOut); 
      }
      digitalWrite(2, LOW);
      for(uint8_t i = 255; i > 0; i-=5)
      {
        analogWrite(7, i);
        delay(tempoFadeOut); 
      }
      digitalWrite(7, LOW);
      for(uint8_t i = 255; i > 0; i-=5)
      {
        analogWrite(3, i);
        delay(tempoFadeOut); 
      }
      digitalWrite(3, LOW);
      for(uint8_t i = 255; i > 0; i-=5)
      {
        analogWrite(4, i);
        delay(tempoFadeOut); 
      }
      digitalWrite(4, LOW);
      for(uint8_t i = 255; i > 0; i-=5)
      {
        analogWrite(9, i);
        delay(tempoFadeOut); 
      }
      digitalWrite(9, LOW);
      for(uint8_t i = 255; i > 0; i-=5)
      {
        analogWrite(6, i);
        delay(tempoFadeOut); 
      }
      digitalWrite(6, LOW);
      inData = "";
      break;
    }
    break;
  }
}

void sequencia()
{
  tempoAtual = millis();

  //Sequência de piscamento
  if(tempoAtual - tempoAnterior1 >= periodo1)
  {
    analogWrite(2, brilhoFadePisca1);
    analogWrite(5, brilhoFadePisca1);
    analogWrite(8, brilhoFadePisca1);

    // muda o brilho na próxima vez do loop:
    brilhoFadePisca1 = brilhoFadePisca1 + quantidadeFadePisca1;
    // muda a direção do fade no final do loop:
    if (brilhoFadePisca1 <= 0 || brilhoFadePisca1 >= 255) {
      quantidadeFadePisca1 = -quantidadeFadePisca1;
    }
    tempoAnterior1 = tempoAtual;
  }
  if(tempoAtual - tempoAnterior2 >= periodo2)
  {
    digitalWrite(3, !digitalRead(3));
    digitalWrite(6, !digitalRead(6));
    digitalWrite(9, !digitalRead(9));
    tempoAnterior2 = tempoAtual;
  }
  if(tempoAtual - tempoAnterior3 >= periodo3)
  {

    analogWrite(4, brilhoFadePisca2);
    analogWrite(7, brilhoFadePisca2);
    
    // muda o brilho na próxima vez do loop:
    brilhoFadePisca2 = brilhoFadePisca2 + quantidadeFadePisca2;
    // muda a direção do fade no final do loop:
    if (brilhoFadePisca2 <= 0 || brilhoFadePisca2 >= 255) {
      quantidadeFadePisca2 = -quantidadeFadePisca2;
    }
    tempoAnterior3 = tempoAtual;
  }
}

