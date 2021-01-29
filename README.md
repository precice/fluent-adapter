# preCICE-adapter for the CFD code ANSYS Fluent
*Developed by Bernhard Gatzhammer and valid for FLUENT 19.5 and preCICE v2.2 on Ubuntu 20.04*
*Updated and maintained done by Ishaan Desai*

## 1. How to build the Fluent-preCICE adapter:
  * Adapt `lnamd64/2ddp_host/user.udf` line 1 "CSOURCES=...": There are several main udf files
  * The variable SOURCES needs one main udf file and the correspoding .c files:
    + fsi_udf.c: This is the main ANSYS readable file used by Fluent to call functions during each iteration
                 for FSI simulations. This ANSYS UDF file needs fsi.c and fsi.h for execution
  * Adapt `lnamd64/2d_host/user.udf` line 3 `FLUENT_INC = ...` to the fluent installation
    folder which will be of the type `./ansys_inc/v195/fluent`
  * Repeat the above steps for `lnamd64/2d_node` folder in the exact same manner
  * Adapt the path of the python library in `src/makefile` line 19.  
    **NOTE**: Python shared library is available within the ANSYS installation. Example path is: `/ansys_inc/v195/commonfiles/CPython/2_7_15/linx64/Release/python/lib/libpython2.7.so`   
  * Update the ANSYS Release version in `src/makefile` line 26
    Example of line 26: `RELEASE=19.5.0`
  * Type: `make "FLUENT_ARCH=lnamd64"` to start the build
  * Use `make clean` to clean build process

## 2. Running FLUENT

### 2.1 Installation using ANSYS GUI

**General remarks if you use Ubuntu 20.04**

* Ubuntu 20.04 is not officially supported by ANSYS and hence only the FLUENT package works on this distribution. All other packages (ANSYS Workbench, etc.) do not work and hence the case setup needs to be done on a different compatible operating system. Current compatible distributions for ANSYS version 2019 R3 are: Ubuntu 16.04, CentOS 7.x, Linux Mint 18.x, Debian 9 (tested with 2019 R3, unknown for 2020 R2)
* Generally it is recommended to only install the required packages, since the installation process might break (tested with 2019 R3)

**ANSYS version 2020 R2 on Ubuntu 20.04**

* Run `./INSTALL` from the ANSYS directory and follow steps of installation as seen in the GUI
* The installation hangs between 80-90%. Close partially completed installation
* All ANSYS packages are installed in a folder `ansys_inc/` at the location defined by the user during installation. The FLUENT executable is located at `/ansys_inc/v202/fluent/bin`

**ANSYS version 2019 R3 on Ubuntu 20.04**

* Run `./INSTALL` from the ANSYS directory and follow steps of installation as seen in the GUI
* The installation completes successfully.
* All ANSYS packages are installed in a folder `ansys_inc/` at the location defined by the user during installation. The FLUENT executable is located at `/ansys_inc/v195/fluent/bin`

**ANSYS Version 2019 R3 on Ubuntu 16.04**

* All packages of ANSYS Version 2019 R3 work on Ubuntu 16.04 and this [forum post](https://www.cfd-online.com/Forums/ansys/199190-ansys-18-2-ubuntu-16-04-installation-guide.html) describes the installation process.

**Troubleshooting**

* If you try to start fluent via `fluent 2ddp` and the program exits with the error `Bad substitution`, the following [forum post}(https://www.cfd-online.com/Forums/fluent/149668-fluent-16-0-0-ubuntu-12-04-a.html) provides a solution. Short: `sudo dpkg-reconfigure dash`, answer **No** to the questions "Use dash as the default system shell (/bin/sh)?".
* If the error: `undefined symbol: FT_Done_MM_Var` is encountered on starting FLUENT, the following [forum post](https://www.cfd-online.com/Forums/fluent/227651-fluent-ubuntu-20-04-a.html) has the solution

  ### 2.1 Installation using ANSYS GUI
  
  * If the error: `undefined symbol: FT_Done_MM_Var` is encountered on starting FLUENT, the following [forum post](https://www.cfd-online.com/Forums/fluent/227651-fluent-ubuntu-20-04-a.html) has the solution


  ### 2.2 Running FLUENT (ANSYS Version 2019 R3) on Ubuntu 16.04
  

  ### 2.3 How to start Fluent without GUI
  * serial:   `fluent 2ddp -g < steer-fluent.txt`
  * parallel: `fluent 2ddp -g -t4 -mpi=openmpi < steer-fluent.txt`
    (-t4 sets 4 processes for computations)
    (steer-fluent.txt is a driver file for Fluent and is only written for convenience)

--------------------------------------------------------------------------------

## 3. Preparing a Fluent .cas file for UDF function usage:
  * Create a folder `libudf/` at the location `<project_dir>/dp0/FFF/Fluent`
  * Copy the folder `lnamd64/` from the adapter into folder `libudf/`
  * Start Fluent and open the project by opening the `.msh`, `.cas` or `.cas.gz` file in Fluent
  * In Fluent, go to top menu User Defined -> Functions... -> Manage... and
    load the `libudf/` folder by typing the name "libudf" in the *Library Name* option and clicking on Load.
    The names of the udf functions should appear in the Fluent command line window.
    (this needs to be done only once for a project)
  * Define a function hook at initialization. Go to top menu User Defined -> Function Hooks ->
    Initialization -> Edit -> select *init::libudf* from Available list -> click Add
  * Define 1 user defined memory for the faces. (define -> user defined -> memory -> 1;
    adds one additional double to each face for precice face ids)

    **------ TO BE COMPLETED ------**

--------------------------------------------------------------------------------

## 4. Preparing a Fluent .cas file for FSI simulations

  * Perform the steps in 1., 2. and 3.
  * Set a user defined mesh motion according to function "gridmotions".

    **------ TO BE COMPLETED ------**

--------------------------------------------------------------------------------

## 5. Preparing a Fluent .cas file for Wave simulations

Go through the menus on the left and perform the following steps:
  * General: Set transient simulation and gravity in y-direction = -9.81
  * Models: Set Multiphase VOF explicit with Implicit body force formulation activated
  * Materials: Air and water-liquid (from Fluent Database) are needed
  * Phases: Name phases "phase-1-air" and set air as material, as well as
    "phase-2-water" with water as material
  * Cell Zone Conditions: Activate in operating conditions "Specified operating
    density" and keep default value.
  * Boundary Conditions:
    + Set the top boundary to pressure outflow
    + Set the left boundary to velocity inlet and set for Phase mixture the
      x- and y- velocity components to follow the corresponding UDFs, for
      Phase-2-water set the volume fraction UDF
  * Dynamic Mesh:
    + Set the structure boundary to follow the UDF
    + Set the domain to be deforming/remeshing as needed
  * Reference values: Only needed for writing output (since Fluent usese
    dimensionless values)
  * Solution methods:
    + Set the PISO (pressure implicit) scheme and as volume fraction scheme the
      Modified HRIC scheme

Preparing the preCICE XML configuration:
  * The number of valid digits for the timestep length needs to be 8, since fluent
    has only a single precision timestep length. Set this in the tag <timestep-length ... valid-digits="8"/>
