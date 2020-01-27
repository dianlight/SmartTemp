Import("env", "projenv")
import subprocess
import os
import time
try:
    import configparser
except ImportError:
    import ConfigParser as configparser

def before_upload(source, target, env):
    env.Dump()
#    print("SOURCE:{}\n TARGET:{}\n ENV:{}\n".format(map(str,source),map(str,target),env.Dump()))
#    print("SOURCE:{}\n TARGET:{}\n ENV:{}\n".format(map(str,source),map(str,target),env.Dump()))
    source_s = source.pop()
#    target_s = target.pop();
#    print("SOURCE:{}\n TARGET:{}\n ".format(source_s,target_s))
#    print("before_upload ", env.get("PIOENV"), env.get("BUILD_DIR"))
    config = configparser.ConfigParser()
    config.read("platformio.ini")
    print(config.sections())
    build_env = "env:{}".format(env.get("PIOENV"))
    print("Current Environment: {}".format(build_env),build_env)
    if config.get(build_env,'upload_protocol',fallback="unk") != "espota":
        print("ATTENTION: DFU Firmware work only for espota protocol!")
        return
    dfu_poject_dir = config.get(build_env,"custom_dfu_project_dir")
    if source_s.get_size() < (450 * 1024):
        print("No DFU Firmware needed!")
        return    
    print("Size {} is over 50% of the avaliable space. Use DFU Firmware.".format(source_s.get_size()))
    subprocess.check_call("pio run -d {} -t upload".format(dfu_poject_dir),shell=True)
    waiting =True
    wait_time = config.getint(build_env,"custom_dfu_wait_time", fallback= 35)
    ping_host = config.get(build_env,"upload_port")
    print("Wait {} second for ESP Boot on {}".format(wait_time,ping_host))
    time.sleep(wait_time)
#    while waiting:
#        counter =0
#        t = os.system("ping {}".format(ping_host))
#        if t:
#            waiting=False
#            print("DFU ready!")
#        else:
#            counter +=1
#            time.sleep(1)
#            if counter == 1000: # this will prevent an never ending loop, set to the number of tries you think it will require
#                waiting = False 


#def after_upload(source, target, env):
#    print("after_upload")
#    # do some actions

#print("Current build targets", map(str, BUILD_TARGETS))

env.AddPreAction("upload", before_upload)
#env.AddPostAction("upload", after_upload)

#
# Custom actions for specific files/objects
#

#env.AddPreAction("$BUILD_DIR/${PROGNAME}.elf", [callback1, callback2,...])
#env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", callback...)