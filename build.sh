### https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html
cd build

# Install minimal prerequisites (Ubuntu 18.04 as reference)
sudo apt update && sudo apt install -y cmake g++ wget unzip
 
# Download and unpack OpenCV source code 
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.x.zip
unzip opencv.zip
 
# Create build directory
mkdir -p opencv_build && cd opencv_build
 
# Configure
cmake  ../opencv-4.x
 
# Build: Building/compiling own libraries (shared object .so files) from downloaded OpenCV source code 
cmake --build .

# By default OpenCV will be installed to the `/usr/local` directory, all files will be copied to following locations:
# Since `/usr/local` is owned by the root user, the installation should be performed with elevated privileges (sudo): 
sudo make install

# Write lib location for OpenCV module libs to the (newly created) file /etc/ld.so.conf.d/opencv.conf
sudo sh -c "echo '/usr/local/lib' > /etc/ld.so.conf.d/opencv.conf"
sudo ldconfig -v

# Premake & make
cd ../.. && build/premake-5.0.0-beta2-linux/premake5 --file=./build/premake5.lua gmake
cd build && make config=release_static64
