#pragma once

#include <stdint.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "common/pimoroni_common.hpp"

namespace servo {

  enum Type {
    ANGULAR = 0,
    LINEAR,
    CONTINUOUS
  };

  class Calibration {
    //--------------------------------------------------
    // Constants
    //--------------------------------------------------
  public:
    static constexpr float DEFAULT_MIN_PULSE = 500.0f;   // in microseconds
    static constexpr float DEFAULT_MID_PULSE = 1500.0f;  // in microseconds
    static constexpr float DEFAULT_MAX_PULSE = 2500.0f;  // in microseconds
    static constexpr float DEFAULT_VALUE_EXTENT = 90.0f; // a range of -90 to +90


    //--------------------------------------------------
    // Substructures
    //--------------------------------------------------
  public:
    struct CalibrationPoint {
        //--------------------------------------------------
        // Constructors/Destructor
        //--------------------------------------------------
        CalibrationPoint();
        CalibrationPoint(uint16_t pulse, float value);


        //--------------------------------------------------
        // Variables
        //--------------------------------------------------
        float pulse;
        float value;
    };

    
    //--------------------------------------------------
    // Constructors/Destructor
    //--------------------------------------------------
  protected:
    Calibration(Type type);
    virtual ~Calibration();


    //--------------------------------------------------
    // Methods
    //--------------------------------------------------
  public:
    void create_default_calibration(Type type);
    bool create_blank_calibration(uint num_points); // Must have at least two points
    void create_two_point_calibration(float min_pulse, float max_pulse, float min_value, float max_value);
    void create_three_point_calibration(float min_pulse, float mid_pulse, float max_pulse, float min_value, float mid_value, float max_value);
    bool create_uniform_calibration(uint num_points, float min_pulse, float min_value, float max_pulse, float max_value); // Must have at least two points

    uint points();
    bool get_point(uint8_t index, CalibrationPoint& point_out);
    void set_point(uint8_t index, const CalibrationPoint& point); // Ensure the points are entered in ascending value order

    void limit_to_calibration(bool lower, bool upper);


    //--------------------------------------------------
    // Variables
    //--------------------------------------------------
  protected:
    CalibrationPoint* calibration;
    uint calibration_points;
    bool limit_lower;
    bool limit_upper;
  };

  class Converter : public Calibration {
    //--------------------------------------------------
    // Constants
    //--------------------------------------------------
  private:
    static constexpr float LOWER_HARD_LIMIT = 500.0f;   // The minimum microsecond pulse to send
    static constexpr float UPPER_HARD_LIMIT = 2500.0f;  // The maximum microsecond pulse to send
    static constexpr float SERVO_PERIOD = 1000000 / 50;    // This is hardcoded as all servos *should* run at this frequency


    //--------------------------------------------------
    // Constructors/Destructor
    //--------------------------------------------------
  public:
    Converter(Type type) : Calibration(type) {}
    virtual ~Converter() {}


    //--------------------------------------------------
    // Methods
    //--------------------------------------------------
  public:
    float min_value();
    float mid_value();
    float max_value();
    float value_to_pulse(float value);
    float value_from_pulse(float pulse);

    static uint32_t pulse_to_level(float pulse, uint32_t resolution);
    static float map_float(float in, float in_min, float in_max, float out_min, float out_max);
  };

  class Servo {
    //--------------------------------------------------
    // Constants
    //--------------------------------------------------
  public:
    static const uint16_t DEFAULT_PWM_FREQUENCY = 50;       //The standard servo update rate

  private:
    static const uint32_t MAX_PWM_WRAP = UINT16_MAX;
    static constexpr uint16_t MAX_PWM_DIVIDER = (1 << 7);

    static constexpr float MIN_VALID_PULSE = 1.0f;


    //--------------------------------------------------
    // Variables
    //--------------------------------------------------
  private:
    uint pin;
    pwm_config pwm_cfg;
    uint16_t pwm_period;
    float pwm_frequency = DEFAULT_PWM_FREQUENCY;

    float servo_value = 0.0f;
    float last_enabled_pulse = 0.0f;
    bool enabled = false;

    Converter converter;


    //--------------------------------------------------
    // Constructors/Destructor
    //--------------------------------------------------
  public:
    Servo(uint pin, Type type = ANGULAR);
    ~Servo();

    //--------------------------------------------------
    // Methods
    //--------------------------------------------------
  public:
    bool init();

    bool is_enabled();
    void enable();
    void disable();

    float get_value();
    void set_value(float value);

    float get_pulse();
    void set_pulse(float pulse);

    void to_min();
    void to_mid();
    void to_max();
    void to_percent(float in, float in_min = 0.0f, float in_max = 1.0f);
    void to_percent(float in, float in_min, float in_max, float value_min, float value_max);

    Calibration& calibration();
  };

}