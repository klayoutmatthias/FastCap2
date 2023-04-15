
import unittest
import tempfile
import os

import fastcap2 as fc2

def format_cap_matrix(cap_matrix, unit = 1e-12):
  return "\n".join([ "".join([ "%-6.0f" % (m / unit) for m in row ]) for row in cap_matrix ])


class TestProblem(unittest.TestCase):

  def test_title(self):

    problem = fc2.Problem(title="t1")

    self.assertEqual(problem.title, "t1")

    problem.title = "t2"
    self.assertEqual(problem.title, "t2")

  def test_perm_factor(self):

    problem = fc2.Problem()

    self.assertEqual(problem.perm_factor, 1.0)

    problem.perm_factor = 2.25
    self.assertEqual(problem.perm_factor, 2.25)

  def test_expansion_order(self):

    problem = fc2.Problem()

    self.assertEqual(problem.expansion_order, 2)

    problem.expansion_order = 7
    self.assertEqual(problem.expansion_order, 7)

  def test_partitioning_depth(self):

    problem = fc2.Problem()

    self.assertEqual(problem.partitioning_depth, -1)

    problem.partitioning_depth = 25
    self.assertEqual(problem.partitioning_depth, 25)

  def test_iter_tol(self):

    problem = fc2.Problem()

    self.assertEqual(str(problem.iter_tol), "0.01")

    problem.iter_tol = 0.125
    self.assertEqual(problem.iter_tol, 0.125)

  def test_skip_conductors(self):

    problem = fc2.Problem()

    self.assertEqual(problem.skip_conductors, None)

    problem.skip_conductors = [ "C1", "C2" ]
    self.assertEqual(problem.skip_conductors, [ "C1", "C2" ])

    problem.skip_conductors = [ "CX" ]
    self.assertEqual(problem.skip_conductors, [ "CX" ])

    problem.skip_conductors = []
    self.assertEqual(problem.skip_conductors, [])

    problem.skip_conductors = None
    self.assertEqual(problem.skip_conductors, None)
    
    # conductor names must not be empty strings
    try:
      problem.skip_conductors = [ "" ]
      self.assertEqual(True, False)
    except RuntimeError as ex:
      self.assertEqual(str(ex), "A conductor name must not be an empty string")
      
    # conductor names must not contain comma characters
    try:
      problem.skip_conductors = [ "a,b" ]
      self.assertEqual(True, False)
    except RuntimeError as ex:
      self.assertEqual(str(ex), "',' character is not allowed in this conductor name: 'a,b'")
      
  def test_remove_conductors(self):

    problem = fc2.Problem()

    self.assertEqual(problem.remove_conductors, None)

    problem.remove_conductors = [ "C1", "C2" ]
    self.assertEqual(problem.remove_conductors, [ "C1", "C2" ])

    problem.remove_conductors = None
    self.assertEqual(problem.remove_conductors, None)
    
  def test_qps_select_q(self):

    problem = fc2.Problem()

    self.assertEqual(problem.qps_select_q, None)

    problem.qps_select_q = [ "C1", "C2" ]
    self.assertEqual(problem.qps_select_q, [ "C1", "C2" ])

    problem.qps_select_q = None
    self.assertEqual(problem.qps_select_q, None)
    
  def test_qps_remove_q(self):

    problem = fc2.Problem()

    self.assertEqual(problem.qps_remove_q, None)

    problem.qps_remove_q = [ "C1", "C2" ]
    self.assertEqual(problem.qps_remove_q, [ "C1", "C2" ])

    problem.qps_remove_q = None
    self.assertEqual(problem.qps_remove_q, None)
    
  def test_qps_no_key(self):

    problem = fc2.Problem()

    self.assertEqual(problem.qps_no_key, False)

    problem.qps_no_key = True
    self.assertEqual(problem.qps_no_key, True)

  def test_ps_no_dielectric(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_no_dielectric, False)

    problem.ps_no_dielectric = True
    self.assertEqual(problem.ps_no_dielectric, True)

  def test_qps_total_charges(self):

    problem = fc2.Problem()

    self.assertEqual(problem.qps_total_charges, False)

    problem.qps_total_charges = True
    self.assertEqual(problem.qps_total_charges, True)

  def test_ps_no_showpage(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_no_showpage, False)

    problem.ps_no_showpage = True
    self.assertEqual(problem.ps_no_showpage, True)

  def test_ps_number_faces(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_number_faces, False)

    problem.ps_number_faces = True
    self.assertEqual(problem.ps_number_faces, True)

  def test_ps_show_hidden(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_show_hidden, False)

    problem.ps_show_hidden = True
    self.assertEqual(problem.ps_show_hidden, True)

  def test_ps_azimuth(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_azimuth, 50.0)

    problem.ps_azimuth = 12.5
    self.assertEqual(problem.ps_azimuth, 12.5)

  def test_ps_elevation(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_elevation, 50.0)

    problem.ps_elevation = 1.5
    self.assertEqual(problem.ps_elevation, 1.5)

  def test_ps_rotation(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_rotation, 0.0)

    problem.ps_rotation = 45.0
    self.assertEqual(problem.ps_rotation, 45.0)

  def test_ps_distance(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_distance, 2.0)

    problem.ps_distance = 1.75
    self.assertEqual(problem.ps_distance, 1.75)

  def test_ps_upaxis(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_upaxis, fc2.Problem.ZI)

    problem.ps_upaxis = fc2.Problem.YI
    self.assertEqual(problem.ps_upaxis, fc2.Problem.YI)

    # conductor names must not contain percent characters
    try:
      problem.ps_upaxis = 4
      self.assertEqual(True, False)
    except RuntimeError as ex:
      self.assertEqual(str(ex), "'ps_upaxis' value needs to be between 0 and 2 (but is 4)")

  def test_ps_scale(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_scale, 1.0)

    problem.ps_scale = 0.5
    self.assertEqual(problem.ps_scale, 0.5)

  def test_ps_linewidth(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_linewidth, 1.0)

    problem.ps_linewidth = 2.5
    self.assertEqual(problem.ps_linewidth, 2.5)

  def test_ps_axislength(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_axislength, 1.0)

    problem.ps_axislength = 22.5
    self.assertEqual(problem.ps_axislength, 22.5)

  def test_verbose(self):

    problem = fc2.Problem()

    self.assertEqual(problem.verbose, False)

    problem.verbose = True
    self.assertEqual(problem.verbose, True)

  def test_load_plates(self):

    test_data_path = os.path.join(os.path.dirname(__file__), "data")

    problem = fc2.Problem()

    # cb is a 0.5 thick cap plate 10x10, place at distance 2
    problem.load(os.path.join(test_data_path, "cb.geo"))
    problem.load(os.path.join(test_data_path, "cb.geo"), d = (0, 0, 2.5))

    cap_matrix = problem.solve()

    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "877   -613  \n"
        "-613  877   "
    )

    self.assertEqual(problem.conductors(), ['cb%GROUP1', 'cb%GROUP2'])

  def test_load_plates_with_groups(self):

    test_data_path = os.path.join(os.path.dirname(__file__), "data")

    problem = fc2.Problem()

    # cb is a 0.5 thick cap plate 10x10, place at distance 2
    problem.load(os.path.join(test_data_path, "cb.geo"), group = "G1")
    problem.load(os.path.join(test_data_path, "cb.geo"), group = "G2", d = (0, 0, 2.5))

    cap_matrix = problem.solve()

    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "877   -613  \n"
        "-613  877   "
    )

    self.assertEqual(problem.conductors(), ['cb%G1', 'cb%G2'])

  def test_load_plates_same_groups(self):

    test_data_path = os.path.join(os.path.dirname(__file__), "data")

    problem = fc2.Problem()

    # cb is a 0.5 thick cap plate 10x10, place at distance 2
    problem.load(os.path.join(test_data_path, "cb.geo"), group = "G")
    problem.load(os.path.join(test_data_path, "cb.geo"), group = "G", d = (0, 0, 2.5))

    cap_matrix = problem.solve()

    # a single conductor 
    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "529   "
    )

    self.assertEqual(problem.conductors(), ['cb%G'])

  def test_load_plates_linked(self):

    test_data_path = os.path.join(os.path.dirname(__file__), "data")

    problem = fc2.Problem()

    # cb is a 0.5 thick cap plate 10x10, place at distance 2
    problem.load(os.path.join(test_data_path, "cb.geo"))
    problem.load(os.path.join(test_data_path, "cb.geo"), d = (0, 0, 2.5), link = True)

    cap_matrix = problem.solve()

    # a single conductor 
    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "529   "
    )

    self.assertEqual(problem.conductors(), ["cb%GROUP1"])

  def test_load_list_file(self):

    test_data_path = os.path.join(os.path.dirname(__file__), "data")

    with open(os.path.join(test_data_path, "all.lst"), "r") as f:
      data = f.read()
      
    data = data.replace("%", os.path.join(test_data_path, ""))

    tmp = tempfile.NamedTemporaryFile()
    tmp.write(str.encode(data))
    tmp.flush()

    problem = fc2.Problem()
    problem.load_list(tmp.name)

    cap_matrix = problem.solve()

    # a single conductor 
    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "1565  -1203 -259  \n"
        "-1203 1565  -259  \n"
        "-259  -259  817   "
    )

    self.assertEqual(problem.conductors(), ['ct1%GROUP1', 'ct2%GROUP2', 'cb%GROUP3'])
    
  def test_add_surface(self):

    problem = fc2.Problem()

    surface = fc2.Surface(name = "C")
    surface.add_meshed_quad((0, 0, 0), (0, 10, 0), (10, 0, 0), edge_width = 0.2, num = 10)

    problem.add(surface, d = (0, 0, -1.0))
    problem.add(surface, d = (0, 0, 1.0))

    cap_matrix = problem.solve()

    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "800   -559  \n"
        "-559  800   "
    )

    self.assertEqual(problem.conductors(), ["C%GROUP1", "C%GROUP2"])

  def test_extent(self):

    problem = fc2.Problem()

    surface = fc2.Surface(name = "C")
    surface.add_tri((1, 2, 3), (11, 2, 3), (1, 12, 3))

    problem.add(surface)
    self.assertEqual(problem.extent(), [[1.0, 2.0, 3.0], [11.0, 12.0, 3.0]])

  def test_add_surface_flipx(self):

    problem = fc2.Problem()

    surface = fc2.Surface(name = "C")
    surface.add_tri((1, 2, 3), (11, 2, 3), (1, 12, 3))

    problem.add(surface, flipx = True)
    self.assertEqual(problem.extent(), [[-11.0, 2.0, 3.0], [-1.0, 12.0, 3.0]])

  def test_add_surface_flipy(self):

    problem = fc2.Problem()

    surface = fc2.Surface(name = "C")
    surface.add_tri((1, 2, 3), (11, 2, 3), (1, 12, 3))

    problem.add(surface, flipy = True)
    self.assertEqual(problem.extent(), [[1.0, -12.0, 3.0], [11.0, -2.0, 3.0]])

  def test_add_surface_flipz(self):

    problem = fc2.Problem()

    surface = fc2.Surface(name = "C")
    surface.add_tri((1, 2, 3), (11, 2, 3), (1, 12, 3))

    problem.add(surface, flipz = True)
    self.assertEqual(problem.extent(), [[1.0, 2.0, -3.0], [11.0, 12.0, -3.0]])

  def test_add_surface_scale(self):

    problem = fc2.Problem()

    surface = fc2.Surface(name = "C")
    surface.add_tri((1, 2, 3), (11, 2, 3), (1, 12, 3))

    problem.add(surface, scale = 2.0)
    self.assertEqual(problem.extent(), [[2.0, 4.0, 6.0], [22.0, 24.0, 6.0]])

  def test_add_surface_scalex(self):

    problem = fc2.Problem()

    surface = fc2.Surface(name = "C")
    surface.add_tri((1, 2, 3), (11, 2, 3), (1, 12, 3))

    problem.add(surface, scalex = 2.0)
    self.assertEqual(problem.extent(), [[2.0, 2.0, 3.0], [22.0, 12.0, 3.0]])

  def test_add_surface_scaley(self):

    problem = fc2.Problem()

    surface = fc2.Surface(name = "C")
    surface.add_tri((1, 2, 3), (11, 2, 3), (1, 12, 3))

    problem.add(surface, scaley = 2.0)
    self.assertEqual(problem.extent(), [[1.0, 4.0, 3.0], [11.0, 24.0, 3.0]])

  def test_add_surface_scalez(self):

    problem = fc2.Problem()

    surface = fc2.Surface(name = "C")
    surface.add_tri((1, 2, 3), (11, 2, 3), (1, 12, 3))

    problem.add(surface, scalez = 2.0)
    self.assertEqual(problem.extent(), [[1.0, 2.0, 6.0], [11.0, 12.0, 6.0]])

  def test_add_surface_rotx(self):

    problem = fc2.Problem()

    surface = fc2.Surface(name = "C")
    surface.add_tri((1, 2, 3), (11, 2, 3), (1, 12, 3))

    problem.add(surface, rotx = 90.0)
    self.assertEqual(problem.extent(), [[1.0, -3.0, 2.0], [11.0, -3.0, 12.0]])

  def test_add_surface_roty(self):

    problem = fc2.Problem()

    surface = fc2.Surface(name = "C")
    surface.add_tri((1, 2, 3), (11, 2, 3), (1, 12, 3))

    problem.add(surface, roty = 90.0)
    self.assertEqual(problem.extent(), [[-3.0, 2.0, 1.0], [-3.0, 12.0, 11.0]])

  def test_add_surface_rotz(self):

    problem = fc2.Problem()

    surface = fc2.Surface(name = "C")
    surface.add_tri((1, 2, 3), (11, 2, 3), (1, 12, 3))

    problem.add(surface, rotz = 90.0)
    self.assertEqual(problem.extent(), [[-12.0, 1.0, 3.0], [-2.0, 11.0, 3.0]])

  def test_add_surface_triple(self):

    problem = fc2.Problem()

    b = fc2.Surface(name = "B")
    b.add_meshed_quad((0, 0, 0), (0, 10, 0), (10, 0, 0), edge_width = 0.2, num = 10)

    t1 = fc2.Surface(name = "T1")
    t1.add_meshed_quad((0, 0, 0), (0, 10, 0), (4, 0, 0), edge_width = 0.2, num = 5)

    t2 = fc2.Surface(name = "T2")
    t2.add_meshed_quad((0, 0, 0), (0, 10, 0), (1, 0, 0), edge_width = 0.2, num = 1)

    problem.add(b, d = (0, 0, -1))
    problem.add(t1, d = (0, 0, 1), group = "TOP")
    problem.add(t1, d = (6, 0, 1), group = "TOP")
    problem.add(t2, d = (4.5, 0, 1))

    cap_matrix = problem.solve()

    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "796   -481  -73   \n"
        "-481  891   -194  \n"
        "-73   -194  290   "
    )

    self.assertEqual(problem.conductors(), ['B%GROUP1', 'T1%TOP', 'T2%GROUP2'])

  def test_remove_conductors_from_input(self):

    problem = fc2.Problem()

    b = fc2.Surface(name = "B")
    b.add_meshed_quad((0, 0, 0), (0, 10, 0), (10, 0, 0), edge_width = 0.2, num = 10)

    t1 = fc2.Surface(name = "T1")
    t1.add_meshed_quad((0, 0, 0), (0, 10, 0), (4, 0, 0), edge_width = 0.2, num = 5)

    t2 = fc2.Surface(name = "T2")
    t2.add_meshed_quad((0, 0, 0), (0, 10, 0), (1, 0, 0), edge_width = 0.2, num = 1)

    problem.add(b, d = (0, 0, -1))
    problem.add(t1, d = (0, 0, 1), group = "TOP")
    problem.add(t1, d = (6, 0, 1), group = "TOP")
    problem.add(t2, d = (4.5, 0, 1))

    self.assertEqual(problem.conductors(), ['B%GROUP1', 'T1%TOP', 'T2%GROUP2'])

    problem.remove_conductors = ["T1%TOP"]

    cap_matrix = problem.solve()

    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "528   -165  \n"
        "-165  221   "
    )

    self.assertEqual(problem.conductors(), ['B%GROUP1', 'T2%GROUP2'])

    problem.remove_conductors = ["B%GROUP1", "T2%GROUP2"]

    cap_matrix = problem.solve()

    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "397   "
    )

    self.assertEqual(problem.conductors(), [ 'T1%TOP'])

    # back to normal now

    problem.remove_conductors = None

    cap_matrix = problem.solve()

    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "796   -481  -73   \n"
        "-481  891   -194  \n"
        "-73   -194  290   "
    )

    self.assertEqual(problem.conductors(), ['B%GROUP1', 'T1%TOP', 'T2%GROUP2'])

  def test_incremental(self):

    problem = fc2.Problem()

    b = fc2.Surface(name = "B")
    b.add_meshed_quad((0, 0, 0), (0, 10, 0), (10, 0, 0), edge_width = 0.2, num = 10)

    t1 = fc2.Surface(name = "T1")
    t1.add_meshed_quad((0, 0, 0), (0, 10, 0), (4, 0, 0), edge_width = 0.2, num = 5)

    t2 = fc2.Surface(name = "T2")
    t2.add_meshed_quad((0, 0, 0), (0, 10, 0), (1, 0, 0), edge_width = 0.2, num = 1)

    # add first part

    problem.add(b, d = (0, 0, -1))

    self.assertEqual(problem.conductors(), ['B%GROUP1'])

    cap_matrix = problem.solve()

    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "405   "
    )

    # add second part

    problem.add(t1, d = (0, 0, 1), group = "TOP")
    problem.add(t1, d = (6, 0, 1), group = "TOP")

    self.assertEqual(problem.conductors(), ['B%GROUP1', 'T1%TOP'])

    cap_matrix = problem.solve()

    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "778   -530  \n"
        "-530  761   "
    )

    # add third part

    problem.add(t2, d = (4.5, 0, 1))

    cap_matrix = problem.solve()

    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "796   -481  -73   \n"
        "-481  891   -194  \n"
        "-73   -194  290   "
    )

    self.assertEqual(problem.conductors(), ['B%GROUP1', 'T1%TOP', 'T2%GROUP2'])

  def test_skip_conductors(self):

    problem = fc2.Problem()

    b = fc2.Surface(name = "B")
    b.add_meshed_quad((0, 0, 0), (0, 10, 0), (10, 0, 0), edge_width = 0.2, num = 10)

    t1 = fc2.Surface(name = "T1")
    t1.add_meshed_quad((0, 0, 0), (0, 10, 0), (4, 0, 0), edge_width = 0.2, num = 5)

    t2 = fc2.Surface(name = "T2")
    t2.add_meshed_quad((0, 0, 0), (0, 10, 0), (1, 0, 0), edge_width = 0.2, num = 1)

    problem.add(b, d = (0, 0, -1))
    problem.add(t1, d = (0, 0, 1), group = "TOP")
    problem.add(t1, d = (6, 0, 1), group = "TOP")
    problem.add(t2, d = (4.5, 0, 1))

    problem.skip_conductors = ["T1%TOP"]

    cap_matrix = problem.solve()

    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "796   -481  -73   \n"
        "-481  0     -194  \n"
        "-73   -194  290   "
    )

    self.assertEqual(problem.conductors(), ['B%GROUP1', 'T1%TOP', 'T2%GROUP2'])

    # NOTE: it's sufficient for the leading part to match
    problem.skip_conductors = ["B%GR", "T2%GROUP2"]

    cap_matrix = problem.solve()

    self.assertEqual(format_cap_matrix(cap_matrix, unit = 1e-12),
        "0     -481  0     \n"
        "-481  891   -194  \n"
        "0     -194  0     "
    )

    self.assertEqual(problem.conductors(), ['B%GROUP1', 'T1%TOP', 'T2%GROUP2'])


if __name__ == '__main__':
    unittest.main()

