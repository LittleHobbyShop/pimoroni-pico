#include "pico/stdlib.h"
#include <stdio.h>
#include <cstring>
#include <string>
#include <algorithm>
#include "pico/time.h"
#include "pico/platform.h"

#include "common/pimoroni_common.hpp"
#include "badger2040.hpp"

#include "badger2040_image_demo_images.hpp"

using namespace pimoroni;

Badger2040 badger;

uint32_t time() {
  absolute_time_t t = get_absolute_time();
  return to_ms_since_boot(t);
}

int main() {

  stdio_init_all();

  sleep_ms(500);

  printf("\n\n=======\nbadger2040 starting up\n\n");

  badger.init();

  badger.pen(15);
  badger.clear();

  badger.pen(0);
  badger.font("sans");
  badger.text("Press A, B, or C", 15, 65, 1.0f);
  badger.update();

  while(true) {

    printf("> waiting for a button press..");
    badger.wait_for_press();
    printf("done!\n");

    if(badger.pressed(badger.A)) {
      printf("> A pressed\n");
      badger.image(shaun);
    }

    if(badger.pressed(badger.B)) {
      printf("> B pressed\n");
      badger.image(paul);
    }

    if(badger.pressed(badger.C)) {
      printf("> C pressed\n");
      badger.image(adam);
    }

    badger.update();
  }
}