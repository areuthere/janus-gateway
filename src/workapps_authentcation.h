#include <glib-2.0/glib.h>
#include <glib.h>

#ifndef JANUS_WORKAPPS_AUTHENTICATION_H
#define JANUS_WORKAPPS_AUTHENTICATION_H
void janus_workapps_auth_init(gboolean enabled, const char *secret);
void janus_workapps_auth_deinit(void);
gboolean janus_workapps_auth_is_enabled(void);
int janus_workapps_token_authenticate(const char* token, const char* signature);
#endif