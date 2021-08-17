# LABORATORIO 1: CRASH 

## Grupo N° 26: Tres acordes

## Parte 1: Desarrollo

La principal manera en la que programamos fue utilizando una extension del programa Visual Studio, llamada Live Share, permite conectar varias personas al Visual de una pc (el que comparte el workspace). Y de esta manera todas las personas conectadas pueden escribir en tiempo real en el mismo archivo.

1. **Modulo scommand**

Al principio, comenzamos con la implementacion del scommand, utilizando el TAD listas simples [GSlist](https://developer.gnome.org/glib/stable/glib-Singly-Linked-Lists.html). Pero eso nos trajo varias dificultades con respecto a los leaks. 

Por lo tanto, decidimos cambiar de TAD, eligiendo el de pila [GQueue](https://developer.gnome.org/glib/stable/glib-Double-ended-Queues.html). Nos fue bastante bien en las funciones generales, pero tuvimos dificultades en las funciones to_string, ya que no teniamos mucho conocimiento sobre el manejo de cadenas de strings.

2. **Modulo pipeline**

Este modulo fue muy similar al anterior modulo, y nos fue mas sencillo implementarlo luego de haber comprendido varios temas del scommand.
Al igual que en el scommand nos trabamos un poco en la funcion to_string, pero terminamos resolviendo todo.

3. **Modulo execute**

Desde un principio, no sabiamos como comenzar. Estuvimos varios dias tratando de arrancarlo pero no sabiamos como. Luego de buscar exhaustivamente en internet, mirando videos de youtube, leyendo el libro de la materia, y consultando nuestras dudas a nuestro profesor asignado. Pudimos de a poco avanzar en este modulo. 

Tambien tuvimos dificultades con el manejo y entendimiento de los file descriptor, lo mismo, consultando todo el material disponible pudimos resolverlo.

## Parte 2: Resultado final 

La alegria y emocion de ver el 100% del test y 0 leaks fue muy grande.

Nos desvelamos varios dias, y pasamos dias enteros literalmente frente al execute.
Tenemos varias dudas sobre el correcto funcionamiento de nuestras implementaciones, dado que no tuvimos tiempo de crear un main para probarlas.

El desarrollo y la mejora de los resultados de los test fue paulatino pero siempre comenzando con un 90% aprox, lo que nos reducia bastante para comprobar y solucionar los errores faltantes.

Finalmente, logramos que funcionara todo adecuadamente. 

Conocimientos nuevos adquiridos con este lab:

- Manejo de cadenas de strings.
- Manejo de file descriptors.
- Uso de tuberias y redirecciones.
- Comprension de las syscalls.

