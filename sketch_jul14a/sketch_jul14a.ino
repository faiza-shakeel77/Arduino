#include "U8glib.h"
#define MOSFET_Pin 2
#define Bat_Pin A0
#define Res_Pin A1
#define Buzzer_Pin 9
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI  
float Capacity = 0.0; // Kapasite mAh
float Res_Value = 9.4;  // Dirençin Ohm Değeri
float Vcc = 4.64; // Arduino 5V Pin Voltajı (Multimetre ile ölçüldü)
float Current = 0.0; // Akım Amper
float mA=0;         // Akım mA
float Bat_Volt = 0.0;  // Batarya Gerilimi
float Res_Volt = 0.0;  // Dirençin alt ucundaki Voltaj Değeri
float Bat_High = 4.3; // Yüksek Batarya Voltajı
float Bat_Low = 0.9; // Düşük Batarya Voltajı
unsigned long previousMillis = 0; // ms (milli-second) Geçen Zaman
unsigned long millisPassed = 0;  // ms (milli-second) Şuanki Zaman
float sample1 =0;
float sample2= 0;
int x = 0;
int row = 0;

//************************ OLED Ekrana Yazdırma İşlevi *******************************************************
void draw(void) {
   u8g.setFont(u8g_font_fub14r); // Yazı Tipi Seçme
   if ( Bat_Volt < 0.5){
    u8g.setPrintPos(10,40);        // Pozisyon ayarlama
    u8g.println("No Battery!"); 
   }
   else if ( Bat_Volt > Bat_High){
    u8g.setPrintPos(25,40);        // Pozisyon ayarlama
    u8g.println("High-V!"); 
   }
   else if(Bat_Volt < Bat_Low){
    u8g.setPrintPos(25,40);        // Pozisyon ayarlama
    u8g.println("Low-V!"); 
   }
   else if(Bat_Volt >= Bat_Low && Bat_Volt < Bat_High  ){
      
   u8g.drawStr(0, 20, "Volt: ");   // X ve Y eksenlerinde ekrana string ekleme
   u8g.drawStr(0, 40, "Curr: ");
   u8g.drawStr(0, 60, "mAh: ");
   u8g.setPrintPos(58,20);        // Pozisyon ayarla
   u8g.print( Bat_Volt,2);  // Batarya Voltajını Volt olarak ekranda gösterme
   u8g.println("V"); 
   u8g.setPrintPos(58,40);        // Pozisyon ayarla
   u8g.print( mA,0);  // Akımı mA olarak ekranda gösterme
   u8g.println("mA"); 
   u8g.setPrintPos(58, 60);        // Pozisyon yarlama
   u8g.print( Capacity ,1);     // Kapasiteyi mAh olarak ekranda gösterme
  
}
}
//******************************Buzzer Bip Sesi İşlevi*********************************************************

  void beep(unsigned char delay_time){
  analogWrite(9, 20);      // Bip sesi oluşturmak için PWM sinyali
  delay(delay_time);          // deleayms'i bekle
  analogWrite(Buzzer_Pin, 0);  // 0 Bip sesini kapatır
  delay(delay_time);          // delayms'i bekle  

}  

//*******************************Kurulum İşlevi***************************************************************
  
  void setup() {
   Serial.begin(9600);
   pinMode(MOSFET_Pin, OUTPUT);
   pinMode(Buzzer_Pin, OUTPUT);
   digitalWrite(MOSFET_Pin, LOW);  // MOSFET Başlama Süresince Kapalı.
   Serial.println("CLEARDATA");
   Serial.println("LABEL,Time,Bat_Volt,capacity");
   
   // Send initial message over Bluetooth
   Serial.println("Bluetooth Initialized");
  }
  //********************************Ana Döngü İşlevi***********************************************************
  void loop() {
 // Vcc = readVcc()/1000.0; // mV'yi Volt'a dönüştür

  
  // Voltage devider Out = Bat_Volt * R2/(R1+R2 ) // R1 =10K and R2 =10K 
  
  //************ Batarya Voltajı Ölçümü ***********
  
  for(int i=0;i< 100;i++)
  {
   sample1=sample1+analogRead(Bat_Pin); //Bölücü devreden Voltajı okur
   delay (2);
  }
  sample1=sample1/100; 
  Bat_Volt = 2* sample1 *Vcc/ 1024.0; 

  // *********  Direnç Voltajı Ölçümü ***********

   for(int i=0;i< 100;i++)
  {
   sample2=sample2+analogRead(Res_Pin); //Bölücü Devre üzerinden voltajı okur
   delay (2);
  }
  sample2=sample2/100;
  Res_Volt = 2* sample2 * Vcc/ 1024.0;

  //********************* Farklı Koşulları Kontrol Etme *************
  
  if ( Bat_Volt > Bat_High){
    digitalWrite(MOSFET_Pin, LOW); // MOSFET kapatıldı // Deşarj Yok
    beep(200);
    Serial.println( "Warning High-V! ");
    delay(1000);
   }
   
   else if(Bat_Volt < Bat_Low){
      digitalWrite(MOSFET_Pin, LOW);
      beep(200);
      Serial.println( "Warning Low-V! ");
      delay(1000);
  }
  else if(Bat_Volt > Bat_Low && Bat_Volt < Bat_High  ) { // Batarya Voltajının güvenli limit içerisinde olup olmadığını kontrol etme
      digitalWrite(MOSFET_Pin, HIGH);
      millisPassed = millis() - previousMillis;
      Current = (Bat_Volt - Res_Volt) / Res_Value;
      mA = Current * 1000.0 ;
      Capacity = Capacity + mA * (millisPassed / 3600000.0); // 1 Hour = 3600000ms
      previousMillis = millis();
      Serial.print("DATA,TIME,"); Serial.print(Bat_Volt); Serial.print(","); Serial.println(Capacity);
      
      // Send data over Bluetooth
      Serial.print("Bat_Volt: "); Serial.print(Bat_Volt); Serial.print("V, ");
      Serial.print("Current: "); Serial.print(mA); Serial.print("mA, ");
      Serial.print("Capacity: "); Serial.print(Capacity); Serial.println("mAh");

      row++;
      x++;
      delay(4000); 
 
     }
  
   //**************************************************

  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
  
 //*************************************************
 } 
