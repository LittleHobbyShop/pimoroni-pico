#include "../../../libraries/breakout_tof/breakout_tof.hpp"

#define MP_OBJ_TO_PTR2(o, t) ((t *)(uintptr_t)(o))

// SDA/SCL on even/odd pins, I2C0/I2C1 on even/odd pairs of pins.
#define IS_VALID_SCL(i2c, pin) (((pin) & 1) == 1 && (((pin) & 2) >> 1) == (i2c))
#define IS_VALID_SDA(i2c, pin) (((pin) & 1) == 0 && (((pin) & 2) >> 1) == (i2c))


using namespace pimoroni;

extern "C" {
#include "breakout_tof.h"

/***** Variables Struct *****/
typedef struct _breakout_tof_BreakoutTOF_obj_t {
    mp_obj_base_t base;
    BreakoutTOF *breakout;
} breakout_tof_BreakoutTOF_obj_t;

/***** Print *****/
void BreakoutTOF_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind; //Unused input parameter    
    breakout_tof_BreakoutTOF_obj_t *self = MP_OBJ_TO_PTR2(self_in, breakout_tof_BreakoutTOF_obj_t);
    BreakoutTOF* breakout = self->breakout;
    mp_print_str(print, "BreakoutTOF(");

    mp_print_str(print, "i2c = ");
    mp_obj_print_helper(print, mp_obj_new_int((breakout->get_i2c() == i2c0) ? 0 : 1), PRINT_REPR);

    mp_print_str(print, ", sda = ");
    mp_obj_print_helper(print, mp_obj_new_int(breakout->get_sda()), PRINT_REPR);

    mp_print_str(print, ", scl = ");
    mp_obj_print_helper(print, mp_obj_new_int(breakout->get_scl()), PRINT_REPR);

    mp_print_str(print, ", int = ");
    mp_obj_print_helper(print, mp_obj_new_int(breakout->get_int()), PRINT_REPR);

    mp_print_str(print, ")");
}

/***** Constructor *****/
mp_obj_t BreakoutTOF_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    breakout_tof_BreakoutTOF_obj_t *self = nullptr;

    if(n_args == 0) {
        mp_arg_check_num(n_args, n_kw, 0, 0, true);
        self = m_new_obj(breakout_tof_BreakoutTOF_obj_t);
        self->base.type = &breakout_tof_BreakoutTOF_type;
        self->breakout = new BreakoutTOF();
    }
    else if(n_args == 1) {
        enum { ARG_address };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_address, MP_ARG_REQUIRED | MP_ARG_INT },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        self = m_new_obj(breakout_tof_BreakoutTOF_obj_t);
        self->base.type = &breakout_tof_BreakoutTOF_type;
        
        self->breakout = new BreakoutTOF(args[ARG_address].u_int);     
    }
    else {
        enum { ARG_i2c, ARG_address, ARG_sda, ARG_scl, ARG_int };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_i2c, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_address, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_sda, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_scl, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_int, MP_ARG_INT, {.u_int = BreakoutTOF::PIN_UNUSED} },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        // Get I2C bus.
        int i2c_id = args[ARG_i2c].u_int;
        if(i2c_id < 0 || i2c_id > 1) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("I2C(%d) doesn't exist"), i2c_id);
        }

        int sda = args[ARG_sda].u_int;
        if (!IS_VALID_SDA(i2c_id, sda)) {
            mp_raise_ValueError(MP_ERROR_TEXT("bad SDA pin"));
        }

        int scl = args[ARG_scl].u_int;
        if (!IS_VALID_SCL(i2c_id, scl)) {
            mp_raise_ValueError(MP_ERROR_TEXT("bad SCL pin"));
        }        

        self = m_new_obj(breakout_tof_BreakoutTOF_obj_t);
        self->base.type = &breakout_tof_BreakoutTOF_type;
        
        i2c_inst_t *i2c = (i2c_id == 0) ? i2c0 : i2c1;
        self->breakout = new BreakoutTOF(i2c, args[ARG_address].u_int, sda, scl, args[ARG_int].u_int);
    }

    self->breakout->init();

    return MP_OBJ_FROM_PTR(self);
}

mp_obj_t BreakoutTOF_get_id(mp_obj_t self_in) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_get_osc(mp_obj_t self_in) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_set_osc(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_set_address(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_get_address(mp_obj_t self_in) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_set_distance_mode(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_set_distance_mode_int(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_get_distance_mode(mp_obj_t self_in) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_set_measurement_timing_budget(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_get_measurement_timing_budget(mp_obj_t self_in) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_start_continuous(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_stop_continuous(mp_obj_t self_in) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_read(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_read_range_continuous_millimeters(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_read_single(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_read_range_single_millimeters(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_data_ready(mp_obj_t self_in) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

//TODO Below is a static in C, so may want to be global?
mp_obj_t BreakoutTOF_range_status_to_string(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_set_timeout(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_get_timeout(mp_obj_t self_in) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_timeout_occurred(mp_obj_t self_in) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}

mp_obj_t BreakoutTOF_get_curr_ms(mp_obj_t self_in) {
    mp_raise_NotImplementedError("not implemented. Please avoid using this function for now");  //TODO

    return mp_const_none;
}
}