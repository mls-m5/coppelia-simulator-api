Simulator api using Coppeliasim
=============================



Installing on linux
=========================================

### Install dependencies
``` bash
sudo apt install libzmq3-dev
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
