#ifndef HARDWARE_H
#define HARDWARE_H

inline void __nesc_enable_interrupt() { EXCEPTION_STATUS |= 1; }
inline void __nesc_disable_interrupt() { EXCEPTION_STATUS &= ~1; }

typedef uint8_t __nesc_atomic_t;
typedef uint8_t mcu_power_t;

inline __nesc_atomic_t __nesc_atomic_start(void) @spontaneous() {
  return 0;
}

inline void __nesc_atomic_end(__nesc_atomic_t x) @spontaneous() { }
inline void __nesc_atomic_sleep() { }

// enum so components can override power saving
enum {
  TOS_SLEEP_NONE = 0,
};

#endif