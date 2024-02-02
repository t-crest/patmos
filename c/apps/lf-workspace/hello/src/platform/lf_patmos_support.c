
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <lf_patmos_support.h>
#include "../platform.h"
#include <machine/rtc.h>
#include <stdio.h>
int _lf_interruptable_sleep_until_locked(environment_t* env, instant_t wakeup) {
    instant_t now;
    _lf_async_event = false;
    lf_disable_interrupts_nested();

    // Do busy sleep
    do {
        _lf_clock_now(&now);
    } while ((now < wakeup) && !_lf_async_event);

    lf_enable_interrupts_nested();

    if (_lf_async_event) {
        _lf_async_event = false;
        return -1;
    } else {
        return 0;
    }
}


/**
 * Initialize the LF clock. Arduino auto-initializes its clock, so we don't do anything.
 */
void _lf_initialize_clock() {

}

/**
 * Write the current time in nanoseconds into the location given by the argument.
 * This returns 0 (it never fails, assuming the argument gives a valid memory location).
 * This has to be called at least once per 35 minutes to properly handle overflows of the 32-bit clock.
 * TODO: This is only addressable by setting up interrupts on a timer peripheral to occur at wrap.
 */

int _lf_clock_now(instant_t* t) {

    assert(t != NULL);

   //*t = (get_cpu_cycles() * 12); //12 sec
    *t = get_cpu_usecs() *1000;
   //*t = ((get_cpu_cycles() >> 1)  * 25) ;
   //*t = ((get_cpu_cycles() *25)  >> 1) ;
    return 0;
}

#if defined(LF_UNTHREADED)

int lf_enable_interrupts_nested() {
    // if (_lf_num_nested_critical_sections++ == 0) {
    //     // First nested entry into a critical section.
    //     // If interrupts are not initially enabled, then increment again to prevent
    //     // TODO: Do we need to check whether the interrupts were enabled to
    //     //  begin with? AFAIK there is no Arduino API for that
    //     noInterrupts();
    // }
    // return 0;
}
int lf_disable_interrupts_nested() {
    // if (_lf_num_nested_critical_sections <= 0) {
    //     return 1;
    // }
    // if (--_lf_num_nested_critical_sections == 0) {
    //     interrupts();
    // }
    // return 0;
}

/**
 * Handle notifications from the runtime of changes to the event queue.
 * If a sleep is in progress, it should be interrupted.
*/
int _lf_unthreaded_notify_of_event() {
//    _lf_async_event = true;
//    return 0;
}

#endif
// Overwrite print functions with NoOp.
int puts(const char *str) {}

// int printf(const char *format, ...) {}
// int sprintf(char *str, const char *format, ...) {}
// int snprintf(char *str, size_t size, const char *format, ...) {}
// int vprintf(const char *format, va_list ap) {}
// int vfprintf(FILE *stream, const char *format, va_list arg) {}

//void lf_print_error_and_exit(const char* format, ...) {}
// Fun