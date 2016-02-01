#include "ecore_drm2_private.h"

Eina_Bool
_ecore_drm2_dbus_open(Eldbus_Connection **conn)
{
   if (!eldbus_init())
     {
        ERR("Could not initialize Eldbus library");
        return EINA_FALSE;
     }

   *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   if (!*conn)
     {
        ERR("Could not get dbus connection");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

void
_ecore_drm2_dbus_close(Eldbus_Connection *conn)
{
   if (conn) eldbus_connection_unref(conn);
   eldbus_shutdown();
}
