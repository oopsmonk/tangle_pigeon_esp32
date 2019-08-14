# TanglePigeon 

The TanglePigeon carries a message to the Tangle.   
This is an example project shows how to send messages to the Tangle with [CClient library](https://github.com/iotaledger/entangled/tree/develop/cclient) and [ESP32](https://en.wikipedia.org/wiki/ESP32), the message can be triggered via GPIO (BOOT button) or Timer.  
The message is in JSON format shown below:  

```
{
    "TanglePigeon":"v0.0.1",
    "Trigger":"TIMER",
    "LocalTime":"2019-08-12 13:23:47",
    "TimeZone":"CST-8",
    "Data":"Hello IOTA"
}
```

You can change or add some sensor data to the message as you need, like temperature, humidity, location...  
**Notice:** Messages will be removed after the IRI snapshot because they are zero-value transactions.  

## The Flowchart  

![](https://raw.githubusercontent.com/oopsmonk/tangle_pigeon_esp32/master/images/tangle_pigeon.png)

## Requirements  

* [ESP32-DevKitC V4](https://docs.espressif.com/projects/esp-idf/en/v3.2.2/get-started/get-started-devkitc.html#esp32-devkitc-v4-getting-started-guide)
* xtensa-esp32 toolchain
* ESP-IDF v3.2.2

## ESP32 build system setup  

Please follow documentations to setup your toolchain and development framework.

Linux:  
* [xtensa-esp32 toolchain](https://docs.espressif.com/projects/esp-idf/en/v3.2.2/get-started-cmake/linux-setup.html) 
* [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/v3.2.2/get-started-cmake/index.html#get-esp-idf) 

Windows:
* [xtensa-esp32 toolchain](https://docs.espressif.com/projects/esp-idf/en/v3.2.2/get-started-cmake/windows-setup.html#standard-setup-of-toolchain-for-windows-cmake) 
* [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/v3.2.2/get-started-cmake/index.html#windows-command-prompt) 

**Notice: We use the ESP-IDF v3.2.2, make sure you clone the right branch of ESP-IDF**

```
git clone -b v3.2.2 --recursive https://github.com/espressif/esp-idf.git
```


Now, you can test your develop environment via the [hello_world](https://github.com/espressif/esp-idf/tree/release/v3.2/examples/get-started/hello_world) project.  

```shell
cd ~/esp
cp -r $IDF_PATH/examples/get-started/hello_world .
idf.py menuconfig
idf.py build
idf.py -p /dev/ttyUSB0 flash && idf.py -p /dev/ttyUSB0 monitor
```

The output would be something like:  

```shell
I (0) cpu_start: App cpu up.
I (184) heap_init: Initializing. RAM available for dynamic allo
cation:
I (191) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (197) heap_init: At 3FFB2EF8 len 0002D108 (180 KiB): DRAM
I (204) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (210) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (216) heap_init: At 40089560 len 00016AA0 (90 KiB): IRAM
I (223) cpu_start: Pro cpu start user code
I (241) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
Hello world!
This is ESP32 chip with 2 CPU cores, WiFi/BT/BLE, silicon revision 1, 4MB external flash
Restarting in 10 seconds...
Restarting in 9 seconds...
```

You can press `Ctrl` + `]` to exit the monitor and ready for the next setup.  

## Building and flashing to ESP32

### Step 1: cloning repository  

```shell
git clone --recursive https://github.com/oopsmonk/tangle_pigeon_esp32.git
```

Or (if you didn't put the `--recursive` command during clone)  

```shell
git clone https://github.com/oopsmonk/tangle_pigeon_esp32.git
cd tangle_pigeon_esp32
git submodule update --init --recursive
```

### Step 2: initializing components

The `init.sh` helps us to generate files and switch to the right branch for the components.  

Linux:

```shell
cd tangle_pigeon_esp32
bash ./init.sh
```

Windows: **TODO**  

### Step 3: Configuration  

In this step, you need to set up the WiFi, SNTP, IRI node, and a recursive.  

```
idf.py menuconfig
# WiFi SSID & Password
[Tangle Pigeon] -> [WiFi]

# SNTP Client
[Tangle Pigeon] -> [SNTP]

# Default IRI node
[Tangle Pigeon] -> [IRI Node]

# Wake up timer
[Tangle Pigeon] -> (5) Wake up time (minutes)

```

You can check configures in `sdkconfig` file.  

Please make sure you assigned the receiver(`CONFIG_MSG_RECEIVER`), Here is an example for your configuration:  

```shell
CONFIG_SNTP_SERVER="pool.ntp.org"
CONFIG_SNTP_TZ="CST-8" 
CONFIG_MSG_RECEIVER="RECEIVER9ADDRESS9RECEIVER9ADDRESS9RECEIVER9ADDRESS9RECEIVER9ADDRESS9RECEIVER9ADDR"
CONFIG_WAKE_UP_TIME=5
CONFIG_IOTA_DEPTH=6
CONFIG_IOTA_MWM=9
CONFIG_IRI_NODE_URI="nodes.devnet.iota.org"
CONFIG_IRI_NODE_PORT=443
CONFIG_ENABLE_HTTPS=y
CONFIG_WIFI_SSID="MY_SSID"
CONFIG_WIFI_PASSWORD="MY_PWD"
```

The `CONFIG_SNTP_TZ` follows the [POSIX Timezone string](https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json)  

### Step 4: Build & Run

```shell
idf.py build
idf.py -p /dev/ttyUSB0 flash && idf.py -p /dev/ttyUSB0 monitor
```

Output:  
```shell
I (3724) TanglePigeon: Connected to AP
I (3724) TanglePigeon: IRI Node: nodes.thetangle.org, port: 443, HTTPS:True
I (3734) TanglePigeon: Initializing SNTP: pool.ntp.org, Timezone: CST-8
I (3744) TanglePigeon: Waiting for system time to be set... (1/10)
I (5744) TanglePigeon: The current date/time is: Tue Aug 13 13:45:22 2019
Power on reset
I (24624) message-builder: transaction sent: OK
I (25634) TanglePigeon: Enabling timer wakeup, 2 minutes
I (25634) TanglePigeon: Entering deep sleep
I (25634) wifi: state: run -> init (0)
I (25634) wifi: pm stop, total sleep time: 19342634 us / 22908089 us
I (25644) wifi: n:11 0, o:11 0, ap:255 255, sta:11 0, prof:1
I (25664) wifi: flush txq
I (25664) wifi: stop sw txq
I (25664) wifi: lmac stop hw txq
E (25664) wifi: esp_wifi_connect 968 wifi not start
ets Jun  8 2016 00:22:57
rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0018,len:4
load:0x3fff001c,len:6160
ho 0 tail 12 room 4
load:0x40078000,len:10180
load:0x40080400,len:6660
entry 0x40080764
I (31) boot: ESP-IDF v3.2.2 2nd stage bootloader
I (31) boot: compile time 12:50:41
I (31) boot: Enabling RNG early entropy source...
......
I (3224) TanglePigeon: Connected to AP
I (3224) TanglePigeon: IRI Node: nodes.thetangle.org, port: 443, HTTPS:True

I (3234) TanglePigeon: Initializing SNTP: pool.ntp.org, Timezone: CST-8
I (3244) TanglePigeon: Waiting for system time to be set... (1/10)
I (5244) TanglePigeon: The current date/time is: Tue Aug 13 13:47:47 2019
Wake up from timer. Time spent in deep sleep: 125572ms
I (38344) message-builder: transaction sent: OK
I (39344) TanglePigeon: Enabling timer wakeup, 2 minutes
I (39344) TanglePigeon: Entering deep sleep
I (39344) wifi: state: run -> init (0)
I (39344) wifi: pm stop, total sleep time: 32638765 us / 36729138 us

I (39354) wifi: n:11 0, o:11 0, ap:255 255, sta:11 0, prof:1
I (39384) wifi: flush txq
I (39384) wifi: stop sw txq
I (39384) wifi: lmac stop hw txq
E (39384) wifi: esp_wifi_connect 968 wifi not start
```

`Ctrl` + `]` to exit.  

A transaction screenshot:  

![](https://raw.githubusercontent.com/oopsmonk/tangle_pigeon_esp32/master/images/transaction_screenshot.png)  


## Troubleshooting

`CONFIG_MSG_RECEIVER` is not set or is invalid:  
```shell
I (0) cpu_start: Starting scheduler on APP CPU.
E (243) TanglePigeon: please set a valid address hash(CONFIG_MSG_RECEIVER) in sdkconfig!
I (243) TanglePigeon: Restarting in 30 seconds...
I (1253) TanglePigeon: Restarting in 29 seconds...
I (2253) TanglePigeon: Restarting in 28 seconds...
```

`CONFIG_MAIN_TASK_STACK_SIZE` is too small, you need to enlarge it:  
```shell
***ERROR*** A stack overflow in task main has been detected.
abort() was called at PC 0x4008af7c on core 0
```
