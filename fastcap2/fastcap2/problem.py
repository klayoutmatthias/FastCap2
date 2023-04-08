
from fastcap2_core import Problem as _Problem

class Problem(_Problem):

  """Describes a FastCap2 project (aka 'problem')

  :param title: the optional title string
  :type title: str

  Use this class following these steps:

  * Create and configure the problem object
  * Add surfaces to it by either using "load"
    or adding "Surface" objects.
  * Call "solve" to compute the capacitance 
    matrix

  .. codeblock:: python
     import fastcap2 as fc2

     problem = fc2.Problem(title = "A sample problem")

     # loads a problem from a FastCap2 list file
     problem.load_list("geometry.lst")

     cap_matrix = problem.solve()
  
  """

  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)


