Proyecto: template_motion_project

Estructura principal:
- prgProject/main.cpp
- prgProject/CMakeLists.txt
- lib/objStepper/*.h
- lib/objMotion/*.h
- src/srcStepper/*.cpp y stepper.pio
- src/srcMotion/*.cpp

Build desde terminal:

cd template/prgProject
mkdir build
cd build
cmake -G "Ninja" ..
cmake --build .

Notas:
- Requiere Raspberry Pi Pico SDK instalado o PICO_SDK_FETCH_FROM_GIT=ON.
- Los pines STEP del ejemplo son GPIO 2, 3 y 4.
- El PIO reserva GPIO 2 a GPIO 9 como grupo de STEP consecutivo.
- El orden HOME se configura por nombre en main.cpp:
  motion.setHomeOrderByName({"Z", "X", "Y"});
