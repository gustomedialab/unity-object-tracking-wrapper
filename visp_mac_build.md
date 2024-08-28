## MACOS Build

### Install prerequisites
```
brew install cmake git

# if you use zsh, please change to ~/.zshrc
echo "export PATH=/usr/local/bin:$PATH" >> ~/.bash_profile
source ~/.bash_profile
```

### build visp
```
# brew would install opencv >= 4.10, so we don't need to build from source
brew install opencv glog lapack eigen libdc1394 zbar

# this is a 3rd-party lib for json which can be used as message transfer between client and server. We need this to build 'visp-dnn-trakcer'.
brew install nlohmann-json

git clone https://github.com/lagadic/visp.git

mkdir -p visp-build
cd visp-build
cmake ../visp
make -j$(sysctl -n hw.logicalcpu)
```

```
# if you use zsh, please change to ~/.zshrc
echo "export VISP_DIR=$PWD" >> ~/.bash_profile
source ~/.bash_profile
cmake ../vispWrapper
make
make install
```


### common issue
1. From Marco's Mac:
```
/Users/user/visp-ws/visp/modules/gui/src/display/vpDisplayX.cpp:2172:18: error: out-of-line definition of 'displayText' does not match any declaration in 'vpDisplayX'

/Users/user/visp-ws/visp/modules/gui/src/display/vpDisplayOpenCV.cpp:888:23: error: out-of-line definition of 'displayText' does not match any declaration in 'vpDisplayOpenCV'

/Users/user/visp-ws/visp/modules/io/src/image/vpImageIo.cpp:1409:17: error: out-of-line definition of 'readPNGfromMem' does not match any declaration in 'vpImageIo'
```
- solved by reinstall 'gsl'


