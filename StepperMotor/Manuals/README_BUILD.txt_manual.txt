MANUAL ACTUALIZADO - README_BUILD.txt
================================================================================
Funcion inicial del archivo
- Archivo de soporte/documentacion del proyecto.

Metodos, variables, secciones o instancias detectadas
- L20: `- Requiere Raspberry Pi Pico SDK instalado o PICO_SDK_FETCH_FROM_GIT=ON.`
- L24: `motion.setHomeOrderByName({"Z", "X", "Y"});`

Quien lo manda a llamar / relacion con otros archivos
- Los headers en lib son incluidos por implementaciones en src y por prgProject/main.cpp.
- Los .cpp son compilados por prgProject/CMakeLists.txt y sus funciones se ejecutan desde main.cpp o desde otros objetos del controlador.
- Los archivos config/recipes/*.txt son consumidos por RecipeLoader/SdRecipeStorage/MachineRecipeParser cuando se carga una receta.
- Los manuales son referencia de uso y no participan en la compilacion.

Explicacion linea por linea
| Linea | Codigo | Explicacion |
|---:|---|---|
| 1 | `Proyecto: template_motion_project` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 2 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 3 | `Estructura principal:` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 4 | `- prgProject/main.cpp` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 5 | `- prgProject/CMakeLists.txt` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 6 | `- lib/objStepper/*.h` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 7 | `- lib/objMotion/*.h` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 8 | `- src/srcStepper/*.cpp y stepper.pio` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 9 | `- src/srcMotion/*.cpp` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 10 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 11 | `Build desde terminal:` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 12 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 13 | `cd template/prgProject` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 14 | `mkdir build` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 15 | `cd build` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 16 | `cmake -G "Ninja" ..` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 17 | `cmake --build .` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 18 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 19 | `Notas:` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 20 | `- Requiere Raspberry Pi Pico SDK instalado o PICO_SDK_FETCH_FROM_GIT=ON.` | Asigna o configura un valor; en recetas define un parametro usado por el firmware. |
| 21 | `- Los pines STEP del ejemplo son GPIO 2, 3 y 4.` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 22 | `- El PIO reserva GPIO 2 a GPIO 9 como grupo de STEP consecutivo.` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 23 | `- El orden HOME se configura por nombre en main.cpp:` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 24 | `  motion.setHomeOrderByName({"Z", "X", "Y"});` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
