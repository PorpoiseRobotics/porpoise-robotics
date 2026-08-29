/*
  Sensors.ino - the INA219 voltage and current sensor.

  Gen 3 vehicles carry one; Gen 2 vehicles do not. Everything in here is a
  no-op on a vehicle without the sensor, so the same binary runs on both.

  The registers are driven directly rather than through a library, because the
  whole driver is about forty lines and seeing the register writes is useful
  if you ever have to read the datasheet.
*/

// Is the sensor actually on the bus?
bool is_ina219_present() {
  Wire.beginTransmission(INA219_I2C_ADDR);
  return Wire.endTransmission() == 0;
}

void ina219_write16(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(INA219_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value >> 8);     // High byte first
  Wire.write(value & 0xFF);
  Wire.endTransmission();
}

uint16_t ina219_read16(uint8_t reg) {
  Wire.beginTransmission(INA219_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);          // Repeated start, do not release the bus
  Wire.requestFrom((uint8_t)INA219_I2C_ADDR, (uint8_t)2);
  return Wire.available() >= 2 ? (Wire.read() << 8) | Wire.read() : 0;
}

void ina219_init() {
  ina219_write16(0x00, 0x8000);   // Reset
  delay(1);
  ina219_write16(0x00, 0b0001100111001111);   // 32V range, 12-bit, continuous
  ina219_write16(0x05, 20999);                // Calibration for this shunt
}

void reset_current_tracking() {
  current_sum = 0.0f;
  current_count = 0;
  current_peak_pos = 0.0f;
  current_peak_neg = 0.0f;
}

/*
  Takes one reading and folds it into the running average and peaks.
  Called every pass of loop(), so "average" means "since the last reset".
*/
void update_ina219() {
  if (!(vehicle_config.capabilities & CAP_CURRENT_MON)) return;

  int16_t raw = (int16_t)ina219_read16(0x01);   // Shunt voltage register
  float current_a = (raw * 0.01f) / (SHUNT_RESISTOR_OHMS * 1000.0f);

  if (current_a > current_peak_pos) current_peak_pos = current_a;
  if (current_a < current_peak_neg) current_peak_neg = current_a;

  current_sum += current_a;
  current_count++;
}

float get_average_current() {
  return current_count > 0 ? current_sum / current_count : 0.0f;
}

float get_bus_voltage() {
  if (!(vehicle_config.capabilities & CAP_CURRENT_MON)) return 0.0f;
  uint16_t value = ina219_read16(0x02);
  return (float)((value >> 3) * 4) * 0.001f;   // Raw counts to volts
}
