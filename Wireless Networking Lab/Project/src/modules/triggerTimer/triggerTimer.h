#ifndef TRIGGER_TIMER_H
#define TRIGGER_TIMER_H

void start_timer(int* carsCount);
void stop_timer(void);

void register_timer_callback(void (*timer_cb)(void));

#endif /* TRIGGER_TIMER_H */
