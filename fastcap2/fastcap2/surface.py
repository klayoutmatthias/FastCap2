
import math
from typing import Optional
from typing import Tuple

from fastcap2_core import Surface as _Surface

class Surface(_Surface):

  """Describes a FastCap2 surface 

  :param name: The conductor name for conductor-type surfaces.
  :param title: The optional title string.

  This object can be used instead of geometry files to specify 
  conductor or dielectric surfaces. Create this object and use it with
  :py:meth:`fastcap2.Problem.add` to specify 
  the geometry of the problem to solve.

  Details of the surface such as the kind of surface etc. are specified
  upon :py:meth:`fastcap2.Problem.add`.

  `name` is a conductor identifier - surfaces with the same
  name/group combination are considered connected. The name is mandatory
  for conductor-type surfaces. The group name can be assigned when
  adding the surface to the Problem object.

  .. code-block:: python

     import fastcap2 as fc2

     # prepares a Surface object

     surface = fc2.Surface()
     surface.name = "S"
     surface.add_meshed_quad((0, 0, 0), (1, 0, 0), (0, 1, 0), edge_width = 0.01, num = 10)

     # prepares a problem using the meshed quad two times for cap plates 

     problem = fc2.Problem(title = "A sample problem")

     # lower plate: shifted down
     problem.add(surface, d = (0, 0, -0.1))
     # lower plate: shifted up
     problem.add(surface, d = (0, 0, 0.1))

     # solves the problem and returns the cap matrix in MKS units
     cap_matrix = problem.solve()
     print(cap_matrix)
  """

  def __init__(self, name: Optional[str] = None, title: Optional[str] = None):
    kwargs = {}
    if name is not None:
      kwargs["name"] = name
    if title is not None:
      kwargs["title"] = title
    super().__init__(**kwargs)

  @property
  def title(self) -> str:
    """The title string

    The title string is an arbitrary optional string. Surfaces will see
    this title or the project title (see :py:meth:`fastcap2.Problem.title`)
    if no title is given for the surface.
    """
    return super()._get_title()

  @title.setter
  def title(self, value: str):
    return super()._set_title(value)

  @property
  def name(self) -> str:
    """The name string

    The name string specifies the conductor the surface belongs to.
    A conductor-type surface needs to have a name. Together with the 
    group name (see :py:meth:`fastcap2.Problem.add`), the
    conductor name formes an identifier. All surfaces with the same
    conductor identifier are considered connected.

    The name must not contain '%' or ',' characters.
    """
    return super()._get_name()

  @name.setter
  def name(self, value: str):
    return super()._set_name(value)

  def add_quad(self, p1: Tuple[float, float, float], p2: Tuple[float, float, float],
                     p3: Tuple[float, float, float], p4: Tuple[float, float, float], 
                     rp: Optional[Tuple[float, float, float]] = None):
    """Adds a quad to the surface

    :param p1 The first point of the quad
    :param p2 The second point of the quad
    :param p3 The third point of the quad
    :param p4 The forth point of the quad
    :param rp An optional reference per-panel reference point

    "rp" will replace the global reference point for a dielectric surface in 
    case it is missing. This feature allows specifying a reference point 
    per panel. That enables building convex geometries in a single surface -
    something which is not possible with a single reference point.
    The per-panel reference point has priority over the global reference point.
    Other that the global reference point, the per-panel reference point 
    is transformed with the panel.
    """
    if type(p1) is list:
      p1 = tuple(p1)
    if type(p2) is list:
      p2 = tuple(p2)
    if type(p3) is list:
      p3 = tuple(p3)
    if type(p4) is list:
      p4 = tuple(p4)
    args = [ p1, p2, p3, p4 ]
    if rp is not None:
      if type(rp) is list:
        rp = tuple(rp)
      args.append(rp)
    return super()._add_quad(*args)

  def add_tri(self, p1: Tuple[float, float, float], 
                    p2: Tuple[float, float, float], 
                    p3: Tuple[float, float, float],
                    rp: Optional[Tuple[float, float, float]] = None):
    """Adds a triangle to the surface

    :param p1 The first point of the triangle
    :param p2 The second point of the triangle
    :param p3 The third point of the triangle
    :param rp An optional reference per-panel reference point

    "rp" will replace the global reference point for a dielectric surface in 
    case it is missing. This feature allows specifying a reference point 
    per panel. That enables building convex geometries in a single surface -
    something which is not possible with a single reference point.
    The per-panel reference point has priority over the global reference point.
    Other that the global reference point, the per-panel reference point 
    is transformed with the panel.
    """
    if type(p1) is list:
      p1 = tuple(p1)
    if type(p2) is list:
      p2 = tuple(p2)
    if type(p3) is list:
      p3 = tuple(p3)
    args = [ p1, p2, p3 ]
    if rp is not None:
      if type(rp) is list:
        rp = tuple(rp)
      args.append(rp)
    return super()._add_quad(*args)

  def add_meshed_quad(self,
                      p0: Tuple[float, float, float], 
                      p1: Tuple[float, float, float], 
                      p2: Tuple[float, float, float], 
                      max_dim: Optional[float] = None,
                      num: Optional[float] = None,
                      edge_width: Optional[float] = None,
                      edge_fraction: Optional[float] = None,
                      rp: Optional[Tuple[float, float, float]] = None):
    """Generates a meshed quad (actually a diamond or rectangle)

    :param p0: the first corner.
    :param p1: the left-side adjacent corner.
    :param p2: the right-side adjacent corner.
    :param max_dim: the maximum dimension of the mesh tiles.
    :param num: the number of mesh tiles per shorter side.
    :param edge_width: the width of the edge.
    :param edge_fraction: the width of the edge as a fraction of the shorter side.
    :param rp: the optional per-panel reference point

    The diamond or rectangle is defined by three vectors defining 
    a point (`p0`) and the adjacent other points (`p1` and
    `p2`). The forth point is implicitly given by

    `p3 = p0 + (p1 - p0) + (p2 - p0)`

    The mesh generation supports a number of features:

    * The mesh size can be determined by number of maximum element dimension.
      If a number is given (`num`) the shorter side of the diamond is divided
      by this number to determine the mesh size. If a maximum dimension is
      given (`max_dim`), the number of tiles is determined such that the size is less
      than this dimension.
    * The edge of the figure can be resolved in smaller tiles rendering a thinner
      and more densely meshed corner. The width of the edge can either be given 
      as a fraction of the shorter side (`edge_fraction`) or directly
      (`edge_width`). Note that the edge width is implemented as 
      subtracting the corresponding lengths from the sides, so for slanted figures
      the edge width is not corresponding to the width of the edge mesh tiles.

    The edge does not contribute in the mesh dimensioning. If neither (`edge_fraction`)
    nor (`edge_width`) is present, no edge is generated.
    """

    # TODO: turn this into a faster C++ version

    epsilon = 1e-10

    l1 = math.sqrt(sum([ (p1[i] - p0[i])**2 for i in range(0, 3) ]))
    l2 = math.sqrt(sum([ (p2[i] - p0[i])**2 for i in range(0, 3) ]))

    q1 = [ (p1[i] - p0[i]) / l1 for i in range(0, 3) ]
    q2 = [ (p2[i] - p0[i]) / l2 for i in range(0, 3) ]

    if edge_fraction is not None:
      edge_width = min(l1 * edge_fraction, l2 * edge_fraction)

    l1b = 0.0
    l1e = l1
    l2b = 0.0
    l2e = l2

    with_edge = (edge_width is not None and edge_width > 0.0)

    if with_edge:
      l1b += edge_width
      l1e -= edge_width
      l2b += edge_width
      l2e -= edge_width

    if num is not None:
      max_dim = min((l1e - l1b) / num, (l2e - l2b) / num)

    if max_dim is not None:
      n1 = max(1, int(math.ceil((l1e - l1b) / max_dim - epsilon)))
      n2 = max(1, int(math.ceil((l2e - l2b) / max_dim - epsilon)))
    else:
      n1 = n2 = 1

    d1 = (l1e - l1b) / n1
    d2 = (l2e - l2b) / n2

    s1 = [ l1b + d1 * i for i in range(0, n1 + 1) ]
    s2 = [ l2b + d2 * i for i in range(0, n2 + 1) ]
    if with_edge:
      s1 = [ 0.0 ] + s1 + [ l1 ]
      s2 = [ 0.0 ] + s2 + [ l2 ]

    for i1 in range(0, len(s1) - 1):
      for i2 in range(0, len(s2) - 1):
        p1 = [ p0[i] + q1[i] * s1[i1]     + q2[i] * s2[i2]     for i in range(0, 3) ]
        p2 = [ p0[i] + q1[i] * s1[i1 + 1] + q2[i] * s2[i2]     for i in range(0, 3) ]
        p3 = [ p0[i] + q1[i] * s1[i1 + 1] + q2[i] * s2[i2 + 1] for i in range(0, 3) ]
        p4 = [ p0[i] + q1[i] * s1[i1]     + q2[i] * s2[i2 + 1] for i in range(0, 3) ]
        args = [ p1, p2, p3, p4 ]
        if rp is not None:
          args.append(rp)
        self.add_quad(*args)


