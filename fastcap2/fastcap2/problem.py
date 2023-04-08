
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

  """Specifies 'x axis' for :py:meth:`ps_upaxis`"""
  XI = 0

  """Specifies 'y axis' for :py:meth:`ps_upaxis`"""
  YI = 1

  """Specifies 'z axis' for :py:meth:`ps_upaxis`"""
  ZI = 2

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
  def perm_factor(self) -> float:
    """The permittivity factor

    All dielectric permittivities will be multiplied by this
    factor. The default value is 1.0.

    This property corresponds to option "-p" of the original
    "fastcap" program.
    """
    return super()._get_perm_factor()

  @perm_factor.setter
  def perm_factor(self, value: float):
    return super()._set_perm_factor(value)

  @property
  def expansion_order(self) -> int:
    """The multipole expansion order

    This property corresponds to option "-o" of the original
    "fastcap" program.

    A value of 0 indicates automatic expansion.
    """
    return super()._get_expansion_order()

  @expansion_order.setter
  def expansion_order(self, value: int):
    return super()._set_expansion_order(value)

  @property
  def partitioning_depth(self) -> int:
    """The partitioning depth

    This property corresponds to option "-d" of the original
    "fastcap" program.

    A negative value indicates automatic depth mode.
    In automatic mode, :py:meth:`solve` will set the depth
    actually used.
    """
    return super()._get_partitioning_depth()

  @partitioning_depth.setter
  def partitioning_depth(self, value: float):
    super()._set_partitioning_depth(value)

  @property
  def iter_tol(self) -> float:
    """The iteration tolerance

    This property corresponds to option "-i" of the original
    "fastcap" program.

    The default value is 0.01;
    """
    return super()._get_iter_tol()

  @iter_tol.setter
  def iter_tol(self, value: float):
    super()._set_iter_tol(value)

  @property
  def skip_conductors(self) -> Optional[list[str]]:
    """Skips the given conductors from the solve list

    This property corresponds to option "-rs" of the original
    "fastcap" program.

    The conductors with the given names will not be present in  
    the capacitance matrix. A value of 'None' for this property
    will select all conductors.
    """
    return super()._get_skip_conductors();

  @skip_conductors.setter
  def skip_conductors(self, value: Optional[list[str]]):
    super()._set_skip_conductors(value);

  @property
  def remove_conductors(self) -> Optional[list[str]]:
    """Removes the given conductors from the input

    This property corresponds to option "-ri" of the original
    "fastcap" program.

    The conductors with the given names will not be considered
    at all. A value of 'None' for this property will enable all
    conductors.
    """
    return super()._get_remove_conductors();

  @remove_conductors.setter
  def remove_conductors(self, value: Optional[list[str]]):
    super()._set_remove_conductors(value);

  @property
  def ps_select_q(self) -> Optional[list[str]]:
    """PS output: select conductors for at-1V charge distribution .ps pictures

    This property corresponds to option "-q" of the original
    "fastcap" program. A value of 'None' for this property will
    select all conductors.
    """
    return super()._get_ps_select_q();

  @ps_select_q.setter
  def ps_select_q(self, value: Optional[list[str]]):
    super()._set_ps_select_q(value);

  @property
  def ps_remove_q(self) -> Optional[list[str]]:
    """PS output: remove conductors from all charge distribution .ps pictures

    This property corresponds to option "-rc" of the original
    "fastcap" program. A value of 'None' for this property will enable
    all conductors in the charge distribution picture.
    """
    return super()._get_ps_remove_q();

  @ps_remove_q.setter
  def ps_remove_q(self, value: Optional[list[str]]):
    super()._set_ps_remove_q(value);

  @property
  def ps_no_key(self) -> bool:
    """PS output: remove key in shaded .ps picture file (use with `select_q` option)

    This property corresponds to option "-rk" of the original
    "fastcap" program.
    """
    return super()._get_ps_no_key();

  @ps_no_key.setter
  def ps_no_key(self, value: bool):
    super()._set_ps_no_key(value);

  @property
  def ps_no_dielectric(self) -> bool:
    """PS output: remove DIELEC type surfaces from all .ps picture files

    This property corresponds to option "-rd" of the original
    "fastcap" program.
    """
    return super()._get_ps_no_dielectric();

  @ps_no_dielectric.setter
  def ps_no_dielectric(self, value: bool):
    super()._set_ps_no_dielectric(value);

  @property
  def ps_total_charges(self) -> bool:
    """PS output: display total charges in shaded .ps picture file (use with `select_q` option)

    This property corresponds to option "-dc" of the original
    "fastcap" program.
    """
    return super()._get_ps_total_charges();

  @ps_total_charges.setter
  def ps_total_charges(self, value: bool):
    super()._set_ps_total_charges(value);

  @property
  def ps_no_showpage(self) -> bool:
    """PS output: suppress showpage in all .ps picture files

    This property corresponds to option "-v" of the original
    "fastcap" program.
    """
    return super()._get_ps_no_showpage();

  @ps_no_showpage.setter
  def ps_no_showpage(self, value: bool):
    super()._set_ps_no_showpage(value);

  @property
  def ps_number_faces(self) -> bool:
    """PS output: number faces with input order numbers

    This property corresponds to option "-n" of the original
    "fastcap" program.
    """
    return super()._get_ps_number_faces();

  @ps_number_faces.setter
  def ps_number_faces(self, value: bool):
    super()._set_ps_number_faces(value);

  @property
  def ps_show_hidden(self) -> bool:
    """PS output: do now remove hidden faces

    This property corresponds to option "-f" of the original
    "fastcap" program.
    """
    return super()._get_ps_show_hidden();

  @ps_show_hidden.setter
  def ps_show_hidden(self, value: bool):
    super()._set_ps_show_hidden(value);

  @property
  def ps_azimuth(self) -> float:
    """PS output: sets the azimuth angle

    This property corresponds to option "-a" of the original
    "fastcap" program.
    """
    return super()._get_ps_azimuth();

  @ps_azimuth.setter
  def ps_azimuth(self, value: float):
    super()._set_ps_azimuth(value);

  @property
  def ps_elevation(self) -> float:
    """PS output: sets the elevation angle

    This property corresponds to option "-e" of the original
    "fastcap" program.
    """
    return super()._get_ps_elevation();

  @ps_elevation.setter
  def ps_elevation(self, value: float):
    super()._set_ps_elevation(value);

  @property
  def ps_rotation(self) -> float:
    """PS output: sets the rotation angle

    This property corresponds to option "-r" of the original
    "fastcap" program.
    """
    return super()._get_ps_rotation();

  @ps_rotation.setter
  def ps_rotation(self, value: float):
    super()._set_ps_rotation(value);

  @property
  def ps_upaxis(self) -> int:
    """PS output: specifies the "up" axis

    This property corresponds to option "-u" of the original
    "fastcap" program.

    Values are:
    * Problem.XI for x axis
    * Problem.YI for y axis
    * Problem.ZI for z axis
    """
    # @@@
    pass

  @ps_upaxis.setter
  def ps_upaxis(self, value: int):
    # @@@
    pass

  @property
  def ps_distance(self) -> float:
    """PS output: sets the distance 

    This property corresponds to option "-h" of the original
    "fastcap" program.
    """
    return super()._get_ps_distance();

  @ps_distance.setter
  def ps_distance(self, value: float):
    super()._set_ps_distance(value);

  @property
  def ps_scale(self) -> float:
    """PS output: sets the scale 

    This property corresponds to option "-s" of the original
    "fastcap" program.
    """
    return super()._get_ps_scale();

  @ps_scale.setter
  def ps_scale(self, value: float):
    super()._set_ps_scale(value);

  @property
  def ps_linewidth(self) -> float:
    """PS output: sets the line width 

    This property corresponds to option "-w" of the original
    "fastcap" program.
    """
    return super()._get_ps_linewidth();

  @ps_linewidth.setter
  def ps_linewidth(self, value: float):
    super()._set_ps_linewidth(value);

  @property
  def ps_axislength(self) -> float:
    """PS output: sets the axis length

    This property corresponds to option "-x" of the original
    "fastcap" program.
    """
    return super()._get_ps_axislength();

  @ps_axislength.setter
  def ps_axislength(self, value: float):
    super()._set_ps_axislength(value);

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

  def add(self, surface: 'fastcap2.Surface', 
                group: Optional[str] = None, 
                dx: float = 0.0,
                dy: float = 0.0,
                dz: float = 0.0,
                flipx: bool = False,
                flipy: bool = False,
                flipz: bool = False,
                rotx: float = 0.0,
                roty: float = 0.0,
                rotz: float = 0.0,
                scale: float = 1.0):
    """Adds a surface object to the problem

    :param surface: The surface to add.
    :param group: If specified, a new conductor group is formed or 
                  the surface is added to the given group.
    :param dx: Translates the surface in x direction.
    :param dy: Translates the surface in y direction.
    :param dz: Translates the surface in z direction.
    :param flipx: Flips the surface and the yz plane.
    :param flipy: Flips the surface and the xz plane.
    :param flipz: Flips the surface and the xy plane.
    :param rotx: Rotates the surface around the x axis by the given angle.
    :param roty: Rotates the surface around the y axis by the given angle.
    :param rotz: Rotates the surface around the z axis by the given angle.
    :param scale: Scales the surface by the given factor.

    A surface can be added multiple times with different
    transformation parameters, so it is easy to form structures
    with multiple conductors.

    The order in which the transformations are applied is:

    * `scale`
    * `flipx`, `flipy` and `flipz`
    * `rotx`
    * `roty`
    * `rotz`
    * `dx`, `dy` and `dz`
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

  def dump_ps(self, filename):
    """Produces a PS file with the geometries and charges

    See the manifold ps_... options that configure PS output.
    """
    # @@@
    pass

