
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


if __name__ == '__main__':
    unittest.main()

