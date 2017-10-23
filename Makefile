CROSS_COMPILE =arm-linux-gnueabihf-gcc
BABY_MONITOR_INC_DIR = inc
BABY_MONITOR_LIB_INC_DIR = lib
BABY_INC_PATH = $(BABY_MONITOR_INC_DIR)/baby
PARENT_INC_PATH = $(BABY_MONITOR_INC_DIR)/parent
BBG_HW_LIB_PATH = $(BABY_MONITOR_LIB_INC_DIR)/BBGHardware
BABY_MONITOR_BABY_OBJ_FILE = babyMonitor
BABY_MONITOR_PARENT_OBJ_FILE = parentMonitor
BABY_MONITOR_SRC_FILE = $(wildcard *.c)
BBG_HW_LIB_SRC_FILE = $(wildcard $(BBG_HW_LIB_PATH)/*.c)
NFS_PATH = $(HOME)/cmpt433/public/myApps
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -I $(BABY_INC_PATH) -I $(BBG_HW_LIB_PATH) -pthread

all: baby

baby:
	$(CROSS_COMPILE) $(CFLAGS) $(BBG_HW_LIB_SRC_FILE) $(BABY_MONITOR_SRC_FILE) -o $(BABY_MONITOR_BABY_OBJ_FILE) $(LFLAGS)
	cp $(BABY_MONITOR_BABY_OBJ_FILE) $(NFS_PATH)/

clean:
	rm -f $(NFS_PATH)/$(BABY_MONITOR_BABY_OBJ_FILE)