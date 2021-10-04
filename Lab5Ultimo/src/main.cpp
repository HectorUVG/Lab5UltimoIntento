//*****************************************************************************
// Universidad del Valle de Guatemala
// BE3015 - Electrónica Digital 2
// Hector de Leon
// Lab5
//*****************************************************************************

//*****************************************************************************
// Librerías
//*****************************************************************************

#include <Arduino.h>

#include <LiquidCrystal.h>

//*****************************************************************************
// Definición de pines
//*****************************************************************************
#define d4 19//los GPIO 1 y 3 se dejan libres porque son los que usa la 
//comnicacion serial
#define d5 21
#define d6 22
#define d7 23
#define en 16
#define rs 0

#define pot1 36
#define pot2 26

#define Boton1 14
#define Boton2 13

//leds
#define ledR 25
#define ledG 33
#define ledB 32
#define pwmChannelR 5 // 16 canales 0-15
#define pwmChannelG 4
#define pwmChannelB 3
#define freqPWM 5000 // Frecuencia en Hz
#define resolution 8 // 1-16 bits de resolución

//*****************************************************************************
// Prototipos de función
//*****************************************************************************
void separarvariables1(void);
void separarvariables2(void);
void contBitsSuma(void);
void configurarPWM(void);
void leds(void);
void comSerial(void);
void CentenasDecenasUnidades(void);
//*****************************************************************************
// Variables Globales
//*****************************************************************************
// Create An LCD Object. Signals: [ RS, EN, D4, D5, D6, D7 ]
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int centenas = 0;
int decenas = 0;
int unidades = 0;
int adcRaw;
float voltaje1;
int volt1;
float voltaje2;
int volt2;

//variables para valores de los leds
float potLedR = 0;
float potLedG = 0;

//Instanciar timer
hw_timer_t *timer = NULL;
//variable para el contador del timer
int ISRB1 = 0;
int ISRB2 = 0;
int contadorDisplay = 0;

//delay1
unsigned long lastTime;
unsigned int sampleTime = 500;//el delay dura 500 milisegundos

//USART
char inByte = 0;
String mensaje = "";


//*****************************************************************************
//ISR
//*****************************************************************************


//interrupcion para el boton1
void IRAM_ATTR boton1()
{
  ISRB1 = 1;
}

//interrupcion para el boton2
void IRAM_ATTR boton2()
{
  ISRB2 = 1;
}
//*****************************************************************************
// Configuración
//*****************************************************************************

void setup() {
  Serial.begin(115200);

  //leds
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  digitalWrite(ledR, 0);
  digitalWrite(ledG, 0);
  //boton1
  pinMode(Boton1, INPUT_PULLUP);
  attachInterrupt(Boton1, boton1, RISING);

  //boton2
  pinMode(Boton2, INPUT_PULLUP);
  attachInterrupt(Boton2, boton2, RISING);

  //pwm
  configurarPWM();
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Pot1:");
  lcd.setCursor(6, 0);
  lcd.print("Pot2:");
  lcd.setCursor(12, 0);
  lcd.print("CPU");
}
//*****************************************************************************
// Loop principal
//*****************************************************************************

void loop() {
  contBitsSuma();//hace la suma y resta con los botones
  separarvariables1();
  separarvariables2();
  leds();
  comSerial();
  CentenasDecenasUnidades();
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(voltaje1);

  lcd.setCursor(6, 1);
  // print the number of seconds since reset:
  lcd.print(voltaje2);

  lcd.setCursor(12, 1);
  // imprime el valir de la centena
  lcd.print(centenas);

  lcd.setCursor(13, 1);
   // imprime el valir de la decena
  lcd.print(decenas);

  lcd.setCursor(14, 1);
   // imprime el valir de la unidad
  lcd.print(unidades);
}
//*****************************************************************************
//*****************************************************************************
// funciones
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
// funcione para configurar el PWM
//*****************************************************************************
void configurarPWM(void)
{

  // Paso 1: Configurar el módulo PWM
  ledcSetup(pwmChannelR, freqPWM, resolution);//ledR
  ledcSetup(pwmChannelG, freqPWM, resolution);//legG
  ledcSetup(pwmChannelB, freqPWM, resolution);//ledB

  // Paso 2: seleccionar en que GPIO tendremos nuestra señal PWM
  ledcAttachPin(ledR, pwmChannelR);
  ledcAttachPin(ledG, pwmChannelG);
  ledcAttachPin(ledB, pwmChannelB);

}

