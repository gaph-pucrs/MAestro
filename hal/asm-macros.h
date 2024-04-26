#pragma once

#ifdef __ASSEMBLY__
  .set last_enum_value, 0
  .macro enum_val name
  .equiv \name, last_enum_value
  .set last_enum_value, last_enum_value + 1
  .endm
#define ENUM_BEGIN  .set last_enum_value, 0
#define ENUM_VAL(name) enum_val name
#define ENUM_VALASSIGN(name, value)            \
  .set last_enum_value, value                 ;\
  enum_val name
#define ENUM_END(enum_name)
#else
#define ENUM_BEGIN typedef enum {
#define ENUM_VAL(name) name,
#define ENUM_VALASSIGN(name, value) name = value,
#define ENUM_END(enum_name) } enum_name;
#endif
