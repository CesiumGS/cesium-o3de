## **How to install Cesium Gem**

### **Prerequisite**

- Make sure all the hardware and software meet the O3DE's system requirement which is specified [here](https://o3de.org/docs/welcome-guide/setup/requirements/).
- O3DE Engine can be downloaded and installed via this [link](https://www.o3de.org/download/)
- [CMake 3.20.5 or later](https://cmake.org/download/#latest)

### **Clone the Gem repo**

- Check out the Gem repo with:

    ```
    git clone git@github.com:CesiumGS/cesium-o3de.git --recurse-submodules
    ```

- If you forget the `--recurse-submodules`, nothing will work because the git submodules will be missing. You can fix it with:

    ```
    git submodule update --init --recursive
    ```

### **Compile Cesium Native and third party libraries for the Gem**

- Before using the Gem for your project, you will need to build and package Cesium Native and other third party libraries first. Below is the instructions to build and package the libraries assuming that the current working directory is the Cesium Gem repo directory `cesium-o3de`:

    ```
    cd External
    cmake -B Build -S .
    cmake --build Build --config release --target install
    ```

- Once the build finishes, all the library binaries and headers will be packaged in the `External/Packages/Install` directory. 

### **Add Cesium Gem to O3DE Engine**

- Open the O3DE project manager, click on `Add Existing Gem` in the `Configure Gems` panel 
- Select the cloned directory containing the Cesium Gem
- In the `Configure Gems` panel, enable the `Cesium Gem`
