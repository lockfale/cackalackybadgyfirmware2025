#include <ESP8266WiFi.h>
#include <Adafruit_SSD1306.h>
#include "uuidGenerator.h"


namespace UUIDG {

    char uuid[64];

    String getUUID( void )
    {
        String u(uuid);
        u.trim();
        return u;
    }

    void generateUUID( void )
    {
        MD5Builder md5;
        md5.begin();
        md5.add(WiFi.macAddress());
        md5.calculate();
        String md5str = md5.toString();
        String new_uuid = md5str.substring(0,8) + "-"
                            + md5str.substring(8,12) + "-"
                            + md5str.substring(12,16) + "-"
                            + md5str.substring(16,20) + "-"
                            + md5str.substring(20,32);
        strlcpy(uuid, new_uuid.c_str(), sizeof(uuid));
    }

}