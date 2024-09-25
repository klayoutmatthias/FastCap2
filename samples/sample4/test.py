
"""A demo script for fastcap2 Python module

The geometry is a square two-plate capacitor with a plate
thickness of 0.2 and a plate distance of 2.0.

The gap is partially filled with a dielectic layer of
thickness 1.0 above the bottom plate. The volume above
that layer is air (k = 1).

This version of this example creates an .lst and .geo files
"""

from typing import Optional
from typing import Tuple
import math

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

def write_meshed_quad(filename, group_no, group_name, 
                      p0: Tuple[float, float, float], 
                      p1: Tuple[float, float, float], 
                      p2: Tuple[float, float, float], 
                      max_dim: Optional[float] = None,
                      num: Optional[float] = None,
                      edge_width: Optional[float] = None,
                      edge_fraction: Optional[float] = None,
                      rp: Optional[Tuple[float, float, float]] = None):

  with open(filename, "w") as file:

    file.write("0 a comment\n")

    epsilon = 1e-10

    l1 = math.sqrt(sum([ (p1[i] - p0[i])**2 for i in range(0, 3) ]))
    l2 = math.sqrt(sum([ (p2[i] - p0[i])**2 for i in range(0, 3) ]))

    q1 = [ (p1[i] - p0[i]) / l1 for i in range(0, 3) ]
    q2 = [ (p2[i] - p0[i]) / l2 for i in range(0, 3) ]

    if edge_fraction is not None:
      edge_width = min(l1 * edge_fraction, l2 * edge_fraction)

    l1b = 0.0
    l1e = l1
    l2b = 0.0
    l2e = l2

    with_edge = (edge_width is not None and edge_width > 0.0)

    if with_edge:
      l1b += edge_width
      l1e -= edge_width
      l2b += edge_width
      l2e -= edge_width

    if num is not None:
      max_dim = min((l1e - l1b) / num, (l2e - l2b) / num)

    if max_dim is not None:
      n1 = max(1, int(math.ceil((l1e - l1b) / max_dim - epsilon)))
      n2 = max(1, int(math.ceil((l2e - l2b) / max_dim - epsilon)))
    else:
      n1 = n2 = 1

    d1 = (l1e - l1b) / n1
    d2 = (l2e - l2b) / n2

    s1 = [ l1b + d1 * i for i in range(0, n1 + 1) ]
    s2 = [ l2b + d2 * i for i in range(0, n2 + 1) ]
    if with_edge:
      s1 = [ 0.0 ] + s1 + [ l1 ]
      s2 = [ 0.0 ] + s2 + [ l2 ]

    for i1 in range(0, len(s1) - 1):
      for i2 in range(0, len(s2) - 1):
        p1 = [ p0[i] + q1[i] * s1[i1]     + q2[i] * s2[i2]     for i in range(0, 3) ]
        p2 = [ p0[i] + q1[i] * s1[i1 + 1] + q2[i] * s2[i2]     for i in range(0, 3) ]
        p3 = [ p0[i] + q1[i] * s1[i1 + 1] + q2[i] * s2[i2 + 1] for i in range(0, 3) ]
        p4 = [ p0[i] + q1[i] * s1[i1]     + q2[i] * s2[i2 + 1] for i in range(0, 3) ]
        args = [ p1, p2, p3, p4 ]
        if rp is not None:
          args.append(rp)
        line = "Q " + str(group_no)
        for a in args:
          line += " " + " ".join([ "%.12g" % c for c in a ])
        file.write(line)
        file.write("\n")

    if group_name is not None:
      file.write(f"N {group_no} {group_name}")

write_meshed_quad("bplate.geo", 1, "B", (-0.5 * width, -0.5 * width, 0), (-0.5 * width, 0.5 * width, 0), (0.5 * width, -0.5 * width, 0), edge_width = 0.2, max_dim = 1)
write_meshed_quad("bside1.geo", 1, "B", (-0.5 * width, -0.5 * width, 0), (-0.5 * width, 0.5 * width, 0), (-0.5 * width, -0.5 * width, cond_height), max_dim = max_dim)
write_meshed_quad("bside2.geo", 1, "B", (-0.5 * width, 0.5 * width, 0), (0.5 * width, 0.5 * width, 0), (-0.5 * width, 0.5 * width, cond_height), max_dim = max_dim)
write_meshed_quad("bside3.geo", 1, "B", (0.5 * width, 0.5 * width, 0), (0.5 * width, -0.5 * width, 0), (0.5 * width, 0.5 * width, cond_height), max_dim = max_dim)
write_meshed_quad("bside4.geo", 1, "B", (0.5 * width, -0.5 * width, 0), (-0.5 * width, -0.5 * width, 0), (0.5 * width, -0.5 * width, cond_height), max_dim = max_dim)

