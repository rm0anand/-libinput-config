#include <errno.h>
#include <locale.h>
#include <string.h>

#include "config.h"

struct keyfile_pair {
    char *key, *value;
};

static bool parse_number(const char *string, double *number) {
    char *dummy = NULL;

    *number = strtod(string, &dummy);

    if (dummy[0] != '\0' || errno == EINVAL || errno == ERANGE) {
        errno = 0;

        return false;
    }

    return true;
}

static struct keyfile_pair keyfile_get_pair(FILE *file) {
    struct keyfile_pair pair = {
        .key = NULL,
        .value = NULL
    };

    char *line = NULL;
    size_t size = 0;

    char *delim = NULL;

    do {
        free(line);
        line = NULL;

        if (getline(&line, &size, file) < 0) {
            return pair;
        }

        delim = strchr(line, '=');
    } while (delim == NULL);

    char *end = strchr(line, '\n');

    if (end == NULL) {
        end = line + strlen(line);
    }

    *delim = '\0';
    *end = '\0';

    pair.key = line;
    pair.value = delim + 1;

    return pair;
}

#define cmp(a, b) strcmp(a, b) == 0

#define keys_start if (false) {}
#define keys_end else {invalid_value();}

#define values_start keys_start
#define values_end(config) else {\
    invalid_value();\
    \
    libinput_config.config = false;\
}

#define fancy_values_start(config)\
    libinput_config.config##_configured = true;\
    values_start
#define fancy_values_end(config) values_end(config##_configured)

#define key(a) else if (cmp(pair.key, a))
#define value(a) else if (cmp(pair.value, a))

#define invalid_key() print("warning: invalid setting key")
#define invalid_value() print("warning: invalid setting value")

#define keyword_value(name, config, prop)\
    value(name) {\
        libinput_config.config = prop;\
    }

#define binary_value(config)\
    keyword_value("disabled", config, false)\
    keyword_value("enabled", config, true)
