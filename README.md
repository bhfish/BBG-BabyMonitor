# **Third Eye Baby Monitor System**

Our product will serve as a realtime baby monitoring system that alerts the parent if an issue is detected. Specifically, it will be comprised of two BeagleBone Greens (one with the parent and the other with the baby) and a PC to connect to and set up the system. The system will have a web interface to allow the parents to see real time data, such as current temperature or view the video streaming in realtime. While the baby monitoring portion will be able to alert on certain conditions such as excessive noise levels and temperatures outside of a set safe range. When these conditions are detected the baby monitor will send a message to the parent’s BeagleBone and thus sounding its buzzer. As well the parent’s BeagleBone also supports the features of being able to view the current status (temperature, decibel, etc.) through its 4-digit display and its joystick to toggle between various modes.  

### **Glossary**
1. B_BBG := BeagleBone that monitors in baby's room
2. P_BBG := BeagleBone that carried by parent
3. $ := host shell/terminal
4. \# := target/BeagleBone shell/terminal

### **BeagleBone System Dependencies** 
1. ffmpeg - see guide/TBA 

### **Host System Dependencies**
1. openssh-server- sudo apt-get install openssh-server
2. arm cross compiler - sudo apt-get install gcc-arm-linux-gnueabihf
3. asound dev lib - sudo apt-get install libasound2-dev
4. nodejs project manager - sudo apt-get install npm

### **Installation**

1. obtain the source: TBA
2. complie from source
```
$ tar -xvf Third-Eye-Baby-Monitor.tar.gz
$ cd Third-Eye-Baby-Monitor/
$ make
```

3. set up both B_BBG and P_BBG's environment
```
$ cd web/
$ npm install
$ scp babyMonitor root@<B_BBG's IP>:/root/
$ scp -r video/ root@<B_BBG's IP>:/root/
$ scp -r web/ root@<B_BBG's IP>:/root/
$ scp parentMonitor root@<P_BBG's IP>:/root/
```

### **Execution**
1. from P_BBG
```
# ./parentMonitor
```

2. from B_BBG
```
# ./babyMonitor
# cd web/
# nodejs --harmony server.js
```

At this point, you can access to the system by entering the following URL from your host

(192.168.7.2:8088)

To configure the system to be automatically running upon BeagleBone boots
```
$ scp -r script/ root@<B_BBG's IP>:/root/
$ cd script
$ systemctl enable babyMonitor*.service
$ scp -r script/ root@<P_BBG's IP>:/root/
$ cd script
$ systemctl enable parentMonitor*.service
```

**NOTE: Above instructions will be adjusted in the future after this entire system becomes a product. Ideally, after the customers recieved the product, they should configure all the neccessary settings (network IP/ports; temperature/decibel tolerance etc.) via an intuitive GUI once the two BBGs powered up**



