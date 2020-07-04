/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include <swcdb/thrift/client/client.h>

#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <security/pam_appl.h>
#include <security/pam_modules.h>


static const char* SWCDB_PAM_NAME = "SWC-DB PAM: ";

struct _swc_pam_cfg {
  gchar *  host;
  gint     port;
  gint     timeout;
  gchar *  column;
  gchar *  key;
  gint     maxtries;
  //gboolean ssl        = FALSE;
};
typedef struct _swc_pam_cfg swc_pam_cfg;


bool swc_pam_read_config(int argc, const char **argv, swc_pam_cfg* cfg) {
  cfg->host     = NULL;
  cfg->port     = 18000;
  cfg->timeout  = 30000;
  cfg->column   = NULL;
  cfg->key      = NULL;
  cfg->maxtries = 10;

  GOptionEntry swc_pam_options[] = {
    { "host",        'h', 0, G_OPTION_ARG_STRING,  &cfg->host,
      "SWC-DB Thrift Broker host(=localhost)", NULL },
    { "port",       'p', 0, G_OPTION_ARG_INT,      &cfg->port,
      "SWC-DB Thrift Broker port(=9090)", NULL },
    { "timeout",    'o', 0, G_OPTION_ARG_INT,      &cfg->timeout,
      "Timeout in ms(=30000)", NULL },
    { "column",     'c', 0, G_OPTION_ARG_STRING,   &cfg->column,
      "SWC-DB Column Name to use", NULL },
    { "key",        'k', 0, G_OPTION_ARG_STRING,   &cfg->key,
      "SWC-DB key with %s for ip", NULL },
    { "maxtries",   'm', 0, G_OPTION_ARG_INT,      &cfg->maxtries,
      "Max Tries (=10)", NULL },
    { NULL }
  };

  GError *error = NULL;
  GOptionContext* opts_ctx = g_option_context_new (NULL);
  g_option_context_add_main_entries(opts_ctx, swc_pam_options, NULL);

  gchar** gargv = (char**)argv;
  if(!g_option_context_parse(opts_ctx, &argc, &gargv, &error)) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s cfg-parse error %s=", 
            SWCDB_PAM_NAME, error->message);
    return false;
  }
  g_option_context_free(opts_ctx);
  
  if(cfg->column == NULL) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s cfg-missing %s", 
            SWCDB_PAM_NAME,  "COLUMN");
    return false;
  }
  if(cfg->host == NULL)
    cfg->host = g_strdup ("localhost");

  if(cfg->key == NULL)
    cfg->key = g_strdup ("[%s]");

  return true;
}

bool swcdb_pam_connect(swcdb_thrift_client* client, swc_pam_cfg* cfg) {
  GError*  err = NULL;
  if(!swcdb_thrift_client_connect(client, cfg->host, cfg->port, &err)) {
    if(err != NULL) {
      syslog(LOG_NOTICE|LOG_AUTH, "%s connect-failed %s", 
              SWCDB_PAM_NAME, err->message);
      g_error_free(err);
      err = NULL;
    } else {
      syslog(LOG_NOTICE|LOG_AUTH, "%s connect-failed error-uknown", 
              SWCDB_PAM_NAME);
    }
    return false;
  }
  return true;
}

bool swcdb_pam_disconnect(swcdb_thrift_client* client, swc_pam_cfg* cfg) {
  GError*  err = NULL;
  if(!swcdb_thrift_client_disconnect(client, &err)) {
    if(err != NULL) {
      syslog(LOG_NOTICE|LOG_AUTH, "%s disconnect-failed %s", 
              SWCDB_PAM_NAME, err->message);
      g_error_free(err);
      err = NULL;
    } else {
      syslog(LOG_NOTICE|LOG_AUTH, "%s disconnect-failed error-uknown", 
              SWCDB_PAM_NAME);
    }
    return false;
  }
  return true;
}

bool swcdb_pam_confirm_state(swc_pam_cfg* cfg, const char* pam_rhost) {
  bool     allowed = true;
  gint64   tries   = 0;

  swcdb_thrift_client client;
  if(!swcdb_pam_connect(&client, cfg))
    return allowed;
  
  gchar* key = NULL;
  gint len = g_snprintf(key, 256, cfg->key, pam_rhost);
  if(!len) {
    swcdb_pam_disconnect(&client, cfg);
    return !allowed;
  }
  

  gchar* sql = NULL;
  len = g_snprintf(sql, 1024,
    "select where col('%s')=(cells=(key=%s limit=1))", 
    cfg->column, key
  );
  if(!len || !sql) {
    if(sql)
      g_free(sql);
    g_free(key);
    swcdb_pam_disconnect(&client, cfg);
    return allowed;
  }

  swcdb_thriftCells* cells = g_object_new (
    SWCDB_THRIFT_TYPE_SERVICE_SQL_SELECT_RESULT, NULL);
  swcdb_thriftException*    exception = NULL;
  GError*                   err = NULL;

  if(swcdb_thrift_service_client_sql_select(
     client.service, &cells, sql, &exception, &err)) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s query %s", SWCDB_PAM_NAME, sql);
    
    if(cells->len > 0) {
      swcdb_thriftCell* cell = g_ptr_array_index(cells, 0);
      char *last;
      tries = strtoll((const char*)cell->v->data, &last, 0);
      g_clear_object(&cell);
    }

  } else if(err != NULL) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s select-failed %s", 
            SWCDB_PAM_NAME, err->message);
    g_error_free(err);
    err = NULL;

  } else if(exception != NULL) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s select-exception %d(%s)", 
            SWCDB_PAM_NAME, exception->code, exception->message);
    g_clear_object(&exception);
    
  } else {
    syslog(LOG_NOTICE|LOG_AUTH, "%s select-failed error-uknown", 
            SWCDB_PAM_NAME);
  }
  g_free(sql);
  g_clear_object(&cells);
  

  bool adj = tries < 0;
  allowed = tries == 0 || !adj || tries <= cfg->maxtries;

  gchar* update;
  len = g_snprintf(update, 1024,
    "update cell(INSERT,'%s', %s, '', '%s%d')", 
    cfg->column, 
    key, 
    adj ? "=" : "+", 
    adj ? cfg->maxtries : 1
  );
  g_free(key);
  if(!len) {
    swcdb_pam_disconnect(&client, cfg);
    return allowed;
  }

  if(swcdb_thrift_service_client_sql_update(
     client.service, update, 0, &exception, &err)) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s update %s", SWCDB_PAM_NAME, update);

  } else if(err != NULL) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s update-failed %s", 
            SWCDB_PAM_NAME, err->message);
    g_error_free(err);
    err = NULL;

  } else if(exception != NULL) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s update-exception %d(%s)", 
            SWCDB_PAM_NAME, exception->code, exception->message);
    g_clear_object(&exception);
    
  } else {
    syslog(LOG_NOTICE|LOG_AUTH, "%s update-failed error-uknown", 
            SWCDB_PAM_NAME);
  }
  g_free(update);

  swcdb_pam_disconnect(&client, cfg);
  return allowed;
}

