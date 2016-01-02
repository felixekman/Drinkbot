Drinkbot
========

## State Machine
```
                   BOOTING
                      |
                      v
   (collect data) LISTENING <-+
        API called? Y |       |
                      v       |
                    START     |
                      |       | finished? Y
                      v       |
              +--> PUMPING    |
  finished? N |       |       |
              +-------+-------+
                      |
                      v
                     end (never reach here)
```

## APIs
```
[librae@centos build]$ spark list
Checking with the cloud...
Retrieving cores... (this might take a few seconds)
core-librae (111111111111111111111111) is online
  Variables:
    temperature (double)
    alcohol1 (double)
    alcohol2 (double)
  Functions:
    int setpumps(String args)
    int debug(String args)
```

### Variables
```
$ spark get core-librae temperature
1454
```
or
```
$ curl https://api.spark.io/v1/devices/111111111111111111111111/temperature?access_token=ffffffffffffffffffffffffffffffffffffffff

{
  "cmd": "VarReturn",
  "name": "temperature",
  "result": 1446,
  "coreInfo": {
    "last_app": "",
    "last_heard": "2015-01-03T16:27:09.022Z",
    "connected": true,
    "deviceID": "111111111111111111111111"
  }
}
```

### Functions

```
$ spark call core-librae setpumps "P0:10;P1:00;P2:20;P3:05;P4:07;P5:15;P6:00;P7:10"
0
```
or
```
$ curl https://api.spark.io/v1/devices/111111111111111111111111/setpumps \
	-d access_token=ffffffffffffffffffffffffffffffffffffffff \
	-d "args=P0:03;P1:03;P2:10;P3:12;P4:01;P5:02;P6:04;P7:00"

{
  "id": "111111111111111111111111",
  "name": "core-librae",
  "last_app": null,
  "connected": true,
  "return_value": 0
}
```