//*****************************************************************************
//Funcion Leds
//*****************************************************************************
void leds()
{
  
  /**
   * ya que el led RGB es de anodo comun, se debe colocar 0 en el ledcWrite 
   * que se desea encender .
   * Esta funcion la saque de la que use en el codigo de mi proyecto1 ubicado 
   * en pruebasproyecto.
   */
  ledcWrite(pwmChannelR, potLedR);//se muestra color verde
  ledcWrite(pwmChannelG, potLedG);
  ledcWrite(pwmChannelB, contadorDisplay);
}

//*****************************************************************************
//Funcion para leer el voltaje del pot1
//*****************************************************************************
void separarvariables1(){
  voltaje1 = analogReadMilliVolts(pot1)/1000.0 ;//al dividirlo da el valor en 
  //volios
  potLedR = (voltaje1/3.15)*255;//divide voltaje1 para obtener un valor entre
  //0 y 1 y lo multiplica por 255 para obtener valores entre 0 y 255 para el 
  //led 


}

//*****************************************************************************
//Funcion para leer el voltaje del pot1
//*****************************************************************************
void separarvariables2(){
  voltaje2 = analogReadMilliVolts(pot2)/1000.0;//al dividirlo da el valor en 
  //volios

  potLedG = (voltaje2/3.14)*255;//divide voltaje2 para obtener un valor entre
  //0 y 1 y lo multiplica por 255 para obtener valores entre 0 y 255 para el 
  //led

}

//*****************************************************************************
//Funcion para realizar la suma y resta con los botones
//*****************************************************************************
void contBitsSuma(){
  if ((ISRB1 == 1 or mensaje == "+") && contadorDisplay < 255){
    if(millis() - lastTime >= sampleTime){
      lastTime = millis(); //el if es un delay de 500milisegundos
      contadorDisplay = contadorDisplay + 1 ;
      ISRB1 = 0;
      mensaje = "";
    }  
  }

  if ((ISRB1 == 1 or mensaje == "+") && contadorDisplay >= 255){
    if(millis() - lastTime >= sampleTime){
      lastTime = millis(); //el if es un delay de 500milisegundos  
      contadorDisplay = 0 ;
      ISRB1 = 0;
      mensaje = "";
    }  
  }

  if ((ISRB2 > 0 or mensaje == "-") && contadorDisplay > 0){
    if(millis() - lastTime >= sampleTime){
      lastTime = millis(); //el if es un delay de 500milisegundos
      contadorDisplay = contadorDisplay - 1 ;
      ISRB2 = 0;
      mensaje = "";
    }  
  }
  if ((ISRB2 == 1 or mensaje == "-") && contadorDisplay <= 0){
    if(millis() - lastTime >= sampleTime){
      lastTime = millis(); //el if es un delay de 500milisegundos  
      contadorDisplay = 255 ;
      ISRB2 = 0;
      mensaje = "";
    }  
  }  
}

//*****************************************************************************
// funcion para comunicacion serial
//*****************************************************************************
void comSerial(){
  //Serial.print("Ingresar dato: ");
  if(Serial.available()>0){
  
    
    //inByte = Serial.read();  // Lectura por bytes
    mensaje = Serial.readStringUntil('\n');
    
    //Serial.print("Recibi el siguiente dato: ");
    //Serial.println(inByte);

    Serial.print("Recibi el siguiente mensaje: ");
    Serial.println(mensaje);
  }
}

//*****************************************************************************
// funcion para decenas centenas y unidades
//*****************************************************************************
void CentenasDecenasUnidades(){
  
  int temp = contadorDisplay;
  centenas = temp/100;
  temp = temp - (centenas*100);
  decenas = temp/10;
  temp = temp - (decenas*10);
  unidades = temp;


}