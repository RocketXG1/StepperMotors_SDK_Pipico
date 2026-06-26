MANUAL DETALLADO: README_BUILD.txt
==================================

Finalidad: Este archivo funciona como archivo de documentación o contexto del proyecto.

Ubicación y relación dentro del proyecto:
- Ruta original: README_BUILD.txt
- Manual generado: Manuals/README_BUILD.txt_manual.txt
- No declara includes C/C++ directos, o es un archivo de texto/configuración.
- Relación general: los encabezados en lib exponen tipos; los .cpp en src implementan comportamiento; prgProject integra la aplicación; FunctionBlocks encapsula lógica reusable; los documentos .txt explican contexto de uso.

Forma de uso o instanciación esperada:
- Si es un .h, se incluye con #include desde archivos .cpp y sus clases/structs se instancian según sus constructores o fábricas estáticas.
- Si es un .cpp, no se instancia directamente: se compila y enlaza para aportar las implementaciones declaradas en su encabezado correspondiente.
- Si es .pio, se carga en el bloque PIO y se inicializa mediante la función C SDK generada/embebida.
- Si es CMake o .txt, se usa como soporte de compilación o documentación; no produce objetos C++ instanciables.

Descripción línea a línea:
Línea 1: `Proyecto: template_motion_project`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 2: ``
  Explicación: Línea en blanco usada para separar bloques y mejorar la legibilidad.
Línea 3: `Estructura principal:`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 4: `- prgProject/main.cpp`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 5: `- prgProject/CMakeLists.txt`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 6: `- lib/objStepper/*.h`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 7: `- lib/objMotion/*.h`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 8: `- src/srcStepper/*.cpp y stepper.pio`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 9: `- src/srcMotion/*.cpp`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 10: ``
  Explicación: Línea en blanco usada para separar bloques y mejorar la legibilidad.
Línea 11: `Build desde terminal:`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 12: ``
  Explicación: Línea en blanco usada para separar bloques y mejorar la legibilidad.
Línea 13: `cd template/prgProject`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 14: `mkdir build`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 15: `cd build`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 16: `cmake -G "Ninja" ..`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 17: `cmake --build .`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 18: ``
  Explicación: Línea en blanco usada para separar bloques y mejorar la legibilidad.
Línea 19: `Notas:`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 20: `- Requiere Raspberry Pi Pico SDK instalado o PICO_SDK_FETCH_FROM_GIT=ON.`
  Explicación: Asigna o actualiza un valor de configuración, estado, cálculo intermedio o salida.
Línea 21: `- Los pines STEP del ejemplo son GPIO 2, 3 y 4.`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 22: `- El PIO reserva GPIO 2 a GPIO 9 como grupo de STEP consecutivo.`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 23: `- El orden HOME se configura por nombre en main.cpp:`
  Explicación: Elemento de sintaxis del archivo que contribuye a la definición, configuración o documentación del proyecto.
Línea 24: `  motion.setHomeOrderByName({"Z", "X", "Y"});`
  Explicación: Invoca una función/método o declara una firma de función según el contexto del archivo.

Resumen funcional:
- README_BUILD.txt participa en el SDK aportando archivo de documentación o contexto del proyecto.
- Para modificarlo, revise primero sus includes/dependencias y después verifique que las declaraciones de encabezados coincidan con las implementaciones.
