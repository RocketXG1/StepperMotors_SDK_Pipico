MicroSD recipe loader
=====================

Objetivo
--------
El firmware intenta leer la receta de ejes desde una microSD por SPI en:

    0:/config/axes.txt

La microSD debe estar formateada en FAT/FAT32 y contener la carpeta /config.

Pines SPI por defecto
---------------------
SPI0:

    MISO = GP16
    CS   = GP17
    SCK  = GP18
    MOSI = GP19

Estos pines evitan el bloque STEP actual GP2..GP9.

Estructura recomendada en la microSD
------------------------------------

    /config/axes.txt
    /logs/

Notas de build
--------------
El proyecto deja preparada la clase SdRecipeStorage y la inicializacion SPI.
Para enlazar una implementacion real de FatFs/SD block I/O, compilar con:

    -DSTEPPER_ENABLE_MICROSD_FATFS=ON

La integracion espera que el target tenga disponibles ff.h y las funciones de
FatFs/disk I/O requeridas por la plataforma.

Mientras STEPPER_ENABLE_MICROSD_FATFS este en OFF, el firmware conserva una
receta embebida de demostracion con los mismos datos de axes.txt para poder
compilar y visualizar la integracion sin backend FatFs.