write_meshed_quad("dplate.geo", 2, "D", (-0.5 * width, -0.5 * width, 0), (-0.5 * width, 0.5 * width, 0), (0.5 * width, -0.5 * width, 0), edge_width = 0.2, max_dim = 1, rp = (0, 0, -1))
write_meshed_quad("dside1.geo", 2, "D", (-0.5 * width, -0.5 * width, 0), (-0.5 * width, 0.5 * width, 0), (-0.5 * width, -0.5 * width, diel_height), max_dim = max_dim, rp = (0, 0, 0))
write_meshed_quad("dside2.geo", 2, "D", (-0.5 * width, 0.5 * width, 0), (0.5 * width, 0.5 * width, 0), (-0.5 * width, 0.5 * width, diel_height), max_dim = max_dim, rp = (0, 0, 0))
write_meshed_quad("dside3.geo", 2, "D", (0.5 * width, 0.5 * width, 0), (0.5 * width, -0.5 * width, 0), (0.5 * width, 0.5 * width, diel_height), max_dim = max_dim, rp = (0, 0, 0))
write_meshed_quad("dside4.geo", 2, "D", (0.5 * width, -0.5 * width, 0), (-0.5 * width, -0.5 * width, 0), (0.5 * width, -0.5 * width, diel_height), max_dim = max_dim, rp = (0, 0, 0))

write_meshed_quad("tplate.geo", 3, "T", (-0.5 * width, -0.5 * width, 0), (-0.5 * width, 0.5 * width, 0), (0.5 * width, -0.5 * width, 0), edge_width = 0.2, max_dim = 1)
write_meshed_quad("tside1.geo", 3, "T", (-0.5 * width, -0.5 * width, 0), (-0.5 * width, 0.5 * width, 0), (-0.5 * width, -0.5 * width, cond_height), max_dim = max_dim)
write_meshed_quad("tside2.geo", 3, "T", (-0.5 * width, 0.5 * width, 0), (0.5 * width, 0.5 * width, 0), (-0.5 * width, 0.5 * width, cond_height), max_dim = max_dim)
write_meshed_quad("tside3.geo", 3, "T", (0.5 * width, 0.5 * width, 0), (0.5 * width, -0.5 * width, 0), (0.5 * width, 0.5 * width, cond_height), max_dim = max_dim)
write_meshed_quad("tside4.geo", 3, "T", (0.5 * width, -0.5 * width, 0), (-0.5 * width, -0.5 * width, 0), (0.5 * width, -0.5 * width, cond_height), max_dim = max_dim)

with open("sample4.lst", "w") as file:

  file.write(f"C bplate.geo 1 0 0 {-cond_height} +\n")
  file.write(f"C bside1.geo 1 0 0 {-cond_height} +\n")
  file.write(f"C bside2.geo 1 0 0 {-cond_height} +\n")
  file.write(f"C bside3.geo 1 0 0 {-cond_height} +\n")
  file.write(f"C bside4.geo 1 0 0 {-cond_height} +\n")
  file.write(f"C bplate.geo {diel_k} 0 0 0\n")

  file.write(f"D dplate.geo 1 {diel_k} 0 0 {diel_height} 0 0 10 -\n")
  file.write(f"D dside1.geo 1 {diel_k} 0 0 0 0 0 10 -\n")
  file.write(f"D dside2.geo 1 {diel_k} 0 0 0 0 0 10 -\n")
  file.write(f"D dside3.geo 1 {diel_k} 0 0 0 0 0 10 -\n")
  file.write(f"D dside4.geo 1 {diel_k} 0 0 0 0 0 10 -\n")

  file.write(f"C tplate.geo 1 0 0 {top_plate} +\n")
  file.write(f"C tside1.geo 1 0 0 {top_plate} +\n")
  file.write(f"C tside2.geo 1 0 0 {top_plate} +\n")
  file.write(f"C tside3.geo 1 0 0 {top_plate} +\n")
  file.write(f"C tside4.geo 1 0 0 {top_plate} +\n")
  file.write(f"C tplate.geo 1 0 0 {top_plate+0.2}\n")

print(".lst and .geo files created. Run 'fastcap2 -lsample4.lst' to compute the cap matrix.")

