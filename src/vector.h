
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

  vector(double x, double y = 0.0, double z = 0.0)
  {
    for (unsigned int i = 0; i < N; ++i) {
      m_v[i] = 0.0;
    }
    set(0, x);
    set(1, y);
    set(2, z);
  }

  vector(double *a)
  {
    for (unsigned int i = 0; i < N; ++i) {
      m_v[i] = a[i];
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
    return *this;
  }

  bool operator==(const vector<N> &other) const
  {
    for (unsigned int i = 0; i < N; ++i) {
      if (m_v[i] != other.m_v[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const vector<N> &other) const
  {
    return !operator==(other);
  }

  bool operator<(const vector<N> &other) const
  {
    for (unsigned int i = 0; i < N; ++i) {
      if (m_v[i] != other.m_v[i]) {
        return m_v[i] < other.m_v[i];
      }
    }
    return false;
  }

  double &operator[] (unsigned int n)
  {
    return m_v[n];
  }

  void set(unsigned int n, double v)
  {
    if (n < N) {
      m_v[n] = v;
    }
  }

  double operator[] (unsigned int n) const
  {
    return n < N ? m_v[n] : 0.0;
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

  void store(double *a)
  {
    for (unsigned int i = 0; i < N; ++i) {
      a[i] = m_v[i];
    }
  }

  double operator*(const vector<N> &other) const
  {
    double s = 0.0;
    for (unsigned int i = 0; i < N; ++i) {
      s += m_v[i] * other.m_v[i];
    }
    return s;
  }

  vector<N> operator*(double s) const
  {
    vector<N> v = *this;
    v *= s;
    return v;
  }

  vector<N> &operator*=(double s)
  {
    for (unsigned int i = 0; i < N; ++i) {
      m_v[i] *= s;
    }
    return *this;
  }

  vector<N> operator+(const vector<N> &other) const
  {
    vector<N> res(*this);
    res += other;
    return res;
  }

  vector<N> &operator+=(const vector<N> &other)
  {
    for (unsigned int i = 0; i < N; ++i) {
      m_v[i] += other.m_v[i];
    }
    return *this;
  }

  vector<N> operator-(const vector<N> &other) const
  {
    vector<N> res(*this);
    res -= other;
    return res;
  }

  vector<N> &operator-=(const vector<N> &other)
  {
    for (unsigned int i = 0; i < N; ++i) {
      m_v[i] -= other.m_v[i];
    }
    return *this;
  }

  vector<N> operator-() const
  {
    return *this * -1.0;
  }

  std::string to_string(double epsilon = 1e-12) const
  {
    std::ostringstream os;
    os << std::setprecision(12) << "(";
    for (unsigned int i = 0; i < N; ++i) {
      if (i > 0) {
        os << ",";
      }
      double v = m_v[i];
      os << (fabs(v) < epsilon ? 0.0 : v);
    }
    os << ")";
    return os.str();
  }

  double x() const { return this->operator[](0); }
  double y() const { return this->operator[](1); }
  double z() const { return this->operator[](2); }
  double &x() { return this->operator[](0); }
  double &y() { return this->operator[](1); }
  double &z() { return this->operator[](2); }

private:
  double m_v[N];
};

typedef vector<4> Vector4d;
typedef vector<3> Vector3d;
typedef vector<2> Vector2d;

template <unsigned int N>
inline vector<N> operator*(double s, const vector<N> &v)
{
  return v * s;
}

inline Vector3d cross_prod(const Vector3d &a, const Vector3d &b)
{
  double x =  a.y() * b.z() - a.z() * b.y();
  double y = -a.x() * b.z() + a.z() * b.x();
  double z =  a.x() * b.y() - a.y() * b.x();
  return Vector3d(x, y, z);
}

#endif
