# curling
A C++ libcurl wrapper

## compile
```sh
git clone https://github.com/paul-caron/curling
cd curling
make
```

## dependencies
generally speaking you need these packages, the names might differ on your distro
```
sudo apt install curl libcurl libcurl-dev
```
on my ubuntu 24 machine, i installed the following:
```
sudo apt install curl libcurl3t64-gnutls libcurl4-openssl-dev libcurl4t64
```
also needs some compiling basic tools
```
sudo apt install g++ make
```

## documentation
for generating documentation, install doxygen and graphviz and then run:
```
make doc
```
or you can take a look into the precompiled pdf manual included in this repo: refman.pdf

