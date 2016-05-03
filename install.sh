sudo apt-get -y install libqt4-dev
sudo apt-get -y install libopencv-dev
sudo apt-get -y install libxvidcore4
sudo apt-get -y install libav-tools
qmake-qt4 OpenFish.pro
make -j 2
