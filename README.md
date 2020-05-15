Simulator api using Coppeliasim
=============================



Installing on linux
=========================================

### Install dependencies
``` bash
sudo apt install libzmq3-dev libboost-all-dev
```

Coppelia Sim dependencies:
```
sudo apt install libavcodec-dev libavformat-dev libswscale-dev
```


### Pull down all repositories
``` bash
git submodule update --init --recursive
```

### Build matmake
```
make -C matmake install
```

### Build b0
``` bash
cd lib/bluezero/
mkdir build
cd build
cmake ..
make b0
../../..
```

### Build program

``` bash
matmake
```

or debug build:

```
matmake config=debug
```

### Coppelia Documentation
[YouTube example of setting up a project](https://www.youtube.com/watch?v=9lOLyM5siTw)

[The B0 (BlueZero) API](https://github.com/CoppeliaRobotics/bluezero) uses [ZeroMq](https://zeromq.org/) under the hood.

#### Settings up the scene
This is described [here](https://coppeliarobotics.com/helpFiles/en/b0RemoteApiServerSide.htm)
1. Start the addon script ***simAddOnScript-b0RemoteApiServer.lua*** through the menu *Add-ons->b0RemoteApiServer* or to make it start automatically rename the file (its in the root of the sim) by replacing the "-" with "_" to become ***simAddOnScript_b0RemoteApiServer.lua***
2. Open your scene and drag in the file ***B0 remote Api server.ttm***. This will start the B0 Api Server
