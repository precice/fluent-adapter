from __future__ import division

import argparse
import numpy as np
import precice
from mpi4py import MPI

# parser = argparse.ArgumentParser()
# parser.add_argument("configurationFileName", help="Name of the xml config file.", type=str)
# parser.add_argument("participantName", help="Name of the solver.", type=str)
# parser.add_argument("meshName", help="Name of the mesh.", type=str)

configuration_file_name = "precice-config.xml"
participant_name = "CSMdummy"
mesh_name = "beam"


def computeDisplacements(force_vals, displ_vals):
    print("Entering computeDisplacements")

    # Considering beam of square cross-section with thickness of 0.01 m
    a = 0.01
    # Area moment of inertia about neutral axis of square cross-section
    MI = pow(a, 4) / 12
    # Modulus of elasticity of copper = 117 GPa
    ME = 117 * pow(10, 9)
    # Length of beam = 1 m as stated in FLUENT
    ll = 1

    for vertex in range(0, vertexSize*dim, 2):
        displ_vals[vertex] = force_vals[vertex]*vertex*(ll-vertex)*(ll*ll + vertex*(ll-x))/(24*ME*MI*ll)
    print("Updating displacement variable")


solver_process_index = 0
solver_process_size = 1

interface = precice.Interface(participant_name, configuration_file_name, solver_process_index, solver_process_size)

dim = interface.get_dimensions()
mesh_id = interface.get_mesh_id(mesh_name)

vertexSize = 100
coords_x = []
coords_y = []
for i in range(vertexSize):
    coords_x.append(i)
    coords_y.append(0.0)

coords = np.stack([coords_x, coords_y], axis=1)

print("coordinate array to be sent to set_mesh_vertices = {}".format(coords))
print("mesh_id sent to set_mesh_vertices = {}".format(mesh_id))

vertexIDs = interface.set_mesh_vertices(mesh_id, coords)

displIDs = interface.get_data_id("Displacements", mesh_id)
forceIDs = interface.get_data_id("Forces", mesh_id)
displacements = np.zeros([vertexSize, dim])
forces = np.zeros([vertexSize, dim])
dt = 1.0

precice_dt = interface.initialize()

while interface.is_coupling_ongoing():

    if interface.is_action_required(precice.action_write_iteration_checkpoint()):
        print("CSMdummy: Writing iteration checkpoint")
        interface.mark_action_fulfilled(precice.action_write_iteration_checkpoint())

    forces = interface.read_block_vector_data(forceIDs, vertexIDs)

#    computeDisplacements(forces,displacements)

    dt = min(precice_dt, dt)

    interface.write_block_vector_data(displIDs, vertexIDs, displacements)

    precice_dt = interface.advance(dt)

    if interface.is_action_required(precice.action_read_iteration_checkpoint()):
        print("CSMdummy: Reading iteration checkpoint")
        interface.mark_action_fulfilled(precice.action_read_iteration_checkpoint())
    else:
        print("CSMdummy: advancing in time")

interface.finalize()
