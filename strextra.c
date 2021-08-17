#include "strextra.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <gmodule.h>

char * strmerge(char *s1, char *s2){    // Concatena el string s1 con el s2
  assert(s1 != NULL &&  s2 != NULL);
  gsize size = (strlen(s1) + strlen(s2) + 1); //Calculamos el tamaño que debe tener el string total
  s1 = realloc(s1,size);                //Y se lo reasignamos al primer argumento
  assert(s1 != NULL);
  strcat(s1,s2);                        // Concatenamos s2 a s1 (quien ya tiene el tamaño necesario)
  assert((s1 != NULL) && strlen(s1) == size-1); 
  return s1;                            // Retorna el string concatenado
}