void swcdb_pam_reduce_attempt(swc_pam_cfg* cfg, const char* pam_rhost) {
  swcdb_thrift_client client;
  if(!swcdb_pam_connect(&client, cfg))
    return;

  gchar* key = NULL;
  gint len = g_snprintf(key, 256, cfg->key, pam_rhost);
  if(!len || !key) {
    if(key)
      g_free(key);
    swcdb_pam_disconnect(&client, cfg);
    return;
  }

  gchar* update;
  len = g_snprintf(update, 1024,
    "update cell(INSERT,'%s', %s, '', '=0'", cfg->column, key);
  if(!len) {
    g_free(key);
    swcdb_pam_disconnect(&client, cfg);
    return;
  }

  swcdb_thriftException*    exception = NULL;
  GError*                   err = NULL;

  if(swcdb_thrift_service_client_sql_update(
     client.service, update, 0, &exception, &err)) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s update %s", SWCDB_PAM_NAME, update);
  
  } else if(err != NULL) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s update-failed %s", 
            SWCDB_PAM_NAME, err->message);
    g_error_free(err);
    err = NULL;

  } else if(exception != NULL) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s update-exception %d(%s)", 
            SWCDB_PAM_NAME, exception->code, exception->message);
    g_clear_object(&exception);
    
  } else {
    syslog(LOG_NOTICE|LOG_AUTH, "%s update-failed error-uknown", 
            SWCDB_PAM_NAME);
  }
  g_free(update);
  
  swcdb_pam_disconnect(&client, cfg);
}



/* PAM HANDLERS */


/* PAM AUTH */
PAM_EXTERN int 
pam_sm_authenticate(pam_handle_t * pamh, int flags, int argc, const char **argv) {

  const char *pam_rhost = NULL;
  if(pam_get_item(pamh, PAM_RHOST, (const void **) &pam_rhost) 
     != PAM_SUCCESS) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s Failed to get pam_rhost", SWCDB_PAM_NAME);
    return PAM_PERM_DENIED;
  }
  if(pam_rhost == NULL) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s Failed pam_rhost is NULL", SWCDB_PAM_NAME);
    return PAM_PERM_DENIED;
  }

  swc_pam_cfg cfg; 
  if(!swc_pam_read_config(argc, argv, &cfg) ||
     swcdb_pam_confirm_state(&cfg, pam_rhost))
    return PAM_SUCCESS;

  return PAM_MAXTRIES;  // PAM_PERM_DENIED;
}

/* expected hook */
PAM_EXTERN int 
pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv ) {
  syslog(LOG_NOTICE|LOG_AUTH, "%s pam_sm_setcred", SWCDB_PAM_NAME);
  return PAM_SUCCESS ;
}


/* PAM SESSION */
PAM_EXTERN int 
pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {

  const char *pam_rhost = NULL;
  if(pam_get_item(pamh, PAM_RHOST, (const void **) &pam_rhost) 
     != PAM_SUCCESS) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s Failed to get pam_rhost", SWCDB_PAM_NAME);
    return PAM_PERM_DENIED;
  }
  if(pam_rhost == NULL) {
    syslog(LOG_NOTICE|LOG_AUTH, "%s Failed pam_rhost is NULL", SWCDB_PAM_NAME);
    return PAM_PERM_DENIED;
  }

  swc_pam_cfg cfg; 
  if(swc_pam_read_config(argc, argv, &cfg))
    swcdb_pam_reduce_attempt(&cfg, pam_rhost);

  return PAM_SUCCESS;
}

/* expected hook */
PAM_EXTERN int 
pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  return PAM_SUCCESS;
}

/* 
PAM_EXTERN int 
pam_sm_acct_mgmt( pam_handle_t *pamh, int flags, int argc, const char **argv ) {
  syslog(LOG_NOTICE|LOG_AUTH, "%s pam_sm_acct_mgmt", SWCDB_PAM_NAME);
  return pam_sm_authenticate(pamh, flags, argc, argv);
}
*/

