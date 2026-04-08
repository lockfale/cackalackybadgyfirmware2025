#pragma once
#include <ArduinoJson.h>

/*
    configData.h -- part of the cackalackabadgy2023 project.
    Function declarations for saving config to LittleFS
*/
namespace CFG {

    void saveSecret( void );
    void printFiles( void );
    void printVERSION(void);
    String getSecretFile( void );

}
