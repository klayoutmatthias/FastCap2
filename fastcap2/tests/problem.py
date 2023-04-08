
import unittest
import fastcap2 as fc2

class TestProblem(unittest.TestCase):

  def test_title(self):

    problem = fc2.Problem(title="t1")

    self.assertEqual(problem.title, "t1")

    problem.title = "t2"
    self.assertEqual(problem.title, "t2")


if __name__ == '__main__':
    unittest.main()

