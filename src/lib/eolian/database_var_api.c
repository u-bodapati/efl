#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

EAPI const Eolian_Variable *
eolian_variable_global_get_by_name(const Eolian_Unit *unit EINA_UNUSED,
                                   const char *name)
{
   if (!_globals) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Variable *v = eina_hash_find(_globals, shr);
   eina_stringshare_del(shr);
   return v;
}

EAPI const Eolian_Variable *
eolian_variable_constant_get_by_name(const Eolian_Unit *unit EINA_UNUSED,
                                     const char *name)
{
   if (!_constants) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Variable *v = eina_hash_find(_constants, shr);
   eina_stringshare_del(shr);
   return v;
}

EAPI Eina_Iterator *
eolian_variable_globals_get_by_file(const Eolian_Unit *unit EINA_UNUSED,
                                    const char *fname)
{
   if (!_globalsf) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(fname);
   Eina_List *l = eina_hash_find(_globalsf, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eina_Iterator *
eolian_variable_constants_get_by_file(const Eolian_Unit *unit EINA_UNUSED,
                                      const char *fname)
{
   if (!_constantsf) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(fname);
   Eina_List *l = eina_hash_find(_constantsf, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eina_Iterator *
eolian_variable_all_constants_get(const Eolian_Unit *unit EINA_UNUSED)
{
   return (_constants ? eina_hash_iterator_data_new(_constants) : NULL);
}

EAPI Eina_Iterator *
eolian_variable_all_globals_get(const Eolian_Unit *unit EINA_UNUSED)
{
   return (_globals ? eina_hash_iterator_data_new(_globals) : NULL);
}

EAPI Eolian_Variable_Type
eolian_variable_type_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, EOLIAN_VAR_UNKNOWN);
   return var->type;
}

EAPI const Eolian_Documentation *
eolian_variable_documentation_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return var->doc;
}

EAPI Eina_Stringshare *
eolian_variable_file_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return var->base.file;
}

EAPI const Eolian_Type *
eolian_variable_base_type_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return var->base_type;
}

EAPI const Eolian_Expression *
eolian_variable_value_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return var->value;
}

EAPI Eina_Stringshare *
eolian_variable_name_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return var->name;
}

EAPI Eina_Stringshare *
eolian_variable_full_name_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return var->full_name;
}

EAPI Eina_Iterator *
eolian_variable_namespaces_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   if (!var->namespaces) return NULL;
   return eina_list_iterator_new(var->namespaces);
}

EAPI Eina_Bool
eolian_variable_is_extern(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, EINA_FALSE);
   return var->is_extern;
}
