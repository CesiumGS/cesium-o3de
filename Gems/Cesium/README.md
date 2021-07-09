## **How to build O3DE engine and Cesium Gem**

### **Prerequisite**

- Make sure all the hardware and software meet the O3DE's system requirement which is specified [here](https://o3de.org/docs/welcome-guide/setup/requirements/).

### **Clone the forked repo**

- Check out the forked repo with:

```
git clone git@github.com:CesiumGS/o3de.git --recurse-submodules
```

- If you forget the --recurse-submodules, nothing will work because the git submodules will be missing. You should be able to fix it with:

```
git submodule update --init --recursive
```

- Check out the `cesium-main` branch which is our main branch for developing Cesium Gem. Any PRs will need to be reviewed before merging into this branch:

```
git checkout cesium-main
```

### **Compile O3DE engine**

- Before compiling the O3DE engine, you will need to build and package Cesium Native library first, which is needed by Cesium Gem. Below is the instructions to build and package the library assuming that the current working directory is the `o3de` repo directory:

```
cd Gems/Cesium/External
cmake -B Build -S .
cmake --build Build --config Release --target install
```

- After that, please follow O3DE instructions in this [link](https://o3de.org/docs/welcome-guide/setup/setup-from-github/#build-the-engine) to build the engine along with Cesium Gem

- After finishing the above steps, you should be able to find Cesium Gem in the O3DE editor when configuring a new project. The editor can be found in the install directory
  `o3de-install/bin/Windows/profile/o3de.exe`. The instructions about how to create a new project in the editor and CI can be found in this [link](https://o3de.org/docs/welcome-guide/get-started/project-config/)

## **How to build Atom sample viewer**

### **Prerequisite**

- Make sure the O3DE engine is compiled and configured using the above steps

### **Clone the forked repo**

- Check out the forked repo with:

```
git clone git@github.com:CesiumGS/o3de-atom-sampleviewer.git
```

- Check out the `cesium-main` branch which is our main branch for incorporating Cesium Gem into the sample viewer. Any PRs will need to be reviewed before merging into this branch:

```
git checkout cesium-main
```

### **Compile O3DE engine**

- Make sure O3DE Asset Processor is turned off first. You can check if the processor is running by opening the hidden icons, which usually have `Slack` or `Bluetooth Devices` or other icons in it, at the right bottom of the Window 10 taskbar. Right click on it and choose `quit`. Or you can open Task Manager and search for O3DE Asset Processor and force quit the process.

- Register the project path with the engine:

```
o3de-install/script/o3de.bat register --project-path <O3DE Atom sample viewer full path>
```

- Follow O3DE instructions in this [link](https://o3de.org/docs/atom-guide/setup-atom-projects/) to build the sample viewer

## **Appendix**

### **Add third party library to O3DE engine**

- This [link](https://o3de.org/docs/user-guide/build/packages/) contains O3DE instructions for how to add third party libraries to a project. The process for adding Cesium Native to the Cesium Gem is based on those instructions. It uses `PostInstall.cmake` script to automate all the steps
