MANUAL ACTUALIZADO - config/README_MICROSD_RECIPE.txt
================================================================================
Funcion inicial del archivo
- Documenta, parsea o proporciona recetas de maquina para configurar ejes desde texto/microSD.

Metodos, variables, secciones o instancias detectadas
- L2: `=====================`
- L16: `MISO = GP16`
- L17: `CS   = GP17`
- L18: `SCK  = GP18`
- L19: `MOSI = GP19`
- L34: `-DSTEPPER_ENABLE_MICROSD_FATFS=ON`

Quien lo manda a llamar / relacion con otros archivos
- Los headers en lib son incluidos por implementaciones en src y por prgProject/main.cpp.
- Los .cpp son compilados por prgProject/CMakeLists.txt y sus funciones se ejecutan desde main.cpp o desde otros objetos del controlador.
- Los archivos config/recipes/*.txt son consumidos por RecipeLoader/SdRecipeStorage/MachineRecipeParser cuando se carga una receta.
- Los manuales son referencia de uso y no participan en la compilacion.

Explicacion linea por linea
| Linea | Codigo | Explicacion |
|---:|---|---|
| 1 | `MicroSD recipe loader` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 2 | `=====================` | Asigna o configura un valor; en recetas define un parametro usado por el firmware. |
| 3 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 4 | `Objetivo` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 5 | `--------` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 6 | `El firmware intenta leer la receta de ejes desde una microSD por SPI en:` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 7 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 8 | `    0:/config/axes.txt` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 9 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 10 | `La microSD debe estar formateada en FAT/FAT32 y contener la carpeta /config.` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 11 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 12 | `Pines SPI por defecto` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 13 | `---------------------` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 14 | `SPI0:` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 15 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 16 | `    MISO = GP16` | Asigna o configura un valor; en recetas define un parametro usado por el firmware. |
| 17 | `    CS   = GP17` | Asigna o configura un valor; en recetas define un parametro usado por el firmware. |
| 18 | `    SCK  = GP18` | Asigna o configura un valor; en recetas define un parametro usado por el firmware. |
| 19 | `    MOSI = GP19` | Asigna o configura un valor; en recetas define un parametro usado por el firmware. |
| 20 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 21 | `Estos pines evitan el bloque STEP actual GP2..GP9.` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 22 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 23 | `Estructura recomendada en la microSD` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 24 | `------------------------------------` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 25 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 26 | `    /config/axes.txt` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 27 | `    /logs/` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 28 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 29 | `Notas de build` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 30 | `--------------` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 31 | `El proyecto deja preparada la clase SdRecipeStorage y la inicializacion SPI.` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 32 | `Para enlazar una implementacion real de FatFs/SD block I/O, compilar con:` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 33 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 34 | `    -DSTEPPER_ENABLE_MICROSD_FATFS=ON` | Asigna o configura un valor; en recetas define un parametro usado por el firmware. |
| 35 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 36 | `La integracion espera que el target tenga disponibles ff.h y las funciones de` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 37 | `FatFs/disk I/O requeridas por la plataforma.` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 38 | ` ` | Linea en blanco usada para separar bloques y mejorar lectura. |
| 39 | `Mientras STEPPER_ENABLE_MICROSD_FATFS este en OFF, el firmware conserva una` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 40 | `receta embebida de demostracion con los mismos datos de axes.txt para poder` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
| 41 | `compilar y visualizar la integracion sin backend FatFs.` | Instruccion de codigo/configuracion que participa en el flujo descrito por el bloque donde aparece. |
