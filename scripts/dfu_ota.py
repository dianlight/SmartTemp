Import("env")

env.AlwaysBuild(env.Alias("dfu-ota", "$BUILD_DIR/${PROGNAME}.elf",["cd ../Esp8266_DFU_Firmware/ && pio run --target upload"]))
