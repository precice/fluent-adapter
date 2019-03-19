# preCICE-adapter for the CFD code ANSYS Fluent
*Developed by Bernhard Gatzhammer and valid for FLUENT 19.1 and preCICE v1.4*

## 1. How to build the Fluent-preCICE adapter: 
  * Put the library of preCICE (libprecice.so) into the lnamd64 folder.
  * Adapt lnamd64/2ddp_host/user.udf line 1 "CSOURCES=...": There are several main udf files
      - fsi_udf.c: For FSI simulations. Needs fsi.c.
      - wave_profile_udf.c: For wave simulations with inflow profile. Needs 
        wave_profile.c.
      - wave_maker_udf.c: For wave simulations with moving wall. Needs 
        wave_maker.c.
      - fsi_and_wave_profile_udf.c: For FSI + wave simulations. Needs fsi.c and 
        wave_profile.c.
      The variable SOURCES needs one one main udf file and the correspoding .c files.

  * Adapt lnamd64/2ddp_host/user.udf lin 3 "FLUENT_INC = ..." to the fluent installation
    folder which will be of the type ./ansys_inc/v191/fluent  
  * Adapt the path of the python library in /src/makefile line 19
  * Update the Ansys RELEASE version in /src/makefile line 26
  * Type: make "FLUENT_ARCH=lnamd64" to start the build. Add a "clean" to clean it.

--------------------------------------------------------------------------------

## 2. Installing and Starting Fluent on Ubuntu machine

  ### 2.1 How to install Fluent - **ANSYS V19.1 on Linux Ubuntu 16.04 LTS** 
  * The following installation guide for an earlier Ansys version works for the above configuration:
    <https://www.cfd-online.com/Forums/ansys/199190-ansys-18-2-ubuntu-16-04-installation-guide.html>
  * After the installation you have two packages installed: Fluent and Ansys Workbench:
    Fluent is the CFD-solver and the workbench can be used for creating
    geometries, meshing and creating zones for boundary conditions
  * Fluent is started by typing *fluent* in *ansys_inc/v191/fluent/bin*
  * Workbench is started by typing *runwb2* in
      *ansys_inc/v191/Framework/bin/Linux64*
  * Consider adding the folders into .bashrc to start it without the full
    path from anywhere.
  * Fluent can also be started from the workbench GUI (Choose Fluid-Flow
    (Fluent), double-click on setup opens fluent)

  ### 2.2 How to start Fluent with GUI
  * start the binary "fluent" from your simulation folder
  * set double precision, processing options (serial or parallel) and the dimension
    (for parallel select also "show more", "parallel settings", "mpi types" -> open mpi

  ### 2.3 How to start Fluent without GUI
  * serial:   fluent 2ddp -g < steer-fluent.txt
  * parallel: fluent 2ddp -g -t4 -mpi=openmpi < steer-fluent.txt
    (-t4 sets 4 processes for computations)
    (steer-fluent.txt drives Fluent, can also be done by hand)

--------------------------------------------------------------------------------

## 3. Preparing a Fluent .cas file for UDF function usage:
  * Copy the *lnamd64/* folder from the adapter to the simulation folder into /libudf/
  * Start Fluent and open the .msh or .cas file to be used
  * In Fluent, go to top menu User Defined -> Functions... -> Manage... and 
    load the libudf folder by typing the name "libudf" in the *Library Name* option. 
    The names of the udf functions should appear in the Fluent command line window. 
    (this needs to be done only once for a .cas file)
  * Define a function hook at initialization. Go to top menu User Defined -> Function Hooks ->
    Initialization -> Edit -> select *init::libudf* from Available list -> click Add
    
    **TO BE COMPLETED**
  * Define 1 user defined memory for the faces. (define -> user defined -> memory -> 1; 
    adds one additional double to each face for precice face ids)

--------------------------------------------------------------------------------

## 4. Preparing a Fluent .cas file for FSI simulations

  * Perform the steps in 1., 2. and 3.
  * Set a user defined mesh motion according to function "gridmotions".
    **TO BE COMPLETED** 

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
