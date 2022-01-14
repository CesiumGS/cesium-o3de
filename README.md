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

- If you forget the --recurse-submodules, nothing will work because the git submodules will be missing. You should be able to fix it with:

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

- Once the build finishes, all the library binaries and headers will be packaged in the `External/Packages/Install` directory. You will need to add this directory to the environment variable `LY_PACKAGE_SERVER_URL`, so that the engine can retrieve those libraries when building your project with the Gem enabled:

#### **Window**
- In the start menu, search for `Edit the system environment variables`
- Once `System Properties` panel opens, click on `Environment Variables...`
- In the `User variables` entry, click on `New`
- In the `Variable name` input, enter `LY_PACKAGE_SERVER_URL` 
- In the `Variable value` input, enter the file url that points to `External/Packages/Install`. For example, if the cloned Gem directory is in `C:/cesium-o3de`, then this value will be `file:///C:/cesium-o3de/External/Packages/Install`

### **Add Cesium Gem to O3DE Engine**
- Open the O3DE project manager, click on `Add Existing Gem` in the `Configure Gems` panel 
- Select the cloned directory containing the Cesium Gem
- In the `Configure Gems` panel, enable the `Cesium Gem`