Import("env")

my_flags = env.ParseFlags(env['BUILD_FLAGS'])
#defines = {k: v for (k, v) in my_flags.get("CPPDEFINES")}
dfu = ""
version = ""
for item in my_flags.get("CPPDEFINES"):
    if item[0] == "SMT_VERSION":
        version = item[1]

env.Replace(PROGNAME="smarttemp_{}".format(version))

