#include "rc.h"
#include "fsm.h"

typedef int (* fsm_trans_fun) (void * data);

static const char* FSM_STATE_NAMES[FSM_STATE_LEN] = {"FSM_STATE_INIT", "FSM_STATE_WIFI_CONNECTION", "FSM_STATE_INIT_CONFIG", "FSM_STATE_FETCH", "FSM_STATE_DISPLAY_UPDATE", "FSM_STATE_ERROR", "FSM_STATE_LEN"};

static enum FsmState fsm_current_state;
static fsm_trans_fun fsm_trans_array[FSM_STATE_LEN];

static int prv_fsm_init_trans_fun(void * data){
    return RC_OK;
}

static int prv_fsm_wifi_connection_trans_fun(void * data){
    return RC_OK;
}

static int prv_fsm_init_config_trans_fun(void * data){
    return RC_OK;
}

static int prv_fsm_fetch_trans_fun(void * data){
    return RC_OK;
}

static int prv_fsm_display_update_trans_fun(void * data){
    return RC_OK;
}

static int prv_fsm_error_trans_fun(void * data){
    return RC_OK;
}

void fsm_init(void){
    fsm_current_state = FSM_STATE_INIT;
    
    fsm_trans_array[FSM_STATE_INIT] = prv_fsm_init_trans_fun;
    fsm_trans_array[FSM_STATE_WIFI_CONNECTION] = prv_fsm_wifi_connection_trans_fun;
    fsm_trans_array[FSM_STATE_INIT_CONFIG] = prv_fsm_init_config_trans_fun;
    fsm_trans_array[FSM_STATE_FETCH] = prv_fsm_fetch_trans_fun;
    fsm_trans_array[FSM_STATE_DISPLAY_UPDATE] = prv_fsm_display_update_trans_fun;
    fsm_trans_array[FSM_STATE_ERROR] = prv_fsm_error_trans_fun;
}

enum FsmState fsm_get_state(void){
    return fsm_current_state;
}

const char* fsm_get_state_name(void){
    return FSM_STATE_NAMES[fsm_current_state];
}

int fsm_update(void * data){
    return fsm_trans_array[fsm_current_state](data);
}

