from __future__ import division

import numpy as np
import precice

configuration_file_name = "precice-config.xml"
participant_name = "CSMdummy"
mesh_name = "beam"


def computeDisplacements(force_vals, displ_vals, coords_x):
    print("Entering computeDisplacements")

    # Considering beam of square cross-section with thickness of 0.01 m
    a = 0.01
    # Area moment of inertia about neutral axis of square cross-section
    MI = pow(a, 4) / 12
    # Modulus of elasticity of copper = 117 GPa
    ME = 117 * pow(10, 9)
    # Length of beam = 1 m as stated in Fluent
    ll = 1

    displ_vals[:, 1] = (1 / (24 * ME * MI * ll)) * \
        np.multiply(np.multiply(np.multiply(force_vals[:, 1],
                                            coords_x),
                                (ll - coords_x)),
                    (ll * ll + coords_x * (ll - coords_x)))

    # cap the displacement at 0.002
    displ_vals = np.array([np.array([val[0], -0.002]) if val[1] < -0.002
                           else np.array([val[0], 0.002]) if val[1] > 0.002
                           else np.array([val[0], val[1]])
                           for val in displ_vals])

    print("Updating displacement variable")

    return displ_vals


solver_process_index = 0
solver_process_size = 1

interface = precice.Participant(participant_name, configuration_file_name, solver_process_index, solver_process_size)

dim = interface.get_mesh_dimensions(mesh_name)

vertexSize = 100
coords_x = np.linspace(0, 1, num=vertexSize)
coords_y = np.linspace(0, 0, num=vertexSize)

coords = np.stack([coords_x, coords_y], axis=1)

print("coordinate array to be sent to set_mesh_vertices = {}".format(coords))
print("mesh_name sent to set_mesh_vertices = {}".format(mesh_name))

vertexIDs = interface.set_mesh_vertices(mesh_name, coords)

displ_name = "Displacements"
force_name = "Forces"
displacements = np.zeros([vertexSize, dim])
forces = np.zeros([vertexSize, dim])
solver_dt = 1.0
if interface.requires_initial_data():
    interface.write_data(mesh_name, force_name, vertexIDs, forces)
    interface.write_data(mesh_name, displ_name, vertexIDs, displacements)

interface.initialize()

while interface.is_coupling_ongoing():

    if interface.requires_writing_checkpoint():
        print("CSMdummy: Writing iteration checkpoint")

    precice_dt = interface.get_max_time_step_size()
    dt = min(precice_dt, solver_dt)

    forces = interface.read_data(mesh_name, force_name, vertexIDs, dt)
    print("Forces read in:\n{}".format(forces))

    displacements = computeDisplacements(forces, displacements, coords_x)
    print("Computed Displacements:\n{}".format(displacements))

    interface.write_data(mesh_name, displ_name, vertexIDs, displacements)

    interface.advance(dt)

    if interface.requires_reading_checkpoint():
        print("CSMdummy: Reading iteration checkpoint")
    else:
        print("CSMdummy: advancing in time")

interface.finalize()
