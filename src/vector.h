
#if !defined(vector_H)
#define vector_H

#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>

template <unsigned int N>
class vector
{
public:
  vector()
  {
    for (unsigned int i = 0; i < N; ++i) {
      m_v[i] = 0.0;
    }
  }

  vector(const vector<N> &other)
  {
    for (unsigned int i = 0; i < N; ++i) {
      m_v[i] = other.m_v[i];
    }
  }

  vector &operator=(const vector<N> &other)
  {
    if (this != &other) {
      for (unsigned int i = 0; i < N; ++i) {
        m_v[i] = other.m_v[i];
      }
    }
  }

  double &operator[] (unsigned int n)
  {
    return m_v[n];
  }

  double operator[] (unsigned int n) const
  {
    return m_v[n];
  }

  double norm_sq() const
  {
    double s = 0.0;
    for (unsigned int i = 0; i < N; ++i) {
      s += m_v[i] * m_v[i];
    }
    return s;
  }

  double norm() const
  {
    return sqrt(norm_sq());
  }

  double operator*(const vector<N> &other) const
  {
    double s = 0.0;
    for (unsigned int i = 0; i < N; ++i) {
      s += m_v[i] * other.m_v[i];
    }
    return s;
  }

  vector<N> operator*(double &s) const
  {
    vector<N> v;
    for (unsigned int i = 0; i < N; ++i) {
      v.m_v[i] = m_v[i] * s;
    }
    return v;
  }

  std::string to_string() const
  {
    std::ostringstream os;
    os << std::setprecision(12) << "(";
    for (unsigned int i = 0; i < N; ++i) {
      if (i > 0) {
        os << ",";
      }
      os << m_v[i];
    }
    os << ")";
    return os.str();
  }

private:
  double m_v[N];
};

class Vector3d
  : public vector<3>
{
public:
  Vector3d(double x, double y, double z)
    : vector<3>()
  {
    this->operator[](0) = x;
    this->operator[](1) = y;
    this->operator[](2) = z;
  }

  double x() const { return this->operator[](0); }
  double y() const { return this->operator[](1); }
  double z() const { return this->operator[](2); }
};

template <int N>
vector<N> operator*(double s, const vector<N> &v)
{
  return v * s;
}

Vector3d cross_prod(const Vector3d &a, const Vector3d &b)
{
  double x =  a.y() * b.z() - a.z() * b.y();
  double y = -a.x() * b.z() + a.z() * b.x();
  double z =  a.x() * b.y() - a.y() * b.x();
  return Vector3d(x, y, z);
}

#endif
