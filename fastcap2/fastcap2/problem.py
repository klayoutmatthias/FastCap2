
from typing import Optional
from typing import Tuple

from fastcap2_core import Problem as _Problem

class Problem(_Problem):

  """Describes a FastCap2 project (aka 'problem')

  :param title: The optional title string. It is used as the default title for surfaces.

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

     # solves the problem and returns the cap matrix in MKS units
     cap_matrix = problem.solve()
     print(cap_matrix)

     # dumps the geometry to a PS file
     problem.dump_ps("geo.ps")

  Dumping the geometry to a PS file comes very handy when
  debugging the geometry setup.

  Once :py:meth:`solve` or :py:meth:`dump_ps` is called, 
  the geometry of the problem should not be modified again.
  Currently this will put the object in an undefined state.
  """

  XI = 0
  """Specifies 'x axis' for :py:meth:`ps_upaxis`"""

  YI = 1
  """Specifies 'y axis' for :py:meth:`ps_upaxis`"""

  ZI = 2
  """Specifies 'z axis' for :py:meth:`ps_upaxis`"""

  CONDTR = 0
  """Specifies 'conductor' kind for the surface (see :py:meth:`add`)"""

  DIELEC = 1
  """Specifies 'dielectric interface' kind for the surface (see :py:meth:`add`)"""

  BOTH = 3
  """Specifies 'dielectric interaface with very thin conductor on it' kind for the surface (see :py:meth:`add`)"""

  def __init__(self, title: Optional[str] = None):
    kwargs = {}
    if title is not None:
      kwargs["title"] = title
    super().__init__(**kwargs)

  @property
  def title(self) -> str:
    """The title string

    The title string is an arbitrary optional string. Surfaces will see
    this title unless they specify their own one.
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

    The default value is 0.01.
    """
    return super()._get_iter_tol()

  @iter_tol.setter
  def iter_tol(self, value: float):
    super()._set_iter_tol(value)

  @property
  def skip_conductors(self) -> Optional[list[str]]:
    """Skips the given conductors from the solve list

    The argument is a list of group-qualified conductor names
    in the form `name%group`, e.g. `PLATE%GROUP1`.
    Conductors will be identified by matching the leading
    part - e.g. `PLATE%GROUP` will match conductors `PLATE%GROUP1`,
    `PLATE%GROUP2` etc.

    This property corresponds to option "-rs" of the original
    "fastcap" program.

    A value of 'None' for this property will select all conductors.

    The effect of this option is to skip the specified 
    conductors from the evaluation. Skipping conductors
    can speed up the computation as self-capacitances of
    these conductors and capacitances between skipped 
    conductors are not considered.
    """
    return super()._get_skip_conductors()

  @skip_conductors.setter
  def skip_conductors(self, value: Optional[list[str]]):
    super()._set_skip_conductors(value)

  @property
  def remove_conductors(self) -> Optional[list[str]]:
    """Removes the given conductors from the input

    The argument is a list of group-qualified conductor names
    in the form `name%group`, e.g. `PLATE%GROUP1`.
    Conductors will be identified by matching the leading
    part - e.g. `PLATE%GROUP` will match conductors `PLATE%GROUP1`,
    `PLATE%GROUP2` etc.

    This property corresponds to option "-ri" of the original
    "fastcap" program.

    The conductors with the given names will not be considered
    at all and will not be present in the capacitance matrix. 
    A value of 'None' for this property will enable all conductors.
    """
    return super()._get_remove_conductors()

  @remove_conductors.setter
  def remove_conductors(self, value: Optional[list[str]]):
    super()._set_remove_conductors(value)

  @property
  def qps_file_base(self) -> Optional[str]:
    """PS output: select file base for at-1V charge distribution .ps pictures

    If this property is set, charge distribution .ps pictures will be generated
    during the :py:meth:`solve` call. For each conductor a different .ps file 
    will be written. The .ps files are named using the value of this property
    and appending the conductor number and a `.ps` suffix.

    For example, setting this property to `/tmp/charges` will generate .ps files
    called `/tmp/charges1.ps`, `/tmp/charges2.ps` and so on.

    Setting this property to None (the default) will disable generating the 
    charge distribution pictures.

    The charge distribution output can be configured further using the
    :py:meth:`qps_select_q`, :py:meth:`qps_remove_q`, :py:meth:`qps_no_key` and
    :py:meth:`qps_total_charges` properties.

    Note, that charge distribution pictures are only generated during
    :py:meth:`solve`, but not by :py:meth:`dump_ps`.
    """
    return super()._get_qps_file_base()

  @qps_file_base.setter
  def qps_file_base(self, value: Optional[str]):
    super()._set_qps_file_base(value)

  @property
  def qps_select_q(self) -> Optional[list[str]]:
    """PS output: select conductors for charge distribution .ps pictures

    The argument is a list of group-qualified conductor names
    in the form `name%group`, e.g. `PLATE%GROUP1`.
    Conductors will be identified by matching the leading
    part - e.g. `PLATE%GROUP` will match conductors `PLATE%GROUP1`,
    `PLATE%GROUP2` etc.

    This property corresponds to option "-q" of the original
    "fastcap" program. A value of 'None' for this property will
    select all conductors.

    This option is effective only if charge distribution pictures are
    enabled by setting the :py:meth:`qps_file_base` property.
    """
    return super()._get_qps_select_q()

  @qps_select_q.setter
  def qps_select_q(self, value: Optional[list[str]]):
    super()._set_qps_select_q(value)

  @property
  def qps_remove_q(self) -> Optional[list[str]]:
    """PS output: remove conductors from all charge distribution .ps pictures

    The argument is a list of group-qualified conductor names
    in the form `name%group`, e.g. `PLATE%GROUP1`.
    Conductors will be identified by matching the leading
    part - e.g. `PLATE%GROUP` will match conductors `PLATE%GROUP1`,
    `PLATE%GROUP2` etc.

    This property corresponds to option "-rc" of the original
    "fastcap" program. A value of 'None' for this property will enable
    all conductors in the charge distribution picture.

    This option is effective only if charge distribution pictures are
    enabled by setting the :py:meth:`qps_file_base` property.
    """
    return super()._get_qps_remove_q()

  @qps_remove_q.setter
  def qps_remove_q(self, value: Optional[list[str]]):
    super()._set_qps_remove_q(value)

  @property
  def qps_no_key(self) -> bool:
    """PS output: remove key from all charge distribution .ps pictures

    This property corresponds to option "-rk" of the original
    "fastcap" program.

    This option is effective only if charge distribution pictures are
    enabled by setting the :py:meth:`qps_file_base` property.
    """
    return super()._get_qps_no_key()

  @qps_no_key.setter
  def qps_no_key(self, value: bool):
    super()._set_qps_no_key(value)

  @property
  def qps_total_charges(self) -> bool:
    """PS output: display total charges in charge distribution .ps pictures

    This property corresponds to option "-dc" of the original
    "fastcap" program.

    This option is effective only if charge distribution pictures are
    enabled by setting the :py:meth:`qps_file_base` property.
    """
    return super()._get_qps_total_charges()

  @qps_total_charges.setter
  def qps_total_charges(self, value: bool):
    super()._set_qps_total_charges(value)

  @property
  def ps_no_dielectric(self) -> bool:
    """PS output: remove DIELEC type surfaces from all .ps picture files

    This property corresponds to option "-rd" of the original
    "fastcap" program.
    """
    return super()._get_ps_no_dielectric()

  @ps_no_dielectric.setter
  def ps_no_dielectric(self, value: bool):
    super()._set_ps_no_dielectric(value)

  @property
  def ps_no_showpage(self) -> bool:
    """PS output: suppress showpage in all .ps picture files

    This property corresponds to option "-v" of the original
    "fastcap" program.
    """
    return super()._get_ps_no_showpage()

  @ps_no_showpage.setter
  def ps_no_showpage(self, value: bool):
    super()._set_ps_no_showpage(value)

  @property
  def ps_number_faces(self) -> bool:
    """PS output: number faces with input order numbers

    This property corresponds to option "-n" of the original
    "fastcap" program.
    """
    return super()._get_ps_number_faces()

  @ps_number_faces.setter
  def ps_number_faces(self, value: bool):
    super()._set_ps_number_faces(value)

  @property
  def ps_show_hidden(self) -> bool:
    """PS output: do not remove hidden faces

    This property corresponds to option "-f" of the original
    "fastcap" program.
    """
    return super()._get_ps_show_hidden()

  @ps_show_hidden.setter
  def ps_show_hidden(self, value: bool):
    super()._set_ps_show_hidden(value)

  @property
  def ps_azimuth(self) -> float:
    """PS output: the azimuth angle

    This property corresponds to option "-a" of the original
    "fastcap" program.
    """
    return super()._get_ps_azimuth()

  @ps_azimuth.setter
  def ps_azimuth(self, value: float):
    super()._set_ps_azimuth(value)

  @property
  def ps_elevation(self) -> float:
    """PS output: the elevation angle

    This property corresponds to option "-e" of the original
    "fastcap" program.
    """
    return super()._get_ps_elevation()

  @ps_elevation.setter
  def ps_elevation(self, value: float):
    super()._set_ps_elevation(value)

  @property
  def ps_rotation(self) -> float:
    """PS output: the rotation angle

    This property corresponds to option "-r" of the original
    "fastcap" program.
    """
    return super()._get_ps_rotation()

  @ps_rotation.setter
  def ps_rotation(self, value: float):
    super()._set_ps_rotation(value)

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
    return super()._get_ps_upaxis()

  @ps_upaxis.setter
  def ps_upaxis(self, value: int):
    super()._set_ps_upaxis(value)

  @property
  def ps_distance(self) -> float:
    """PS output: the distance 

    This property corresponds to option "-h" of the original
    "fastcap" program.
    """
    return super()._get_ps_distance()

  @ps_distance.setter
  def ps_distance(self, value: float):
    super()._set_ps_distance(value)

  @property
  def ps_scale(self) -> float:
    """PS output: the scale 

    This property corresponds to option "-s" of the original
    "fastcap" program.
    """
    return super()._get_ps_scale()

  @ps_scale.setter
  def ps_scale(self, value: float):
    super()._set_ps_scale(value)

  @property
  def ps_linewidth(self) -> float:
    """PS output: the line width 

    This property corresponds to option "-w" of the original
    "fastcap" program.
    """
    return super()._get_ps_linewidth()

  @ps_linewidth.setter
  def ps_linewidth(self, value: float):
    super()._set_ps_linewidth(value)

  @property
  def ps_axislength(self) -> float:
    """PS output: the axis length

    This property corresponds to option "-x" of the original
    "fastcap" program.
    """
    return super()._get_ps_axislength()

  @ps_axislength.setter
  def ps_axislength(self, value: float):
    super()._set_ps_axislength(value)

  @property
  def verbose(self) -> bool:
    """If true, the solve methods will print information about the problem and the solution
    """
    return super()._get_verbose()

  @verbose.setter
  def verbose(self, value: bool):
    super()._set_verbose(value)

  def load(self, file: str, 
           link: bool = False, 
           group: Optional[str] = None,
           kind: int = CONDTR,
           ref_point_inside: bool = True,
           outside_perm: float = 1.0,
           inside_perm: float = 1.0,
           d: Tuple[float, float, float] = (0.0, 0.0, 0.0),
           r: Tuple[float, float, float] = (0.0, 0.0, 0.0),
           flipx: bool = False,
           flipy: bool = False,
           flipz: bool = False,
           rotx: float = 0.0,
           roty: float = 0.0,
           rotz: float = 0.0,
           scale: float = 1.0,
           scalex: float = 1.0,
           scaley: float = 1.0,
           scalez: float = 1.0):
    """Loads a single "quickif"-style geometry file

    :param file: The file to load
    :param link: If True, links this surface to the previous 
                 conductor.
    :param group: The name of the conductor group to form.
    :param kind: The type of the surface (conductor or dielectric or dielectric interface).
    :param outside_perm: The permittivity outside of the surface.
    :param inside_perm: The permittivity inside of the surface (only for dielectric surfaces).
    :param d: Translates the surface.
    :param r: The reference point for surface normalisation. Needed for dielectric surfaces.
    :param ref_point_inside: True, if the reference point is inside the surface. Needed for dielectric surfaces.
    :param flipx: Flips the surface at the yz plane.
    :param flipy: Flips the surface at the xz plane.
    :param flipz: Flips the surface at the xy plane.
    :param rotx: Rotates the surface around the x axis by the given angle.
    :param roty: Rotates the surface around the y axis by the given angle.
    :param rotz: Rotates the surface around the z axis by the given angle.
    :param scale: Scales the surface by the given factor.
    :param scalex: Scales the surface by the given factor in x direction.
    :param scaley: Scales the surface by the given factor in y direction.
    :param scalez: Scales the surface by the given factor in z direction.

    Note that the files are not loaded immediately, but 
    upon :py:meth:`solve`.
    
    By design of the file format, only conductor surfaces can be
    loaded by this method.

    The `group` name together with the conductor name inside the
    file will form an effective conductor name of the form 
    `name%group`. All surfaces with the same effective conductor
    name become connected. So it is possible to form a connected
    surface from multiple files or multiple calls of the same file 
    by giving them the same group name.

    `link` is a similar mechanism than `group`, but will
    automatically use the group of the surface loaded previously
    if set to True.

    `link` and `group` are mutually exclusive.

    `kind` specifies which type of surface is given. The surface
    can be:

    * a conductive surface (value :py:meth:`Problem.CONDTR`)
    * a dielectric surface separating two different dielectrics (value :py:meth:`Problem.DIELEC`) 
    * or a thin conducting surface separating two dielectrics (value :py:meth:`Problem.BOTH`)

    `outside_perm` specifies the permittivity outside of the 
    conductor or dielectric surface.
    `inside_perm` specifies the permittivity inside the surface for 
    `DIELEC` or `BOTH` type.

    In order to tell inside from outside halfspace and to properly orient the faces, 
    a reference point needs to be given that does not lie on the surface.
    This is mandatory for types `DIELEC` and `BOTH`. For `CONDTR` type, this 
    specification is not required - "outside" refers to both sides of the surface.
    Specify the reference point with the `r` parameter which is a triple of x, y and z values.

    Alternatively, the reference point can be specified individually per triangle
    or quad.

    By default, the reference point is taken to be inside the surface. 
    By using `ref_point_inside = False`, the reference point is taken to be outside
    the surface.

    Additional transformations can be applied to the surface before using it:

    * A displacement (translation) - `d` parameter
    * Rotation around different axes - `rotx`, `roty` and `rotz` parameters
    * Mirroring in x, y or z direction - `flipx`, `flipy` and `flipz` parameters
    * Scaling (isotropic or single direction) - `scale`, `scalex`, `scaley` and `scalez` parameters

    The order in which the transformations are applied is:

    * Scaling by `scale`, `scalex`, `scaley` and `scalez`
    * Flipping by `flipx`, `flipy` and `flipz`
    * Rotation by `rotx`
    * Rotation by `roty`
    * Rotation by `rotz`
    * Translation by `d`
    """

    if type(d) is list:
      d = tuple(d)
    if type(r) is list:
      r = tuple(r)

    return super()._load(file, link, group, kind, ref_point_inside, 
                         outside_perm, inside_perm, d, r, flipx, flipy, flipz, 
                         rotx, roty, rotz, scale * scalex, scale * scaley, scale * scalez)

  def load_list(self, file: str):
    """Loads a "list-style" geometry file

    This method corresponds to option "-l" in the original
    "fastcap" program.
    """
    return super()._load_list(file)

  def add(self, surface: 'fastcap2.Surface', 
                link: bool = False,
                group: Optional[str] = None, 
                kind: int = CONDTR,
                ref_point_inside: bool = True,
                outside_perm: float = 1.0,
                inside_perm: float = 1.0,
                d: Tuple[float, float, float] = (0.0, 0.0, 0.0),
                r: Tuple[float, float, float] = (0.0, 0.0, 0.0),
                flipx: bool = False,
                flipy: bool = False,
                flipz: bool = False,
                rotx: float = 0.0,
                roty: float = 0.0,
                rotz: float = 0.0,
                scale: float = 1.0,
                scalex: float = 1.0,
                scaley: float = 1.0,
                scalez: float = 1.0):
    """Adds a surface object to the problem

    :param surface: The surface to add.
    :param group: See :py:meth:`load` for details.
    :param link: See :py:meth:`load` for details.
    :param kind: The type of the surface (conductor or dielectric or dielectric interface). See :py:meth:`load` for details.
    :param outside_perm: The permittivity outside of the surface. See :py:meth:`load` for details.
    :param inside_perm: The permittivity inside of the surface (only for dielectric surfaces). See :py:meth:`load` for details.
    :param d: Translates the surface. See :py:meth:`load` for details.
    :param r: The reference point for surface normalisation. Needed for dielectric surfaces. See :py:meth:`load` for details.
    :param ref_point_inside: True, if the reference point is inside the surface. Needed for dielectric surfaces. See :py:meth:`load` for details.
    :param flipx: Flips the surface at the yz plane. See :py:meth:`load` for details.
    :param flipy: Flips the surface at the xz plane. See :py:meth:`load` for details.
    :param flipz: Flips the surface at the xy plane. See :py:meth:`load` for details.
    :param rotx: Rotates the surface around the x axis by the given angle. See :py:meth:`load` for details.
    :param roty: Rotates the surface around the y axis by the given angle. See :py:meth:`load` for details.
    :param rotz: Rotates the surface around the z axis by the given angle. See :py:meth:`load` for details.
    :param scale: Scales the surface by the given factor. See :py:meth:`load` for details.
    :param scalex: Scales the surface by the given factor in x direction. See :py:meth:`load` for details.
    :param scaley: Scales the surface by the given factor in y direction. See :py:meth:`load` for details.
    :param scalez: Scales the surface by the given factor in z direction. See :py:meth:`load` for details.

    A surface can be added multiple times with different
    transformation parameters, so it is easy to form structures
    with multiple conductors.

    The other parameters are explained in the description
    of the :py:meth:`load` function.
    """

    if type(d) is list:
      d = tuple(d)
    if type(r) is list:
      r = tuple(r)

    return super()._add(surface, link, group, kind, ref_point_inside, 
                        outside_perm, inside_perm, d, r, flipx, flipy, flipz, 
                        rotx, roty, rotz, scale * scalex, scale * scaley, scale * scalez)

  def extent(slef) -> list[ list[float], list[float] ]:
    """Gets the extent of all geometries of all surfaces (bounding box)

    The first tuple is the minimum x, y and z,
    the second one the maximum x, y and z.
    """
    return super()._extent()

  def solve(self) -> list[ list[float] ]:
    """Solves the problem and returns the capacitance matrix

    Raises an exception if an error occurs.

    The rows and columns correspond to the conductors 
    returned by the :py:meth:`conductors` method.

    If elements are not computed because the corresponding
    rows and columns have been dropped from the input, a zero
    capacitance value is written there.
    """
    return super()._solve()

  def conductors(self) -> list[str]:
    """Returns the effective list of conductors present in the capacitance matrix

    The list corresponds to the rows and columns of the
    capacitance matrix.

    Note: The list of conductors is only available after a call to :py:meth:`solve`
    or :py:meth:`dump_ps`.
    """
    return super()._conductors()

  def dump_ps(self, filename):
    """Produces a PS file with the geometries

    See the manifold `ps_...` options that configure PS output.

    Note: calling this function continuously allocates memory until
    the Problem object is released.
    """
    super()._dump_ps(filename)

