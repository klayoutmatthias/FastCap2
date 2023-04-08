
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

    self.assertEqual(problem.expansion_order, 0)

    problem.expansion_order = 25
    self.assertEqual(problem.expansion_order, 25)


if __name__ == '__main__':
    unittest.main()

