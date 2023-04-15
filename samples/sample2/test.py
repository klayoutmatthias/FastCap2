
"""A demo script for fastcap2 Python module

The geometry is a square two-plate capacitor with a plate
thickness of 0.2 and a plate distance of 2.0.

The gap is partially filled with a dielectic layer of
thickness 1.0 above the bottom plate. The volume above
that layer is air (k = 1).
"""

import fastcap2 as fc2

problem = fc2.Problem()

# enable printing of logs:
# problem.verbose = True

problem.perm_factor = 1.0

# PS picture file or None for no file
ps_file = "cap.ps"

# height of dielectic layer
diel_height     = 1.0 
# dielectric layer k
diel_k          = 1.5
# distance of top plate vs. bottom plate (air + dielectric layer)
top_plate       = 2.0
# width and length of capacitor
width           = 10.0
# capacitor top and bottom plate thickness
cond_height     = 0.2

# mesh resolution
max_dim         = 0.5

# inside reference point for face orientation of sides 
r = (0, 0, 0)  

plate = fc2.Surface("C")
plate.add_meshed_quad((-0.5 * width, -0.5 * width, 0), (-0.5 * width, 0.5 * width, 0), (0.5 * width, -0.5 * width, 0), edge_width = 0.2, max_dim = 1)

cside = fc2.Surface("C")
cside.add_meshed_quad((-0.5 * width, -0.5 * width, 0), (-0.5 * width, 0.5 * width, 0), (-0.5 * width, -0.5 * width, cond_height), max_dim = max_dim)

dside = fc2.Surface("D")
dside.add_meshed_quad((-0.5 * width, -0.5 * width, 0), (-0.5 * width, 0.5 * width, 0), (-0.5 * width, -0.5 * width, diel_height), max_dim = max_dim)

# the bottom conductor - group "B"
# bottom plate
problem.add(plate, group = "B", d = (0, 0, -cond_height))
# sides
for a in range(0, 360, 90):
  problem.add(cside, group = "B", d = (0, 0, -cond_height), rotz = a)

# the interface between bottom conductor and dielectric cap
problem.add(plate, group = "B", d = (0, 0, 0), outside_perm = diel_k)

# the dielectric cap
# top plate
problem.add(plate, d = (0, 0, diel_height), 
                     kind = fc2.Problem.DIELEC, inside_perm = diel_k, outside_perm = 1.0, r = r)
# sides
for a in range(0, 360, 90):
  problem.add(dside, kind = fc2.Problem.DIELEC, inside_perm = diel_k, outside_perm = 1.0, r = r, rotz = a)

# the top conductor - group "T"
# bottom plate
problem.add(plate, group = "T", d = (0, 0, top_plate))
# top plate
problem.add(plate, group = "T", d = (0, 0, top_plate + 0.2))
# sides
for a in range(0, 360, 90):
  problem.add(cside, group = "T", d = (0, 0, top_plate), rotz = a)

# computes the capacitance matrix
cap_matrix = problem.solve()

def format_cap_matrix(cap_matrix, unit = 1e-12):
  return "\n".join([ "  " + "".join([ "%-8.1f" % (m / unit) for m in row ]) for row in cap_matrix ])

print("Capacitance Matrix:\n" + format_cap_matrix(cap_matrix))

if ps_file:
  problem.dump_ps(ps_file)
  print("Picture written to " + ps_file)

