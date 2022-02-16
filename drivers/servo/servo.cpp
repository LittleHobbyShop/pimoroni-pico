#include "servo.hpp"
#include <cstdio>

namespace servo {
  Calibration::CalibrationPoint::CalibrationPoint()
    : pulse(0.0f), value(0.0f) {
  }

  Calibration::CalibrationPoint::CalibrationPoint(uint16_t pulse, float value)
    : pulse(pulse), value(value) {
  }

  Calibration::Calibration(Type type)
    : calibration(nullptr), calibration_points(0), limit_lower(true), limit_upper(true) {
    create_default_calibration(type);
  }

  Calibration::~Calibration() {
    if(calibration != nullptr) {
      delete[] calibration;
      calibration = nullptr;
    }
  }

  void Calibration::create_default_calibration(Type type) {
    switch(type) {
    default:
    case ANGULAR:
      create_three_point_calibration(DEFAULT_MIN_PULSE, DEFAULT_MID_PULSE, DEFAULT_MAX_PULSE,
                                     -90.0f,            0.0f,              +90.0f);
      break;
    case LINEAR:
      create_two_point_calibration(DEFAULT_MIN_PULSE, DEFAULT_MAX_PULSE,
                                   0.0f,              1.0f);
      break;
    case CONTINUOUS:
      create_three_point_calibration(DEFAULT_MIN_PULSE, DEFAULT_MID_PULSE, DEFAULT_MAX_PULSE,
                                     -1.0f,            0.0f,              +1.0f);
      break;
    }
  }

  bool Calibration::create_blank_calibration(uint num_points) {
    bool success = false;
    if(num_points >= 2) {
      if(calibration != nullptr)
        delete[] calibration;
      
      calibration = new CalibrationPoint[num_points];
      calibration_points = num_points;
      success = true;
    }
    return success;
  }

  void Calibration::create_two_point_calibration(float min_pulse, float max_pulse, float min_value, float max_value) {
    create_blank_calibration(2);
    calibration[0] = CalibrationPoint(min_pulse, min_value);
    calibration[1] = CalibrationPoint(max_pulse, max_value);
  }

  void Calibration::create_three_point_calibration(float min_pulse, float mid_pulse, float max_pulse, float min_value, float mid_value, float max_value) {
    create_blank_calibration(3);
    calibration[0] = CalibrationPoint(min_pulse, min_value);
    calibration[1] = CalibrationPoint(mid_pulse, mid_value);
    calibration[2] = CalibrationPoint(max_pulse, max_value);
  }

  bool Calibration::create_uniform_calibration(uint num_points, float min_pulse, float min_value, float max_pulse, float max_value) {
    bool success = false;
    if(create_blank_calibration(num_points)) {
      float points_minus_one = (float)(num_points - 1);
      for(uint i = 0; i < num_points; i++) {
        float pulse = ((max_pulse - min_pulse) * (float)i) / points_minus_one;
        float value = ((max_value - min_value) * (float)i) / points_minus_one;
        calibration[i] = CalibrationPoint(pulse, value);
      }
      success = true;
    }
    return success;
  }

  uint Calibration::points() {
    return calibration_points;
  }

  bool Calibration::get_point(uint8_t index, CalibrationPoint& point_out) {
    bool success = false;
    if(index < calibration_points) {
      point_out = CalibrationPoint(calibration[index]);
      success = true;
    }

    return success;
  }

  void Calibration::set_point(uint8_t index, const CalibrationPoint& point) {
    if(index < calibration_points) {
      calibration[index] = CalibrationPoint(point);
    }
  }

  void Calibration::limit_to_calibration(bool lower, bool upper) {
    limit_lower = lower;
    limit_upper = upper;
  }

  float Converter::min_value() {
    float value = 0.0f;
    if(calibration_points >= 2) {
      value = calibration[0].value;
    }
    return value;
  }

  float Converter::mid_value() {
    float value = 0.0f;
    if(calibration_points >= 2) {
      value = (calibration[0].value + calibration[calibration_points - 1].value) / 2.0f;
    }
    return value;
  }

  float Converter::max_value() {
    float value = 0.0f;
    if(calibration_points >= 2) {
      value = calibration[calibration_points - 1].value;
    }
    return value;
  }

  float Converter::value_to_pulse(float value) {
    float pulse = 0.0f;
    if(calibration_points >= 2) {
      uint8_t last = calibration_points - 1;

      // Is the value below the bottom most calibration point?
      if(value < calibration[0].value) {
        // Should the value be limited to the calibration or projected below it?
        if(limit_lower)
          pulse = calibration[0].pulse;
        else
          pulse = map_float(value, calibration[0].value, calibration[1].value,
                                   calibration[0].pulse, calibration[1].pulse);
      }
      // Is the value above the top most calibration point?
      else if(value > calibration[last].value) {
        // Should the value be limited to the calibration or projected above it?
        if(limit_upper)
          pulse = calibration[last].pulse;
        else
          pulse = map_float(value, calibration[last - 1].value, calibration[last].value,
                                   calibration[last - 1].pulse, calibration[last].pulse);
      }
      else {
        // The value must between two calibration points, so iterate through them to find which ones
        for(uint8_t i = 0; i < last; i++) {
          if(value <= calibration[i + 1].value) {
            pulse = map_float(value, calibration[i].value, calibration[i + 1].value,
                                     calibration[i].pulse, calibration[i + 1].pulse);
            break; // No need to continue checking so break out of the loop
          }
        }
      }
    }

    return pulse;
  }

