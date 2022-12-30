#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

typedef enum
{
    PROVISION_ACCESSPOINT = 0,
    PROVISION_SMARTCONFIG = 1,
} provision_type_t;

_Bool is_provisioned(void);
void app_config(void);
void ap_start(void);
#endif