## mac build

### Install prerequisites
```
brew install cmake git
echo "export PATH=/usr/local/bin:$PATH" >> ~/.bash_profile
source ~/.bash_profile
```

### 
```
# brew would install opencv >= 4.10, so we don't need to build from source
brew install opencv glog lapack eigen libdc1394 zbar
brew install nlohmann-json

git clone https://github.com/lagadic/visp.git

mkdir -p visp-build
cd visp-build
cmake ../visp
make -j$(sysctl -n hw.logicalcpu)


echo "export VISP_DIR=$PWD" >> ~/.bash_profile
source ~/.bash_profile


cmake ../vispWrapper
make
make install
```
