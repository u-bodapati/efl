#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <assert.h>

typedef struct _Elm_Atspi_Cache_Data {
     Eina_Hash *cache;
} Elm_Atspi_Cache_Data;

#include "atspi/atspi-constants.h"
#include "elm_priv.h"
#include "elm_atspi_cache.eo.h"

static Elm_Interface_Atspi_Accessible*
_id_to_obj(uintptr_t ptr)
{
   Eo *obj = (Eo *) ptr;
   return efl_isa(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN) ? obj : NULL;
}

static long
_obj_to_id(Elm_Interface_Atspi_Accessible *object)
{
   uintptr_t ptr = (uintptr_t)object;
   return (long)ptr;
}

EOLIAN static Elm_Atspi_Cache*
_elm_atspi_cache_efl_object_constructor(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd)
{
   efl_constructor(efl_super(cache, ELM_ATSPI_CACHE_CLASS));

   pd->cache = eina_hash_pointer_new(NULL);
   assert (pd->cache != NULL);

   return cache;
}

EOLIAN static void
_elm_atspi_cache_efl_object_destructor(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd)
{
   eina_hash_free(pd->cache);
   efl_destructor(efl_super(cache, ELM_ATSPI_CACHE_CLASS));
}

EOLIAN static Elm_Interface_Atspi_Accessible*
_elm_atspi_cache_object_from_id(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd, long id)
{
   Elm_Interface_Atspi_Accessible *obj = _id_to_obj((uintptr_t)id);
   return obj ? eina_hash_find(pd->cache, &obj) : NULL;
}

EOLIAN static Eina_Bool
_elm_atspi_cache_contains_id(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd, long id)
{
   Elm_Interface_Atspi_Accessible *obj = _id_to_obj((uintptr_t)id);
   return obj ? eina_hash_find(pd->cache, &obj) : NULL;
}

EOLIAN static Eina_Bool
_elm_atspi_cache_contains_object(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd, Elm_Interface_Atspi_Accessible *object)
{
   return eina_hash_find(pd->cache, &object) ? EINA_TRUE : EINA_FALSE;
}

EOLIAN static long
_elm_atspi_cache_id_from_object(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd, Elm_Interface_Atspi_Accessible *object)
{
}

EOLIAN static void
_elm_atspi_cache_add(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd, Elm_Interface_Atspi_Accessible *object)
{
   eina_hash_add(pd->cache, &object, object);
}

EOLIAN static void
_elm_atspi_cache_remove(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd, Elm_Interface_Atspi_Accessible *object)
{
   eina_hash_del(pd->cache, &object, object);
}

EOLIAN static Eina_Iterator*
_elm_atspi_cache_iterator_get(Elm_Atspi_Cache *cache EINA_UNUSED, Elm_Atspi_Cache_Data *pd)
{
   return eina_hash_iterator_data_new(pd->cache);
}

#include "elm_atspi_cache.eo.c"
