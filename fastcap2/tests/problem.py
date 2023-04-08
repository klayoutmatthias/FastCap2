
import unittest
import fastcap2 as fc2

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
      self.assertEqual(str(ex), "'%' or ',' characters are not allowed in this conductor name: 'a,b'")
      
    # conductor names must not contain percent characters
    try:
      problem.skip_conductors = [ "a%b" ]
      self.assertEqual(True, False)
    except RuntimeError as ex:
      self.assertEqual(str(ex), "'%' or ',' characters are not allowed in this conductor name: 'a%b'")
      
  def test_remove_conductors(self):

    problem = fc2.Problem()

    self.assertEqual(problem.remove_conductors, None)

    problem.remove_conductors = [ "C1", "C2" ]
    self.assertEqual(problem.remove_conductors, [ "C1", "C2" ])

    problem.remove_conductors = None
    self.assertEqual(problem.remove_conductors, None)
    
  def test_ps_select_q(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_select_q, None)

    problem.ps_select_q = [ "C1", "C2" ]
    self.assertEqual(problem.ps_select_q, [ "C1", "C2" ])

    problem.ps_select_q = None
    self.assertEqual(problem.ps_select_q, None)
    
  def test_ps_remove_q(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_remove_q, None)

    problem.ps_remove_q = [ "C1", "C2" ]
    self.assertEqual(problem.ps_remove_q, [ "C1", "C2" ])

    problem.ps_remove_q = None
    self.assertEqual(problem.ps_remove_q, None)
    
  def test_ps_no_key(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_no_key, False)

    problem.ps_no_key = True
    self.assertEqual(problem.ps_no_key, True)

  def test_ps_no_dielectric(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_no_dielectric, False)

    problem.ps_no_dielectric = True
    self.assertEqual(problem.ps_no_dielectric, True)

  def test_ps_total_charges(self):

    problem = fc2.Problem()

    self.assertEqual(problem.ps_total_charges, False)

    problem.ps_total_charges = True
    self.assertEqual(problem.ps_total_charges, True)

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


if __name__ == '__main__':
    unittest.main()

