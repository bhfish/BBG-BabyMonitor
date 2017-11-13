CROSS_COMPILE =arm-linux-gnueabihf-gcc
BABY_MONITOR_INC_DIR = inc
BABY_MONITOR_LIB_INC_DIR = lib
BABY_INC_PATH = $(BABY_MONITOR_INC_DIR)/baby
PARENT_INC_PATH = $(BABY_MONITOR_INC_DIR)/parent
BBG_HW_LIB_PATH = $(BABY_MONITOR_LIB_INC_DIR)/BBGHardware
BABY_MONITOR_SYS_LIB_PATH = $(BABY_MONITOR_LIB_INC_DIR)/sys
BABY_MONITOR_BABY_OBJ_FILE = babyMonitor
BABY_MONITOR_PARENT_OBJ_FILE = parentMonitor
BABY_MONITOR_SRC_FILE = $(wildcard *.c)
BABY_MONITOR_MICROPHONE_SRC_FILE = $(wildcard sound/*.c)
BABY_MONITOR_WEB_DIR = web
BBG_HW_LIB_SRC_FILE = $(wildcard $(BBG_HW_LIB_PATH)/*.c)
NFS_PATH = $(HOME)/cmpt433/public/myApps
ASOUND_LIB_PATH = $(HOME)/cmpt433/public/asound_lib_BBB
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -I $(BABY_INC_PATH) -I $(BABY_MONITOR_SYS_LIB_PATH) -I $(BBG_HW_LIB_PATH)
LFLAGS = -L$(ASOUND_LIB_PATH) -lasound -lpthread -lm

# optional build flags
# DEBUG_MODE = -D DEBUG_MODE
# DEMO_MODE = -D DEMO_MODE

all: baby webServer

baby:
	$(CROSS_COMPILE) $(CFLAGS) $(BBG_HW_LIB_SRC_FILE) $(BABY_MONITOR_MICROPHONE_SRC_FILE) $(BABY_MONITOR_SRC_FILE) $(DEBUG_MODE) $(DEMO_MODE) -o $(BABY_MONITOR_BABY_OBJ_FILE) $(LFLAGS)
	cp $(BABY_MONITOR_BABY_OBJ_FILE) $(NFS_PATH)/

webServer:
	mkdir -p $(NFS_PATH)/$(BABY_MONITOR_WEB_DIR)/
	cp -R $(BABY_MONITOR_WEB_DIR)/* $(NFS_PATH)/$(BABY_MONITOR_WEB_DIR)/

clean:
	rm -f $(BABY_MONITOR_BABY_OBJ_FILE)
	rm -f $(NFS_PATH)/$(BABY_MONITOR_BABY_OBJ_FILE)
	rm -rf $(NFS_PATH)/$(BABY_MONITOR_WEB_DIR)