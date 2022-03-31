#include <sps30.h>

void setup() {
  int16_t ret;
  uint8_t auto_clean_days = 4;
  uint32_t auto_clean;

  Serial.begin(9600);
  delay(2000);

  sensirion_i2c_init();

  while (sps30_probe() != 0) {
    Serial.print("SPS sensor probing failed\n");
    delay(500);
  }
  ret = sps30_set_fan_auto_cleaning_interval_days(auto_clean_days);
  if (ret) {
    Serial.print("error setting the auto-clean interval: ");
    Serial.println(ret);
  }

  ret = sps30_start_measurement();
  if (ret < 0) {
    Serial.print("error starting measurement\n");
  }


  delay(1000);
}

void loop() {
  struct sps30_measurement m;
  char serial[SPS30_MAX_SERIAL_LEN];
  uint16_t data_ready;
  int16_t ret;
  
  ret = sps30_read_data_ready(&data_ready);
  if (ret < 0) {
    Serial.print("error reading data-ready flag: ");
    Serial.println(ret);
  } 
  else if (!data_ready){
    Serial.print("data not ready, no new measurement available\n");
  }
  ret = sps30_read_measurement(&m);
  if (ret < 0) {
    Serial.print("error reading measurement\n");
  } else {

    Serial.print("sps_start\n");
    Serial.print("PM1.0: ");
    Serial.println(m.mc_1p0);
    Serial.print("PM2.5: ");
    Serial.println(m.mc_2p5);
    Serial.print("PM4.0: ");
    Serial.println(m.mc_4p0);
    Serial.print("PM10.0: ");
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

    Serial.print("Typical partical size: ");
    Serial.println(m.typical_particle_size);
    Serial.print("sps_stop\n");
  }
}
