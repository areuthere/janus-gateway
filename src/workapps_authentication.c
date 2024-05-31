#include <stdio.h>
#include <string.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <glib-2.0/glib.h>
#include <glib.h>
#include <jansson.h>
#include "config.h"
#include "workapps_authentcation.h"
#include "debug.h"
#include "apierror.h"

typedef uint32_t token_time_t;
static gboolean workapps_auth_enabled = FALSE;
static char *workapps_auth_secret = NULL;

#define time_left(t1,t2) ((((int32_t)(t1))-((int32_t)(t2))) < 0)
static gboolean authenticate_workapps_token(const char* token, const char* signature);


/* Setup */

void janus_workapps_auth_init(gboolean enabled, const char *secret) {

	if(enabled) {
		if(secret != NULL) {
			JANUS_LOG(LOG_INFO, "Workapps token based authentication enabled\n");
			workapps_auth_enabled = TRUE;
			workapps_auth_secret = g_strdup(secret);
		}
		else{
			JANUS_LOG(LOG_INFO, "Workapps Token based authentication disable as workapps secret key not configured\n");
		}
	}
 	else {
		JANUS_LOG(LOG_WARN, "Workapps token based authentication disabled\n");
	}
}

void janus_workapps_auth_deinit(void) {
	g_free(workapps_auth_secret);
	workapps_auth_secret = NULL;
}

gboolean janus_workapps_auth_is_enabled(void) {
	return workapps_auth_enabled;
}

//using same timestamp extraction method as used in coturn
static token_time_t get_workapps_api_timestamp(const char *orig_token)
{
	char* token = strdup(orig_token);
	char *token_ptr = token;
  	token_time_t ts = 0;
	int ts_set = 0;

	char *col = strchr(token_ptr,':');

	if(col) {
		// Nothing before ':', so consider the whole string after ':' as timestamp
		if(col == token_ptr) {
			token_ptr +=1;
		} else {
			char *ptr = token_ptr;
			int found_non_figure = 0;
			while(ptr < col) {
				if(!(ptr[0]>='0' && ptr[0]<='9')) {
					found_non_figure=1;
					break;
				}
				++ptr;
			}
			if(found_non_figure) {
				ts = (token_time_t)atol(col+1);
				ts_set = 1;
			} else {
				*col=0;
				ts = (token_time_t)atol(token);
				ts_set = 1;
				*col=':';
			}
		}
	}

	// no random string after ':',so consider the whole token as timestamp
	if(!ts_set) {
		ts = (token_time_t)atol(token_ptr);
	}
	free(token);
	return ts;
}

static int authenticate_workapps_token(const char* token, const char* signature)
{
  unsigned char result[EVP_MAX_MD_SIZE];
  unsigned int len;
  if(janus_workapps_auth_is_enabled()== FALSE)
  {
	  //authentication is not enabled
	  return 0;
  }

  HMAC(EVP_sha1(), (const unsigned char* )workapps_auth_secret, strlen(workapps_auth_secret), (const unsigned char* )token, strlen(token), result, &len);
  gchar *base64 = g_base64_encode(result, len);
  if(strcmp(signature, base64) == 0)
  {
        g_free(base64);
        return 0;
  }
  g_free(base64);
  return JANUS_ERROR_UNAUTHORIZED;
}

int janus_workapps_token_authenticate(const char* token, const char* signature)
{
  token_time_t time_token = get_workapps_api_timestamp(token);
  if(time_token == 0)
  {
    JANUS_LOG(LOG_WARN,"Invalid timestamp in %s token\n",token);
    return JANUS_ERROR_UNAUTHORIZED;
  }
  token_time_t current_time = (token_time_t) time(NULL);
  if(time_left(time_token,current_time))
  {
    JANUS_LOG(LOG_WARN,"Token has been expired\n");
    return JANUS_ERROR_WORKAPPS_EXPIRED_TOKEN;
  }
  return authenticate_workapps_token(token,signature);
}