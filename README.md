# OpenFish

This project allows to deform videos in order to make them visible after projection by fish-eye. The computer should have video
codec available. Under microsoft, it could be needed to install for example the K-lite codec pack in case of troubles.

# Compilation / Installation

**Windows**

This project can be compiled under microsoft windows. However there is no easy way to install the dependencies, so it is
recommended to use the pre-build version.

**On Ubuntu**

The easiest way to install Openfish is by launching the install script on Ubuntu. It may also work on others distributions
working with the aptitude package manager.

`sudo ./install_Openfish_Ubuntu`

This script downloads automatically the latest files of OpenFish, then install all the dependencies
needed by calling multiples apt-get install. After that it compiles the program.

*Warning for developers!*

The script doesn't check if dependencies are already manually installed. Therefore, if you have installed a custom
version of Qt or OpenCV for example, you may want to comment the installation of thoses packages in the script in order
to keep your configuration intact.

**Manual installation**

If yout want to manually install the project, or your system has an other package manager than aptitude, you should
still be able to install OpenFish. Check the install script to know the dependencies, then intall all of them on your
system.

If all dependencies are met, then:

`qmake`

`make -j 4`

should compile the project


**Special thanks**

MJC Pont du Sonnant
Lionel Ruiz, for the good suggestions
My friend Viktorija, for translating OpenFish in Russian
