#include <ServoTimer2.h>
#include <VirtualWire.h>
#include <sps30.h>

ServoTimer2 servoRoll;

#define rollPin  9

#define R1 2751
#define R2 991

int RF_RX_PIN = 12;

uint8_t buf[VW_MAX_MESSAGE_LEN];
uint8_t buflen = VW_MAX_MESSAGE_LEN;

const byte OUT1 = 5;
const byte OUT2 = 6;
const byte OUT3 = 3;
const byte OUT4 = 10;

const int ledPin = 11;
const int buzPin = 2;

int buttonState = 0;

int angle = 1000;

int rotR = 0;
int rotL = 0;


void setup()
{
  Serial.begin(9600);
  
  servoRoll.attach(rollPin);
  pinMode(ledPin, OUTPUT);
  pinMode(buzPin, OUTPUT);

  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);
  analogWrite(OUT1, 0);
  analogWrite(OUT2, 0);
  analogWrite(OUT3, 0);
  analogWrite(OUT4, 0);

  vw_set_rx_pin(RF_RX_PIN);
  vw_setup(2000);
  vw_rx_start(); 

  int16_t ret;
  uint8_t auto_clean = 4;
  
  sensirion_i2c_init();

    while (sps30_probe() != 0) {
    Serial.print("SPS sensor probing failed\n");
    delay(118);
    }
    ret = sps30_set_fan_auto_cleaning_interval_days(auto_clean);
    if (ret) {
    Serial.print("error setting the auto-clean interval: ");
    Serial.println(ret);
    }

    ret = sps30_start_measurement();
    if (ret < 0) {
    Serial.print("error starting measurement\n");
    }
}

void loop()
{
  
  sps_read_send_data();
  
  if (vw_get_message(buf, &buflen)) 
  {
    for (int i = 0; i < buflen; ++i)
    {
      Serial.print(buf[i]);
      Serial.print(" ");
    }
    if(buf[3] == 2){
      rotR = 1;
    }
    else if(buf[3] == 4){
      rotL = 1;
    }
    else{
      rotR = 0;
      rotL = 0;
    }
    Serial.println();
    motor_control(buf);
    led_buzzer(buf);
  }
  if(rotR == 1){
    if(angle != 2200){
      angle += 5;
      servoRoll.write(angle);
    }
  }
  if(rotL == 1){
    if(angle != 800){
      angle -= 5;
      servoRoll.write(angle);
    }  
  }
  Serial.println(angle);
}
void led_buzzer(uint8_t buf[VW_MAX_MESSAGE_LEN]) {
  if (buf[3] == 1) {
    if (buttonState == 0) {
      digitalWrite(ledPin, LOW);
      buttonState = 1;
    }
    else {
      digitalWrite(ledPin, HIGH);
      buttonState = 0  ;
    }
  }
  if (buf[3] == 3) {
    digitalWrite(buzPin, HIGH);
  }
  else if (buf[3] == 0) {
    digitalWrite(buzPin, LOW);
  }
  
}
int sps_read_send_data() {
  struct sps30_measurement m;
  char serial[SPS30_MAX_SERIAL_LEN];
  uint16_t data_ready;
  int16_t ret;

  ret = sps30_read_data_ready(&data_ready);
  if (ret < 0) {
    Serial.print("error reading data-ready flag: ");
    Serial.println(ret);
    return 1;
  }
  else if (!data_ready) {
    Serial.print("data not ready, no new measurement available\n");
    return 2;
  }
  ret = sps30_read_measurement(&m);
  if (ret < 0) {
    Serial.print("error reading measurement\n");
    return 3;
  } else {

    Serial.print("sps_start\n");
    Serial.print("PM 1.0: ");
    Serial.println(m.mc_1p0);
    Serial.print("PM 2.5: ");
    Serial.println(m.mc_2p5);
    Serial.print("PM 4.0: ");
    Serial.println(m.mc_4p0);
    Serial.print("PM 10.0: ");
    Serial.println(m.mc_10p0);

    Serial.print("NC  0.5: ");
    Serial.println(m.nc_0p5);
    Serial.print("NC  1.0: ");
    Serial.println(m.nc_1p0);
    Serial.print("NC  2.5: ");
    Serial.println(m.nc_2p5);
    Serial.print("NC  4.0: ");
    Serial.println(m.nc_4p0);
    Serial.print("NC 10.0: ");
    Serial.println(m.nc_10p0);

    Serial.print("Voltage: ");
    Serial.println(battery_voltage(), 2);
    Serial.print("sps_stop\n");
  }
}
float battery_voltage() {
  int sensorValue = analogRead(A0);
  float Vout = sensorValue * (5.0 / 1023.0);
  return (Vout * (R1 + R2)) / R2;
}
void motor_control(uint8_t buf[VW_MAX_MESSAGE_LEN]) {
 
    uint8_t X = buf[2];
    uint8_t Y = buf[1];

    Serial.print("X =");
    Serial.println(X);
    Serial.print("Y =");
    Serial.println(Y);
    
    int max_speed = 100; 
    
    int foward = map(X, 138, 255, 0, max_speed);
    int backward = map(X, 118, 0, 0, max_speed);
    int left = map(Y, 138, 255, 0, max_speed);
    int right = map(Y, 118, 0, 0, max_speed);
    
    if (X > 138 && Y < 138 && Y > 118) {
      analogWrite(OUT3, foward);
      analogWrite(OUT4, 0);
      analogWrite(OUT1, foward);
      analogWrite(OUT2, 0);
    } else if (X < 118 && Y < 138 && Y > 118) {
      analogWrite(OUT4, backward);
      analogWrite(OUT3, 0);
      analogWrite(OUT2, backward);
      analogWrite(OUT1, 0);
    } else if (X < 138 && X > 118 && Y < 138 && Y > 118) {
      analogWrite(OUT4, 0);
      analogWrite(OUT3, 0);
      analogWrite(OUT2, 0);
      analogWrite(OUT1, 0);
    }
    else if (X < 138 && X > 118 && Y > 138) {
      analogWrite(OUT4, 0);
      analogWrite(OUT3, left);
      analogWrite(OUT2, left);
      analogWrite(OUT1, 0);
    } else if (X < 138 && X > 118 && Y < 118) {
      analogWrite(OUT4, right);
      analogWrite(OUT3, 0);
      analogWrite(OUT2, 0);
      analogWrite(OUT1, right);
    } else if (X > 138 && Y > 138) {
      analogWrite(OUT3, foward);
      analogWrite(OUT4, 0);
      analogWrite(OUT1, foward - right);
      analogWrite(OUT2, 0);
    } else if (X > 138 && Y < 118) {
      analogWrite(OUT3, foward - left);
      analogWrite(OUT4, 0);
      analogWrite(OUT1, foward);
      analogWrite(OUT2, 0);
    } else if (X < 118 && Y > 138) {
      analogWrite(OUT4, backward);
      analogWrite(OUT3, 0);
      analogWrite(OUT2, backward - right);
      analogWrite(OUT1, 0);
    } else if (X < 118 && Y < 118) {
      analogWrite(OUT4, backward - left);
      analogWrite(OUT3, 0);
      analogWrite(OUT2, backward);
      analogWrite(OUT1, 0);
    }
}
