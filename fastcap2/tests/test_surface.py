
import unittest
import fastcap2 as fc2

class TestSurface(unittest.TestCase):

  def test_title(self):

    surface = fc2.Surface(title="t1")

    self.assertEqual(surface.title, "t1")

    surface.title = "t2"
    self.assertEqual(surface.title, "t2")

    surface.title = None
    self.assertEqual(surface.title, None)

  def test_name(self):

    surface = fc2.Surface(name="n1")

    self.assertEqual(surface.name, "n1")

    surface.name = "n2"
    self.assertEqual(surface.name, "n2")

    surface.name = None
    self.assertEqual(surface.name, None)

  def test_add_quad(self):
    
    surface = fc2.Surface(name="n1")

    self.assertEqual(surface._quad_count(), 0)
    self.assertEqual(surface._quad_area(), 0)
    self.assertEqual(surface._to_string(), "")

    # clockwise -> positive area
    surface.add_quad((0, 0, 0), (0, 1, 0), (1, 1, 0), (1, 0, 0))

    self.assertEqual(surface._quad_count(), 1)
    self.assertEqual(surface._quad_area(), 1.0)
    self.assertEqual(surface._to_string(), "Q (0,0,0) (0,1,0) (1,1,0) (1,0,0)\n")

    # clockwise -> positive area
    surface.add_quad((0, 0, 1), (0, 1, 1), (1, 1, 1), (1, 0, 1))
    self.assertEqual(surface._quad_count(), 2)
    self.assertEqual(surface._quad_area(), 2.0)
    self.assertEqual(surface._to_string(), "Q (0,0,1) (0,1,1) (1,1,1) (1,0,1)\nQ (0,0,0) (0,1,0) (1,1,0) (1,0,0)\n")

  def test_add_tri(self):
    
    surface = fc2.Surface(name="n1")

    self.assertEqual(surface._tri_count(), 0)
    self.assertEqual(surface._tri_area(), 0)
    self.assertEqual(surface._to_string(), "")

    # clockwise -> positive area
    surface.add_tri((0, 0, 0), (0, 1, 0), (1, 1, 0))

    self.assertEqual(surface._tri_count(), 1)
    self.assertEqual(surface._tri_area(), 0.5)
    self.assertEqual(surface._to_string(), "T (0,0,0) (0,1,0) (1,1,0)\n")

    # clockwise -> positive area
    surface.add_tri((0, 0, 1), (0, 1, 1), (1, 1, 1))

    self.assertEqual(surface._tri_count(), 2)
    self.assertEqual(surface._tri_area(), 1.0)
    self.assertEqual(surface._to_string(), "T (0,0,1) (0,1,1) (1,1,1)\nT (0,0,0) (0,1,0) (1,1,0)\n")


if __name__ == '__main__':
    unittest.main()

