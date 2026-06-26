#ifndef SD_RECIPE_STORAGE_H
#define SD_RECIPE_STORAGE_H

#include "pico/stdlib.h"

static const char* const DEFAULT_RECIPE_PATH = "0:/config/axes.txt";

struct SdSpiConfig {
    uint spiIndex;
    uint misoPin;
    uint csPin;
    uint sckPin;
    uint mosiPin;
    uint baudRateHz;

    static SdSpiConfig createDefault();
};

class SdRecipeStorage {
private:
    SdSpiConfig spiConfig;
    char lastError[160];

    void clearError();
    bool fail(const char* message);

public:
    explicit SdRecipeStorage(const SdSpiConfig& selectedSpiConfig);

    bool begin();
    bool readTextFile(
        const char* filePath,
        char* outputBuffer,
        uint outputBufferSize
    );
    void end();

    const char* getLastError() const;
};

#endif
