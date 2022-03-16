#include "libraries/breakout_paa5100/breakout_paa5100.hpp"

#define MP_OBJ_TO_PTR2(o, t) ((t *)(uintptr_t)(o))

#define IS_VALID_PERIPH(spi, pin)   ((((pin) & 8) >> 3) == (spi))
#define IS_VALID_SCK(spi, pin)      (((pin) & 3) == 2 && IS_VALID_PERIPH(spi, pin))
#define IS_VALID_MOSI(spi, pin)     (((pin) & 3) == 3 && IS_VALID_PERIPH(spi, pin))
#define IS_VALID_MISO(spi, pin)     (((pin) & 3) == 0 && IS_VALID_PERIPH(spi, pin))


using namespace pimoroni;

extern "C" {
#include "breakout_paa5100.h"
#include <cstring>

/***** Variables Struct *****/
typedef struct _breakout_paa5100_BreakoutPAA5100_obj_t {
    mp_obj_base_t base;
    BreakoutPAA5100 *breakout;
} breakout_paa5100_BreakoutPAA5100_obj_t;

/***** Print *****/
void BreakoutPAA5100_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind; //Unused input parameter
    breakout_paa5100_BreakoutPAA5100_obj_t *self = MP_OBJ_TO_PTR2(self_in, breakout_paa5100_BreakoutPAA5100_obj_t);
    BreakoutPAA5100* breakout = self->breakout;
    mp_print_str(print, "BreakoutPAA5100(");

    mp_print_str(print, "spi = ");
    mp_obj_print_helper(print, mp_obj_new_int((breakout->get_spi() == spi0) ? 0 : 1), PRINT_REPR);

    mp_print_str(print, ", cs = ");
    mp_obj_print_helper(print, mp_obj_new_int(breakout->get_cs()), PRINT_REPR);

    mp_print_str(print, ", sck = ");
    mp_obj_print_helper(print, mp_obj_new_int(breakout->get_sck()), PRINT_REPR);

    mp_print_str(print, ", mosi = ");
    mp_obj_print_helper(print, mp_obj_new_int(breakout->get_mosi()), PRINT_REPR);

    mp_print_str(print, ", miso = ");
    mp_obj_print_helper(print, mp_obj_new_int(breakout->get_miso()), PRINT_REPR);

    mp_print_str(print, ", int = ");
    mp_obj_print_helper(print, mp_obj_new_int(breakout->get_int()), PRINT_REPR);

    mp_print_str(print, ")");
}

/***** Constructor *****/
mp_obj_t BreakoutPAA5100_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    breakout_paa5100_BreakoutPAA5100_obj_t *self = nullptr;

    if(n_args + n_kw == 1) {
        enum { ARG_slot };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_slot, MP_ARG_REQUIRED | MP_ARG_INT },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        int slot = args[ARG_slot].u_int;
        if(slot == BG_SPI_FRONT || slot == BG_SPI_BACK) {
            self = m_new_obj_with_finaliser(breakout_paa5100_BreakoutPAA5100_obj_t);
            self->base.type = &breakout_paa5100_BreakoutPAA5100_type;

            self->breakout = new BreakoutPAA5100((BG_SPI_SLOT)slot);
        }
        else {
            mp_raise_ValueError("slot not a valid value. Expected 0 to 1");
        }
    }
    else {
        enum { ARG_spi, ARG_cs, ARG_sck, ARG_mosi, ARG_miso, ARG_interrupt };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_spi, MP_ARG_INT, {.u_int = -1} },
            { MP_QSTR_cs, MP_ARG_INT, {.u_int = pimoroni::SPI_BG_FRONT_CS} },
            { MP_QSTR_sck, MP_ARG_INT, {.u_int = pimoroni::SPI_DEFAULT_SCK} },
            { MP_QSTR_mosi, MP_ARG_INT, {.u_int = pimoroni::SPI_DEFAULT_MOSI} },
            { MP_QSTR_miso, MP_ARG_INT, {.u_int = pimoroni::SPI_DEFAULT_MISO} },
            { MP_QSTR_interrupt, MP_ARG_INT, {.u_int = pimoroni::SPI_BG_FRONT_PWM} },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        // Get SPI bus.
        int spi_id = args[ARG_spi].u_int;
        int sck = args[ARG_sck].u_int;
        int mosi = args[ARG_mosi].u_int;
        int miso = args[ARG_miso].u_int;

        if(spi_id == -1) {
            spi_id = (sck >> 3) & 0b1;  // If no spi specified, choose the one for the given SCK pin
        }
        if(spi_id < 0 || spi_id > 1) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("SPI(%d) doesn't exist"), spi_id);
        }

        if(!IS_VALID_SCK(spi_id, sck)) {
            mp_raise_ValueError(MP_ERROR_TEXT("bad SCK pin"));
        }

        if(!IS_VALID_MOSI(spi_id, mosi)) {
            mp_raise_ValueError(MP_ERROR_TEXT("bad MOSI pin"));
        }

        if(!IS_VALID_MISO(spi_id, miso)) {
            mp_raise_ValueError(MP_ERROR_TEXT("bad MISO pin"));
        }

        self = m_new_obj_with_finaliser(breakout_paa5100_BreakoutPAA5100_obj_t);
        self->base.type = &breakout_paa5100_BreakoutPAA5100_type;

        spi_inst_t *spi = (spi_id == 0) ? spi0 : spi1;
        self->breakout = new BreakoutPAA5100(spi, args[ARG_cs].u_int, sck, mosi, miso, args[ARG_interrupt].u_int);
    }

    self->breakout->init();

    return MP_OBJ_FROM_PTR(self);
}

