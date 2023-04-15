
#if !defined(matrix_H)
#define matrix_H

#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>

#include "vector.h"

template <unsigned int N, unsigned int M>
class matrix
{
public:
  matrix()
  {
    for (unsigned int i = 0; i < N; ++i) {
      for (unsigned int j = 0; j < M; ++j) {
        m_m[i][j] = 0.0;
      }
    }
  }

  matrix(double **a)
  {
    for (unsigned int i = 0; i < N; ++i) {
      for (unsigned int j = 0; j < M; ++j) {
        m_m[i][j] = a[i][j];
      }
    }
  }

  matrix(const matrix<N, M> &other)
  {
    for (unsigned int i = 0; i < N; ++i) {
      for (unsigned int j = 0; j < M; ++j) {
        m_m[i][j] = other.m_m[i][j];
      }
    }
  }

  matrix<N, M> &operator=(const matrix<N, M> &other)
  {
    if (this != &other) {
      for (unsigned int i = 0; i < N; ++i) {
        for (unsigned int j = 0; j < M; ++j) {
          m_m[i][j] = other.m_m[i][j];
        }
      }
    }
    return *this;
  }

  bool operator==(const matrix<N, M> &other) const
  {
    for (unsigned int i = 0; i < N; ++i) {
      for (unsigned int j = 0; j < M; ++j) {
        if (m_m[i][j] != other.m_m[i][j]) {
          return false;
        }
      }
    }
    return true;
  }

  bool operator!=(const matrix<N, M> &other) const
  {
    return !operator==(other);
  }

  bool is_null() const
  {
    for (unsigned int i = 0; i < N; ++i) {
      for (unsigned int j = 0; j < M; ++j) {
        if (m_m[i][j] != 0.0) {
          return false;
        }
      }
    }
    return true;
  }

  bool operator<(const matrix<N, M> &other) const
  {
    for (unsigned int i = 0; i < N; ++i) {
      for (unsigned int j = 0; j < M; ++j) {
        if (m_m[i][j] != other.m_m[i][j]) {
          return m_m[i][j] < other.m_m[i][j];
        }
      }
    }
    return false;
  }

  void set(unsigned int n, unsigned int m, double v)
  {
    if (n < N && m < M) {
      m_m[n][m] = v;
    }
  }

  double get(unsigned int n, unsigned int m) const
  {
    if (n < N && m < M) {
      return m_m[n][m];
    } else {
      return 0.0;
    }
  }

  void store(double **a)
  {
    for (unsigned int i = 0; i < N; ++i) {
      for (unsigned int j = 0; j < M; ++j) {
        a[i][j] = m_m[i][j];
      }
    }
  }

  template <unsigned int O>
  matrix<N, O> operator*(const matrix<M, O> &other) const
  {
    matrix<N, O> res;
    for (unsigned int i = 0; i < N; ++i) {
      for (unsigned int k = 0; k < O; ++k) {
        double s = 0.0;
        for (unsigned int j = 0; j < M; ++j) {
          s += m_m[i][j] * other.m_m[j][k];
        }
        res.set(i, k, s);
      }
    }
    return res;
  }

  vector<N> operator*(const vector<M> &vec) const
  {
    vector<N> res;
    for (unsigned int i = 0; i < N; ++i) {
      double s = 0.0;
      for (unsigned int j = 0; j < M; ++j) {
        s += m_m[i][j] * vec[j];
      }
      res.set(i, s);
    }
    return res;
  }

  matrix<N, M> operator*(double s) const
  {
    matrix<N, M> m = *this;
    m *= s;
    return m;
  }

  matrix<N, M> &operator*=(double s)
  {
    for (unsigned int i = 0; i < N; ++i) {
      for (unsigned int j = 0; j < M; ++j) {
        m_m[i][j] *= s;
      }
    }
    return *this;
  }

  matrix<N, M> operator+(const matrix<N, M> &other) const
  {
    matrix<N, M> res(*this);
    res += other;
    return res;
  }

  matrix<N, M> &operator+=(const matrix<N, M> &other)
  {
    for (unsigned int i = 0; i < N; ++i) {
      for (unsigned int j = 0; j < M; ++j) {
        m_m[i][j] += other.m_m[i][j];
      }
    }
    return *this;
  }

  matrix<N, M> operator-(const matrix<N, M> &other) const
  {
    matrix<N, M> res(*this);
    res -= other;
    return res;
  }

  matrix<N, M> &operator-=(const matrix<N, M> &other)
  {
    for (unsigned int i = 0; i < N; ++i) {
      for (unsigned int j = 0; j < M; ++j) {
        m_m[i][j] -= other.m_m[i][j];
      }
    }
    return *this;
  }

  matrix<N, M> operator-() const
  {
    return *this * -1.0;
  }

  std::string to_string(double epsilon = 1e-12) const
  {
    std::ostringstream os;
    os << std::setprecision(12);
    for (unsigned int i = 0; i < N; ++i) {
      if (i > 0) {
        os << std::endl;
      }
      os << "(";
      for (unsigned int j = 0; j < M; ++j) {
        if (j > 0) {
          os << ",";
        }
        double v = m_m[i][j];
        os << (fabs(v) < epsilon ? 0.0 : m_m[i][j]);
      }
      os << ")";
    }
    return os.str();
  }

private:
  double m_m[N][M];
};

typedef matrix<4, 4> Matrix4d;
typedef matrix<3, 3> Matrix3d;
typedef matrix<2, 2> Matrix2d;

template <unsigned int N, unsigned int M>
matrix<N, M> operator*(double s, const matrix<N, M> &v)
{
  return v * s;
}

template <unsigned int N>
matrix<N, N> unity()
{
  matrix<N, N> u;
  for (unsigned int i = 0; i < N; ++i) {
    u.set(i, i, 1.0);
  }
  return u;
}

template <unsigned int N>
matrix<N, N> inverse(const matrix<N, N> &m)
{
  matrix<N, N> r = unity<N>();
  matrix<N, N> w = m;

  for (unsigned int i = 0; i < N; ++i) {
    double p = w.get(i, i);
    if (p == 0.0) {
      //  Error: return null matrix
      return matrix<N, N>();
    }
    for (unsigned int j = 0; j < N; ++j) {
      w.set(i, j, w.get(i, j) / p);
      r.set(i, j, r.get(i, j) / p);
    }
    for (unsigned int k = i + 1; k < N; ++k) {
      p = w.get(k, i);
      for (unsigned int j = i; j < N; ++j) {
        w.set(k, j, w.get(k, j) - w.get(i, j) * p);
      }
      for (unsigned int j = 0; j < N; ++j) {
        r.set(k, j, r.get(k, j) - r.get(i, j) * p);
      }
    }
  }

  for (unsigned int i = N; i > 1; ) {
    --i;
    for (unsigned int k = i; k > 0; ) {
      --k;
      double p = w.get(k, i);
      for (unsigned int j = i; j < N; ++j) {
        w.set(k, j, w.get(k, j) - w.get(i, j) * p);
      }
      for (unsigned int j = 0; j < N; ++j) {
        r.set(k, j, r.get(k, j) - r.get(i, j) * p);
      }
    }
  }

  return r;
}

#endif
