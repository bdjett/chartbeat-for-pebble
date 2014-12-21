#include <pebble.h>
#include "concurrent_visits.h"
#include "graphics.h"

// APPMESSAGE KEYS
  
enum {
  PEOPLE = 0x0,
  NEW = 0x1,
  MOBILE = 0x2,
  DESKTOP = 0x3,
  GET_DATA = 0x4,
  MAX_PEOPLE = 0x5,
  PEOPLE_STRING = 0x6
};