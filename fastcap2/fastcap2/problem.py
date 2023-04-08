
from typing import Optional

from fastcap2_core import Problem as _Problem

class Problem(_Problem):

  """Describes a FastCap2 project (aka 'problem')

  :param title: The optional title string

  Use this class following these steps:

  * Create and configure the problem object
  * Add surfaces to it by either using :py:meth:`load`, :py:meth:`load_list`
    or adding :py:class:`fastcap2.surface.Surface` objects using :py:meth:`add`.
  * Call :py:meth:`solve` to compute the capacitance matrix

  .. code-block:: python

     import fastcap2 as fc2

     problem = fc2.Problem(title = "A sample problem")

     # loads a problem from a FastCap2 list file
     problem.load_list("geometry.lst")

     cap_matrix = problem.solve()
  
  """

  def __init__(self, title: Optional[str] = None):
    kwargs = {}
    if title is not None:
      kwargs["title"] = title
    super().__init__(**kwargs)

  @property
  def title(self) -> str:
    """The title string

    The title string is an arbitrary optional string that describes 
    the project in the documentation.
    """
    return super()._get_title()

  @title.setter
  def title(self, value: str):
    return super()._set_title(value)

  @property
  def relative_perm(self) -> float:
    """The relative permittivity

    This value specifies the relative permittivity of 
    the dielectrics. The default value is 1.0.

    This property corresponds to option "-p" of the original
    "fastcap" program.
    """
    # @@@
    pass

  @relative_perm.setter
  def relative_perm(self, value: float):
    # @@@
    pass

  @property
  def expansion_order(self) -> int:
    """The multipole expansion order

    This property corresponds to option "-o" of the original
    "fastcap" program.
    """
    # @@@
    pass

  @expansion_order.setter
  def expansion_order(self, value: float):
    # @@@
    pass

  @property
  def partitioning_depth(self) -> int:
    """The partitioning depth

    This property corresponds to option "-d" of the original
    "fastcap" program.
    """
    # @@@
    pass

  @partitioning_depth.setter
  def partitioning_depth(self, value: float):
    # @@@
    pass

  @property
  def iter_tol(self) -> float:
    """The iteration tolerance

    This property corresponds to option "-i" of the original
    "fastcap" program.

    The default value is 0.01;
    """
    # @@@
    pass

  @iter_tol.setter
  def iter_tol(self, value: float):
    # @@@
    pass

  @property
  def skip_conductors(self) -> list[str]:
    """Skips the given conductors from the solve list

    This property corresponds to option "-rs" of the original
    "fastcap" program.

    The conductors with the given names will not be present in  
    the capacitance matrix.
    """
    # @@@
    pass

  @skip_conductors.setter
  def skip_conductors(self, value: list[str]):
    # @@@
    pass

  @property
  def remove_conductors(self) -> list[str]:
    """Removes the given conductors from the input

    This property corresponds to option "-ri" of the original
    "fastcap" program.

    The conductors with the given names will not be considers
    at all.
    """
    # @@@
    pass

  @remove_conductors.setter
  def remove_conductors(self, value: list[str]):
    # @@@
    pass

  def load(self, file: str, group: Optional[str] = None):
    """Loads a single "quickif"-style geometry file

    :param file: The file to load
    :param group: If given, a conductor group will be formed or 
                  the surface will be added to the respective 
                  group
    """
    # @@@
    pass

  def load_list(self, file: str):
    """Loads a "list-style" geometry file

    This method corresponds to option "-l" in the original
    "fastcap" program.
    """
    # @@@
    pass

  def solve(self) -> list[ list[float] ]:
    """Solves the problem and returns the capacitance matrix

    Raises an exception if an error occurs.

    The rows and columns correspond to the conductors 
    returned by the :py:meth:`conductors` method.
    """
    # @@@
    pass

  def conductors(self) -> list[str]:
    """Returns the effective list of conductors present in the capacitance matrix
    """
    # @@@
    pass

"""
:nodoc

# PS output options
problem.ps_select_q = list        # -q
problem.ps_remove_q = list        # -rc
problem.ps_no_key = True          # -rk
problem.ps_no_dielectric = True   # -rd
problem.ps_total_charges = True   # -dc
problem.ps_no_showpage = True     # -v
problem.ps_number_faces = True    # -n
problem.ps_no_hidden = True       # -f
problem.ps_azimuth = 0.0          # -a
problem.ps_elevation = 0.0        # -e
problem.ps_rotation = 0.0         # -r
problem.ps_distance = 0.0         # -h
problem.ps_scale = 1.0            # -s
problem.ps_linewidth = 0.1        # -w
problem.ps_upaxis = 0.1           # -u
problem.ps_axeslength = 10.0      # -x
"""


