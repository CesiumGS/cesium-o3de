## **Overview**

This is a summary of a setup and workflows for developers who want to work with the *Cesium for O3DE* Gem. Such a setup consists of three main components:

- [`cesium-native`](https://github.com/CesiumGS/cesium-native) : A collection of engine-independent libraries for 3D Tiles, geospatial, etc. Most of the functionality of *Cesium for O3DE* is built based on these libraries.
- [`cesium-o3de`](https://github.com/CesiumGS/cesium-o3de) : The source code of the actual *Cesium for O3DE* Gem.
- An O3DE project that uses the Gem. You can use the [`cesium-o3de-samples`](https://github.com/CesiumGS/cesium-o3de-samples) here to get started quickly. It contains sample levels for different use cases, and can therefore be used to quickly check for possible regressions of feature changes.

> Note: It is generally possible to work with `cesium-native` *independent* of `cesium-o3de`. But any modification in `cesium-native` will have to be checked carefully for possible breaking changes in the API or the build process. So the following describes the developer setup from the perspective of someone who wants to work with `cesium-native` mainly in the context of `cesium-o3de`.

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

### **Add Cesium Gem to an O3DE Project**

- If you want to add the Gem to a new project, please follow those steps below:
    - Open the O3DE project manager, click on `Create a Project` or `Add a Project`
    - Click on `Add Existing Gem` in the `Configure Gems` panel 
    - Select the cloned directory containing the Cesium Gem
    - In the `Configure Gems` panel, enable the `Cesium Gem`
    - Click on `Build Project`
    - Once the project is finished building, click on `Open Editor` button to begin using the Gem

- If you want to use the [`cesium-o3de-samples`](https://github.com/CesiumGS/cesium-o3de-samples) for the Gem development, please follow its [`Get Started`](https://github.com/CesiumGS/cesium-o3de-samples#rocket-get-started) instructions.  

### **Open the project Visual Studio Solution**

- Once the project is finished building, open the menu button right next to the project's name and click on `Open the Project folder...`
- In the project directory, go to `build/windows_vs2019` directory
- You can find a Visual Studio `sln` solution file with the project name to open for development. The solution will also include the source code of the actual *Cesium for O3DE* Gem.