#define fancy_binary_value(config, CONFIG)\
    keyword_value("disabled", config, LIBINPUT_CONFIG_##CONFIG##_DISABLED)\
    keyword_value("enabled", config, LIBINPUT_CONFIG_##CONFIG##_ENABLED)

#define binary_preset(name, config)\
    key(name) {\
        fancy_values_start(config)\
        \
        binary_value(config)\
        \
        fancy_values_end(config)\
    }
#define fancy_binary_preset(name, config, CONFIG)\
    key(name) {\
        fancy_values_start(config)\
        \
        fancy_binary_value(config, CONFIG)\
        \
        fancy_values_end(config)\
    }

#define enum_preset_2(name, config, o1, c1, o2, c2)\
    key(name) {\
        fancy_values_start(config)\
        \
        keyword_value(o1, config, c1)\
        keyword_value(o2, config, c2)\
        \
        fancy_values_end(config)\
    }
#define enum_preset_3(name, config, o1, c1, o2, c2, o3, c3)\
    key(name) {\
        fancy_values_start(config)\
        \
        keyword_value(o1, config, c1)\
        keyword_value(o2, config, c2)\
        keyword_value(o3, config, c3)\
        \
        fancy_values_end(config)\
    }
#define enum_preset_4(name, config, o1, c1, o2, c2, o3, c3, o4, c4)\
    key(name) {\
        fancy_values_start(config)\
        \
        keyword_value(o1, config, c1)\
        keyword_value(o2, config, c2)\
        keyword_value(o3, config, c3)\
        keyword_value(o4, config, c4)\
        \
        fancy_values_end(config)\
    }

#define fancy_parse_number(config)\
    bool success = parse_number(\
        pair.value,\
        &libinput_config.config\
    )

#define check_parse_failure(config, default_value)\
    if (!success) {\
        invalid_value();\
        \
        libinput_config.config = default_value;\
    }

#define simple_parse_preset(name, config)\
    key(name) {\
        fancy_values_start(config)\
        \
        fancy_parse_number(config);\
        \
        check_parse_failure(config##_configured, false)\
    }

#define hacky_parse_preset(name, config)\
    key(name) {\
        fancy_parse_number(config);\
        \
        check_parse_failure(config, 1)\
    }
#define double_hacky_parse_preset(name, config)\
    key(name) {\
        fancy_parse_number(config##_x);\
        \
        if (!success) {\
            invalid_value();\
            \
            libinput_config.config##_x = 1;\
            libinput_config.config##_y = 1;\
        } else {\
            libinput_config.config##_y =\
                libinput_config.config##_x;\
        }\
    }

#define xy_parse_preset(name, config)\
    double_hacky_parse_preset(name, config)\
    hacky_parse_preset(name "-x", config##_x)\
    hacky_parse_preset(name "-y", config##_y)

void libinput_config_init(void) {
    print("initializing");

    if (libinput_config.configured) {
        print("already initialized");

        return;
    }
    libinput_config.configured = true;

    print("replacing the device config functions");
    libinput_real_init();

    print("initializing the key mapper");
    libinput_keymap_init();

    FILE *file = fopen("/etc/libinput.conf", "r");

    if (file == NULL) {
        print("couldn't read the config file");

        return;
    }

    // We need to switch locales to correctly parse numbers

    locale_t
        initial_locale = newlocale(LC_NUMERIC_MASK, "", NULL),
        c_locale = newlocale(LC_NUMERIC_MASK, "C", NULL);

    uselocale(c_locale);

    while (true) {
        struct keyfile_pair pair = keyfile_get_pair(file);

        if (pair.key == NULL || pair.value == NULL) {
            break;
        }

        print("option '%s' is '%s'", pair.key, pair.value);

        keys_start

        key("override-compositor") {
            values_start

            binary_value(override_compositor)

            values_end(override_compositor)
        }

        binary_preset("natural-scroll", natural_scroll)
        binary_preset("left-handed", left_handed)

        fancy_binary_preset("tap", tap, TAP)
        fancy_binary_preset("drag", drag, DRAG)
        fancy_binary_preset("drag-lock", drag_lock, DRAG_LOCK)
        fancy_binary_preset("scroll-button-lock", scroll_button_lock, SCROLL_BUTTON_LOCK)
        fancy_binary_preset("middle-emulation", middle_emulation, MIDDLE_EMULATION)
        fancy_binary_preset("dwt", dwt, DWT)
#ifdef LIBINPUT_HAS_DWTP
        fancy_binary_preset("dwtp", dwtp, DWTP)
#endif

        enum_preset_2("tap-button-map", tap_button_map,
            "lrm", LIBINPUT_CONFIG_TAP_MAP_LRM,
            "lmr", LIBINPUT_CONFIG_TAP_MAP_LMR
        )

        enum_preset_3("accel-profile", accel_profile,
            "none", LIBINPUT_CONFIG_ACCEL_PROFILE_NONE,
            "flat", LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT,
            "adaptive", LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE
        )
        enum_preset_3("click-method", click_method,
            "none", LIBINPUT_CONFIG_CLICK_METHOD_NONE,
            "button-areas", LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS,
            "clickfinger", LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER
        )

        enum_preset_4("scroll-method", scroll_method,
            "none", LIBINPUT_CONFIG_SCROLL_NO_SCROLL,
            "two-fingers", LIBINPUT_CONFIG_SCROLL_2FG,
            "edge", LIBINPUT_CONFIG_SCROLL_EDGE,
            "on-button-down", LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN
        )

        simple_parse_preset("accel-speed", accel_speed)

        xy_parse_preset("scroll-factor", scroll_factor)
        xy_parse_preset("discrete-scroll-factor", discrete_scroll_factor)
        xy_parse_preset("speed", speed)
        xy_parse_preset("gesture-speed", gesture_speed)

        key("scroll-button") {
            fancy_values_start(scroll_button)

            double val = 0;

            bool success =
                parse_number(pair.value, &val) &&

                val >= 1 &&
                val <= UINT32_MAX;

            if (!success) {
                invalid_value();

                libinput_config.scroll_button_configured = false;
            } else {
                libinput_config.scroll_button = (uint32_t) val;
            }
        }

        key("remap-key") {
            char *delim = strchr(pair.value, ':');

            if (delim == NULL) {
                invalid_value();
            } else {
                *delim = '\0';

                char
                    *source_str = pair.value,
                    *destination_str = delim + 1;

                double
                    source = 0,
                    destination = 0;

                bool success =
                    parse_number(source_str, &source) &&
                    parse_number(destination_str, &destination) &&

                    source >= 1 &&
                    source <= UINT32_MAX &&

                    destination >= 1 &&
                    destination <= UINT32_MAX &&

                    libinput_keymap_set((uint32_t) source, (uint32_t) destination);

                if (!success) {
                    invalid_value();
                }
            }
        }

        keys_end

        free(pair.key);
    }

    fclose(file);

    uselocale(initial_locale);

    print("initialized");
}
