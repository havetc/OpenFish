wget https://github.com/havetc/OpenFish/archive/master.zip
unzip master.zip
mv -f OpenFish-master/* .
rm -rf OpenFish-master/
rm master.zip
sudo apt-get -y install libqt4-dev
sudo apt-get -y install libopencv-dev
sudo apt-get -y install libxvidcore4
sudo apt-get -y install libav-tools
qmake-qt4 OpenFish.pro
make -j 2
