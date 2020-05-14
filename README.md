Simulator api using Coppeliasim
=============================



Installing on linux
=========================================

### Install dependencies
``` bash
sudo apt install libzmq3-dev libboost-all-dev
```

Coppelia Sim dependencies:
> sudo apt install libavcodec-dev libavformat-dev libswscale-dev


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

#### Settings up the scene
This is described [here](https://coppeliarobotics.com/helpFiles/en/b0RemoteApiServerSide.htm)
1. Start the addon script ***simAddOnScript-b0RemoteApiServer.lua***
1. Open a new document and drag in the file ***B0 remote Api server.ttm***. This will start the B0 Api Server