  float Converter::value_from_pulse(float pulse) {
    float value = 0.0f;
    if(calibration_points >= 2) {
      uint8_t last = calibration_points - 1;

      // Is the pulse below the bottom most calibration point?
      if(pulse < calibration[0].pulse) {
        // Should the pulse be limited to the calibration or projected below it?
        if(limit_lower)
          value = calibration[0].value;
        else
          value = map_float(pulse, calibration[0].pulse, calibration[1].pulse,
                                   calibration[0].value, calibration[1].value);
      }
      // Is the pulse above the top most calibration point?
      else if(pulse > calibration[last].pulse) {
        // Should the pulse be limited to the calibration or projected above it?
        if(limit_upper)
          value = calibration[last].value;
        else
          value = map_float(pulse, calibration[last - 1].pulse, calibration[last].pulse,
                                   calibration[last - 1].value, calibration[last].value);
      }
      else {
        // The pulse must between two calibration points, so iterate through them to find which ones
        for(uint8_t i = 0; i < last; i++) {
          if(pulse <= calibration[i + 1].pulse) {
            value = map_float(pulse, calibration[i].pulse, calibration[i + 1].pulse,
                                     calibration[i].value, calibration[i + 1].value);
            break; // No need to continue checking so break out of the loop
          }
        }
      }
    }

    return value;
  }

  uint32_t Converter::pulse_to_level(float pulse, uint32_t resolution) {
    // Constrain the level to hardcoded limits to protect the servo
    pulse = MIN(MAX(pulse, LOWER_HARD_LIMIT), UPPER_HARD_LIMIT);
    return (uint32_t)((pulse * (float)resolution) / SERVO_PERIOD);
  }

  float Converter::map_float(float in, float in_min, float in_max, float out_min, float out_max) {
    return (((in - in_min) * (out_max - out_min)) / (in_max - in_min)) + out_min;
  }

  Servo::Servo(uint pin, Type type)
    : pin(pin), converter(type) {
  }

  Servo::~Servo() {
    gpio_set_function(pin, GPIO_FUNC_NULL);
  }

  bool Servo::init() {
    pwm_cfg = pwm_get_default_config();
    pwm_config_set_wrap(&pwm_cfg, 20000 - 1);

    float div = clock_get_hz(clk_sys) / 1000000;
    pwm_config_set_clkdiv(&pwm_cfg, div);

    pwm_init(pwm_gpio_to_slice_num(pin), &pwm_cfg, true);
    gpio_set_function(pin, GPIO_FUNC_PWM);

    pwm_set_gpio_level(pin, 0);

    return true;
  }

  bool Servo::is_enabled() {
    return enabled;
  }

  void Servo::enable() {
    if(last_enabled_pulse < MIN_VALID_PULSE) {
      servo_value = converter.mid_value();
      last_enabled_pulse = converter.value_to_pulse(servo_value);
    }
    pwm_set_gpio_level(pin, (uint16_t)converter.pulse_to_level(last_enabled_pulse, 20000));
    enabled = true;
  }

  void Servo::disable() {
    pwm_set_gpio_level(pin, 0);
    enabled = false;
  }

  float Servo::get_value() {
    return servo_value;
  }

  void Servo::set_value(float value) {
    servo_value = value;
    float pulse = converter.value_to_pulse(value);
    if(pulse >= MIN_VALID_PULSE) {
      last_enabled_pulse = pulse;
      pwm_set_gpio_level(pin, (uint16_t)converter.pulse_to_level(last_enabled_pulse, 20000));
      enabled = true;
    }
    else {
      disable();
    }
  }

  float Servo::get_pulse() {
    return last_enabled_pulse;
  }

  void Servo::set_pulse(float pulse) {
    if(pulse >= MIN_VALID_PULSE) {
      servo_value = converter.value_from_pulse(pulse);
      last_enabled_pulse = pulse;
      pwm_set_gpio_level(pin, (uint16_t)converter.pulse_to_level(last_enabled_pulse, 20000));
      enabled = true;
    }
    else {
      disable();
    }
  }

  void Servo::to_min() {
    set_value(converter.min_value());
  }

  void Servo::to_mid() {
    set_value(converter.mid_value());
  }

  void Servo::to_max() {
    set_value(converter.max_value());
  }

  void Servo::to_percent(float in, float in_min, float in_max) {
    float value = Converter::map_float(in, in_min, in_max, converter.min_value(), converter.max_value());
    set_value(value);
  }

  void Servo::to_percent(float in, float in_min, float in_max, float value_min, float value_max) {
    float value = Converter::map_float(in, in_min, in_max, value_min, value_max);
    set_value(value);
  }

  Calibration& Servo::calibration() {
    return converter;
  }
};