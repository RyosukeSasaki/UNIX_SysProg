#ifndef MYDHCPCLIENT_TYPES_H
#define MYDHCPCLIENT_TYPES_H

typedef enum _stat {
    init=1,
    wait_offer1,
    wait_offer2,
    wait_ack1,
    wait_ack2,
    in_use,
    wait_ext_ack,
    term,
} state_t;

typedef enum _event {
    offer_ok,
    offer_ng,
    ack_ok,
    ack_ng,
    half_ttl,
    sighup,
    unknown_msg
} event_t;

#endif