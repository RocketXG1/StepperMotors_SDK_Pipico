#include "SdRecipeStorage.h"

#include <cstdio>
#include <cstring>

#include "hardware/gpio.h"
#include "hardware/spi.h"

#if STEPPER_ENABLE_MICROSD_FATFS
#include "ff.h"
#endif

SdSpiConfig SdSpiConfig::createDefault() {
    SdSpiConfig config = {};
    config.spiIndex = 0;
    config.misoPin = 16;
    config.csPin = 17;
    config.sckPin = 18;
    config.mosiPin = 19;
    config.baudRateHz = 1000 * 1000;
    return config;
}

SdRecipeStorage::SdRecipeStorage(const SdSpiConfig& selectedSpiConfig) {
    spiConfig = selectedSpiConfig;
    clearError();
}

void SdRecipeStorage::clearError() {
    lastError[0] = '\0';
}

bool SdRecipeStorage::fail(const char* message) {
    std::snprintf(lastError, sizeof(lastError), "%s", message);
    return false;
}

const char* SdRecipeStorage::getLastError() const {
    return lastError;
}

bool SdRecipeStorage::begin() {
    clearError();

    spi_inst_t* spiPort = (spiConfig.spiIndex == 0) ? spi0 : spi1;

    spi_init(spiPort, spiConfig.baudRateHz);
    gpio_set_function(spiConfig.misoPin, GPIO_FUNC_SPI);
    gpio_set_function(spiConfig.sckPin, GPIO_FUNC_SPI);
    gpio_set_function(spiConfig.mosiPin, GPIO_FUNC_SPI);

    gpio_init(spiConfig.csPin);
    gpio_set_dir(spiConfig.csPin, GPIO_OUT);
    gpio_put(spiConfig.csPin, 1);

#if STEPPER_ENABLE_MICROSD_FATFS
    static FATFS filesystem;
    FRESULT result = f_mount(&filesystem, "0:", 1);
    if (result != FR_OK) {
        return fail("Could not mount microSD filesystem.");
    }
    return true;
#else
    return fail("microSD FatFs backend is not enabled. Set STEPPER_ENABLE_MICROSD_FATFS=ON and provide FatFs/SD disk I/O.");
#endif
}

bool SdRecipeStorage::readTextFile(
    const char* filePath,
    char* outputBuffer,
    uint outputBufferSize
) {
    if (outputBuffer == nullptr || outputBufferSize == 0) {
        return fail("Invalid output buffer.");
    }

    outputBuffer[0] = '\0';

#if STEPPER_ENABLE_MICROSD_FATFS
    FIL file;
    FRESULT result = f_open(&file, filePath, FA_READ | FA_OPEN_EXISTING);
    if (result != FR_OK) {
        return fail("Could not open recipe file on microSD.");
    }

    UINT bytesRead = 0;
    result = f_read(&file, outputBuffer, outputBufferSize - 1, &bytesRead);
    f_close(&file);

    if (result != FR_OK) {
        outputBuffer[0] = '\0';
        return fail("Could not read recipe file from microSD.");
    }

    outputBuffer[bytesRead] = '\0';
    return true;
#else
    (void)filePath;
    return fail("microSD FatFs backend is not enabled. Recipe file was not read from card.");
#endif
}

void SdRecipeStorage::end() {
#if STEPPER_ENABLE_MICROSD_FATFS
    f_mount(nullptr, "0:", 0);
#endif
}
