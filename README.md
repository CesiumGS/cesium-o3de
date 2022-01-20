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

### **Getting Started**

#### **Add Georeference**
- Before being able to view 3D Tiles in the global scale, you will need to setup georeference entity for the level. The entity will place the engine coordinate origin to be at a particular place on the globe. It ensures that objects near the viewer can be represented very precisely. Below is the instruction to add georeference to the level:
  + In the level entity, which is the very top entity in the `Entity Outliner`, click on `Add Component`, then search for and add the `Level Coordinate Transform` component to the entity
  + Then, right click on the level entity and choose `Create entity`. For the newly-created entity, add `Georeference` component into it.
  + Within the `Georeference` component, you can use ECEF cartesian or cartographic to specify where on the globe you want to place the O3DE coordinate origin. For example, if you want to the O3DE coordinate origin to be at New York City, in the `Origin` group, choose `Cartographic` type. Then, enter `Longitude` to be -73.99 deg, `Latitude` to be 40.736 deg, and `Height` to be 20.0 m. Finally, click on `Set As Level Georeference`. Every entity that you add to the editor will have the position relative to this coordinate, or relative to the New York City area.
  + You can have multiple georeference entities in the scene, but at most there is only one active georeference at a time. You can specify which georeference entity to be an active georeference in the level by clicking `Set As Level Georeference` in the UI of `Georeference` component. To view which georeference entity is currently active, click on Level entity, and this info is saved in the `Level Coordinate Transform` component. 

#### **Add Cesium World Terrain**
- In the Entity Outliner, righ click and choose `Create Entity`. Please make sure that this entity is highlighted in the Entity Outliner, so that we can add Cesium World Terrain `3D Tile` component to this entity
- Click on the Cesium logo in the tool bar menu to open Cesium Ion panel.
- In the Cesium Ion panel, login to your Cesium Ion account to see Cesium World Terrain
- Once successfully login to your account, you will see `Quick Add Cesium Ion Assets` with the option to add Cesium World Terrain with different imageries.
- Click the plus symbol to add 3D tile component to the currently highlighted entity.