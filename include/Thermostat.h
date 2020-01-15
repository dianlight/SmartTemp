#pragma once

#define TARGET_TEMP         (myConfig.get()->targetTemp[myConfig.get()->hold] + \
                            ( \
                                myConfig.get()->away?myConfig.get()->awayModify:0 \
                            ))

#define CURRENT_MODE_STR     myConfig.MODES_NAME[myConfig.get()->mode]
#define CURRENT_MODE_MQTT    myConfig.MODES_MQTT_NAME[myConfig.get()->mode]

#define CURRENT_HOLD_STR     myConfig.HOLDS_NAME[myConfig.get()->hold]
#define CURRENT_HOLD_MQTT    myConfig.HOLDS_MQTT_NAME[myConfig.get()->hold]

#define CURRENT_ACTION_MQTT  heating?"heating":((myConfig.get()->mode == Config::MODES::OFF)?"off":"idle"); // idle, cooling, heating, drying, or off
