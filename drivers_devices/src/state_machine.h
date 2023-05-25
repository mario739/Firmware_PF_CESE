/**
 * @file state_machine.h
 * @author Mario Aguilar Montoya (fernandoaguilar731010@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-02-15
 * 
 * @copyright Copyright (c) 2023
 * 
 */

typedef struct state_machine_s state_machine_state_t;


typedef void(*state_machine_state_cb)(state_machine_state_t *self);

typedef struct {
    state_machine_state_cb entry;
    state_machine_state_cb loop;
    state_machine_state_cb exit;
}state_machine_state_t;


struct state_machine_s
{
    
    state_machine_state_t state[];
};


void state_machine_init(state_machine_state_t *self )
{
    
}

void state_machine_state_init(state_machine_state_t *self,state_machine_state_cb state_entry,state_machine_state_cb state_loop,state_machine_state_cb state_exit )
{
}

void state_machine_loop(state_machine_state_t *self)
{
    state_machine_state_t *current_state =
}