/***** Destructor ******/
mp_obj_t BreakoutPAA5100___del__(mp_obj_t self_in) {
    breakout_paa5100_BreakoutPAA5100_obj_t *self = MP_OBJ_TO_PTR2(self_in, breakout_paa5100_BreakoutPAA5100_obj_t);
    delete self->breakout;
    return mp_const_none;
}

/***** Methods *****/
mp_obj_t BreakoutPAA5100_get_id(mp_obj_t self_in) {
    breakout_paa5100_BreakoutPAA5100_obj_t *self = MP_OBJ_TO_PTR2(self_in, breakout_paa5100_BreakoutPAA5100_obj_t);
    return mp_obj_new_int(self->breakout->get_id());
}

mp_obj_t BreakoutPAA5100_get_revision(mp_obj_t self_in) {
    breakout_paa5100_BreakoutPAA5100_obj_t *self = MP_OBJ_TO_PTR2(self_in, breakout_paa5100_BreakoutPAA5100_obj_t);
    return mp_obj_new_int(self->breakout->get_revision());
}

mp_obj_t BreakoutPAA5100_set_rotation(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_degrees };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_degrees, MP_ARG_INT, {.u_int = (uint8_t)BreakoutPAA5100::DEGREES_0} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    breakout_paa5100_BreakoutPAA5100_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, breakout_paa5100_BreakoutPAA5100_obj_t);

    int degrees = args[ARG_degrees].u_int;
    if(degrees < 0 || degrees > 3)
        mp_raise_ValueError("degrees out of range. Expected 0 (0), 1 (90), 2 (180) or 3 (270)");
    else
        self->breakout->set_rotation((BreakoutPAA5100::Degrees)degrees);

    return mp_const_none;
}

mp_obj_t BreakoutPAA5100_set_orientation(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_invert_x, ARG_invert_y, ARG_swap_xy };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_invert_x, MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_invert_y, MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_swap_xy, MP_ARG_BOOL, {.u_bool = true} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    breakout_paa5100_BreakoutPAA5100_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, breakout_paa5100_BreakoutPAA5100_obj_t);

    bool invert_x = args[ARG_invert_x].u_int;
    bool invert_y = args[ARG_invert_y].u_int;
    bool swap_xy = args[ARG_swap_xy].u_int;
    self->breakout->set_orientation(invert_x, invert_y, swap_xy);

    return mp_const_none;
}

mp_obj_t BreakoutPAA5100_get_motion(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_timeout, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    breakout_paa5100_BreakoutPAA5100_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, breakout_paa5100_BreakoutPAA5100_obj_t);

    float timeout = (float)BreakoutPAA5100::DEFAULT_MOTION_TIMEOUT_MS / 1000.0f;
    uint16_t timeout_ms = BreakoutPAA5100::DEFAULT_MOTION_TIMEOUT_MS;

    if (args[ARG_timeout].u_obj != mp_const_none) {
        timeout = mp_obj_get_float(args[ARG_timeout].u_obj);
        timeout_ms = (uint16_t)(timeout * 1000.0f);
    }
    int16_t x = 0;
    int16_t y = 0;
    if(self->breakout->get_motion(x, y, timeout_ms)) {
        mp_obj_t tuple[2];
        tuple[0] = mp_obj_new_int(x);
        tuple[1] = mp_obj_new_int(y);
        return mp_obj_new_tuple(2, tuple);
    }
    return mp_const_none;
}

mp_obj_t BreakoutPAA5100_get_motion_slow(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_timeout, MP_ARG_OBJ, {.u_obj = mp_obj_new_float((float)BreakoutPAA5100::DEFAULT_MOTION_TIMEOUT_MS / 1000)} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    breakout_paa5100_BreakoutPAA5100_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, breakout_paa5100_BreakoutPAA5100_obj_t);

    float timeout = mp_obj_get_float(args[ARG_timeout].u_obj);
    uint16_t timeout_ms = (uint16_t)(timeout * 1000.0f);
    int16_t x = 0;
    int16_t y = 0;
    if(self->breakout->get_motion_slow(x, y, timeout_ms)) {
        mp_obj_t tuple[2];
        tuple[0] = mp_obj_new_int(x);
        tuple[1] = mp_obj_new_int(y);
        return mp_obj_new_tuple(2, tuple);
    }
    return mp_const_none;
}

mp_obj_t BreakoutPAA5100_frame_capture(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_buffer, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_buffer, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_timeout, MP_ARG_OBJ, {.u_obj = mp_obj_new_float((float)BreakoutPAA5100::DEFAULT_MOTION_TIMEOUT_MS / 1000)} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    breakout_paa5100_BreakoutPAA5100_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, breakout_paa5100_BreakoutPAA5100_obj_t);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buffer].u_obj, &bufinfo, MP_BUFFER_RW);
    uint8_t *buffer = (uint8_t *)bufinfo.buf;
    if(bufinfo.len != (size_t)(BreakoutPAA5100::FRAME_BYTES)) {
        mp_raise_ValueError("Supplied buffer is the wrong size for frame capture. Needs to be 1225.");
    }

    float timeout = mp_obj_get_float(args[ARG_timeout].u_obj);
    uint16_t timeout_ms = (uint16_t)(timeout * 1000.0f);

    uint16_t data_size = 0;
    uint8_t data[BreakoutPAA5100::FRAME_BYTES];
    if(self->breakout->frame_capture(data, data_size, timeout_ms)) {
        memcpy(buffer, data, BreakoutPAA5100::FRAME_BYTES);
    }
    return mp_obj_new_int(data_size);
}
}
