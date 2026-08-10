use std::fmt::Display;
use std::marker::PhantomData;

use crate as chalk;
use crate::chalk;
use crate::AddIdentity;
use crate::Basis;
use crate::Binary;
use crate::MulIdentity;
use crate::Ring;
use crate::StandardBasis;

/// A dense, square matrix over a ring `R`, acting on the free module with basis
/// `B`.
///
/// Square matrices over a ring form a (generally non-commutative) [`Ring`]:
/// addition is entrywise and multiplication is the usual matrix product. The
/// additive identity is the zero matrix and the multiplicative identity is the
/// identity matrix.
///
/// # Invariant
/// `entries` always has length exactly `B::DIM * B::DIM`.
#[chalk(
  |> Ring where R: Clone,
)]
pub struct Matrix<R: Ring, B: Basis> {
  entries: Vec<R>,
  basis: PhantomData<B>,
}

pub type Mat<R, const N: usize> = Matrix<R, StandardBasis<N>>;

// `Clone`/`Debug`/`PartialEq`/`Eq` are written by hand rather than derived so
// they do not spuriously require `B: Clone`/`B: Eq`; the basis is only a
// phantom type parameter.
impl<R: Ring + Clone, B: Basis> Clone for Matrix<R, B> {
  fn clone(&self) -> Self {
    Matrix {
      entries: self.entries.clone(),
      basis: PhantomData,
    }
  }
}

impl<R: Ring + std::fmt::Debug, B: Basis> std::fmt::Debug for Matrix<R, B> {
  fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
    f.debug_struct("Matrix")
      .field("entries", &self.entries)
      .finish()
  }
}

impl<R: Ring, B: Basis> PartialEq for Matrix<R, B> {
  fn eq(&self, other: &Self) -> bool {
    self.entries == other.entries
  }
}

impl<R: Ring, B: Basis> Eq for Matrix<R, B> {}

impl<R: Ring, B: Basis> Binary for Matrix<R, B> {
  type Rhs = Self;
  fn into_rhs(&self) -> &Self {
    self
  }
}

impl<R: Ring, B: Basis> Matrix<R, B> {
  /// Builds a matrix whose `(row, col)` entry is `f(row, col)`.
  pub fn from_fn<F: FnMut(&B, &B) -> R>(mut f: F) -> Matrix<R, B> {
    let n = B::DIM;
    let mut entries = Vec::with_capacity(n * n);
    for i in B::elements() {
      for j in B::elements() {
        entries.push(f(&i, &j));
      }
    }
    Matrix { entries, basis: PhantomData }
  }

  /// Builds a matrix from its rows. Panics unless there are exactly `B::DIM`
  /// rows, each with exactly `B::DIM` entries.
  pub fn from_rows<
    Row: IntoIterator<Item = R>,
    Rows: IntoIterator<Item = Row>,
  >(
    rows: Rows,
  ) -> Matrix<R, B> {
    let n = B::DIM;
    let mut entries = Vec::with_capacity(n * n);
    let mut row_count = 0;
    for row in rows {
      let before = entries.len();
      entries.extend(row);
      assert_eq!(
        entries.len() - before,
        n,
        "each row must have exactly B::DIM entries"
      );
      row_count += 1;
    }
    assert_eq!(row_count, n, "must have exactly B::DIM rows");
    Matrix { entries, basis: PhantomData }
  }
}

/// Indexing by `(row, col)`. Panics if either index is `>= B::DIM`.
impl<R: Ring, B: Basis> std::ops::Index<(usize, usize)> for Matrix<R, B> {
  type Output = R;
  fn index(&self, (row, col): (usize, usize)) -> &R {
    let n = B::DIM;
    assert!(row < n && col < n, "matrix index out of range");
    &self.entries[row * n + col]
  }
}

impl<R: Ring, B: Basis> std::ops::IndexMut<(usize, usize)> for Matrix<R, B> {
  fn index_mut(&mut self, (row, col): (usize, usize)) -> &mut R {
    let n = B::DIM;
    assert!(row < n && col < n, "matrix index out of range");
    &mut self.entries[row * n + col]
  }
}

/// The scalar matrix `r * I`, embedding `R` into the matrix ring as the ring
/// homomorphism `R -> M_n(R)`.
impl<R: Ring + Clone, B: Basis> From<R> for Matrix<R, B> {
  fn from(r: R) -> Matrix<R, B> {
    Matrix::from_fn(|i, j| if i == j { r.clone() } else { R::add_id() })
  }
}

impl<R: Ring, B: Basis> AddIdentity for Matrix<R, B> {
  fn add_id() -> Matrix<R, B> {
    let n = B::DIM;
    Matrix {
      entries: (0..n * n).map(|_| R::add_id()).collect(),
      basis: PhantomData,
    }
  }

  fn is_add_id(&self) -> bool {
    self.entries.iter().all(|e| e.is_add_id())
  }
}

impl<R: Ring, B: Basis> MulIdentity for Matrix<R, B> {
  fn mul_id() -> Matrix<R, B> {
    Matrix::from_fn(|i, j| if i == j { R::mul_id() } else { R::add_id() })
  }

  fn is_mul_id(&self) -> bool {
    let n = B::DIM;
    for i in 0..n {
      for j in 0..n {
        let e = &self.entries[i * n + j];
        let ok = if i == j { e.is_mul_id() } else { e.is_add_id() };
        if !ok {
          return false;
        }
      }
    }
    true
  }
}

impl<'a, R: Ring, B: Basis> std::ops::AddAssign<&'a Matrix<R, B>>
  for Matrix<R, B>
{
  fn add_assign(&mut self, rhs: &Matrix<R, B>) {
    for (a, b) in self.entries.iter_mut().zip(rhs.entries.iter()) {
      *a += b.into_rhs();
    }
  }
}

impl<'a, R: Ring, B: Basis> std::ops::SubAssign<&'a Matrix<R, B>>
  for Matrix<R, B>
{
  fn sub_assign(&mut self, rhs: &Matrix<R, B>) {
    for (a, b) in self.entries.iter_mut().zip(rhs.entries.iter()) {
      *a -= b.into_rhs();
    }
  }
}

impl<'a, R: Ring + Clone, B: Basis> std::ops::MulAssign<&'a Matrix<R, B>>
  for Matrix<R, B>
{
  fn mul_assign(&mut self, rhs: &Matrix<R, B>) {
    let n = B::DIM;
    let lhs = std::mem::take(&mut self.entries);
    let mut out: Vec<R> = (0..n * n).map(|_| R::add_id()).collect();
    // `ikj` traversal: accumulate `lhs[i,k] * rhs[k,j]` into `out[i,j]`,
    // skipping zero left operands.
    for i in 0..n {
      for k in 0..n {
        let a = &lhs[i * n + k];
        if a.is_add_id() {
          continue;
        }
        for j in 0..n {
          let mut term = a.clone();
          term *= rhs.entries[k * n + j].into_rhs();
          out[i * n + j] += term.into_rhs();
        }
      }
    }
    self.entries = out;
  }
}

// Additive inverse. Hand-written because the `#[chalk]` macro does not generate
// `Neg`, yet `Ring` (via `AddGroup`) requires it. Expressed via subtraction so
// no extra `R: Neg` bound is needed.
impl<R: Ring, B: Basis> std::ops::Neg for Matrix<R, B> {
  type Output = Matrix<R, B>;
  fn neg(self) -> Matrix<R, B> {
    let mut result = Matrix::add_id();
    result -= &self;
    result
  }
}

impl<R: Ring + Display, B: Basis> Display for Matrix<R, B> {
  fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
    let n = B::DIM;
    for i in 0..n {
      if i > 0 {
        writeln!(fmt)?;
      }
      let mut separator = "";
      for j in 0..n {
        write!(fmt, "{separator}{}", self.entries[i * n + j])?;
        separator = " ";
      }
    }
    Ok(())
  }
}

#[cfg(test)]
mod tests {
  use super::*;
  use crate::StandardBasis;

  type M = Matrix<i32, StandardBasis<3>>;

  #[test]
  fn identities() {
    assert!(M::add_id().is_add_id());
    assert!(!M::add_id().is_mul_id());
    assert!(M::mul_id().is_mul_id());
    assert!(!M::mul_id().is_add_id());
    // The identity is the scalar matrix `1 * I`.
    assert_eq!(M::mul_id(), M::from(1));
    assert_eq!(M::from(0), M::add_id());
  }

  #[test]
  fn from_scalar_is_diagonal() {
    let m = M::from(5);
    for i in 0..3 {
      for j in 0..3 {
        assert_eq!(m[(i, j)], if i == j { 5 } else { 0 });
      }
    }
  }

  #[test]
  fn add_and_sub_are_entrywise() {
    let a = M::from_rows([[1, 2, 3], [4, 5, 6], [7, 8, 9]]);
    let b = M::from_rows([[9, 8, 7], [6, 5, 4], [3, 2, 1]]);
    assert_eq!(
      a.clone() + &b,
      M::from_rows([[10, 10, 10], [10, 10, 10], [10, 10, 10]])
    );
    assert_eq!(a.clone() - &a, M::add_id());
    // Negation is `0 - a`.
    assert_eq!(-a.clone(), M::add_id() - &a);
  }

  #[test]
  fn mul_matches_hand_computation() {
    // Multiplying by the identity is a no-op either side.
    let a = M::from_rows([[1, 2, 3], [4, 5, 6], [7, 8, 9]]);
    assert_eq!(a.clone() * &M::mul_id(), a);
    assert_eq!(M::mul_id() * &a, a);
    // Multiplying by zero yields zero.
    assert_eq!(a.clone() * &M::add_id(), M::add_id());

    // A concrete 2x2 product embedded in a 3x3 corner.
    let x = M::from_rows([[1, 2, 0], [3, 4, 0], [0, 0, 0]]);
    let y = M::from_rows([[5, 6, 0], [7, 8, 0], [0, 0, 0]]);
    // [[1,2],[3,4]] * [[5,6],[7,8]] == [[19,22],[43,50]].
    assert_eq!(x * &y, M::from_rows([[19, 22, 0], [43, 50, 0], [0, 0, 0]]));
  }

  #[test]
  fn mul_is_not_commutative() {
    let a = M::from_rows([[1, 1, 0], [0, 1, 0], [0, 0, 1]]);
    let b = M::from_rows([[1, 0, 0], [1, 1, 0], [0, 0, 1]]);
    assert_ne!(a.clone() * &b, b * &a);
  }

  #[test]
  fn mul_is_associative_and_distributes() {
    let a = M::from_rows([[1, 2, 0], [0, 1, 3], [1, 0, 1]]);
    let b = M::from_rows([[0, 1, 1], [2, 0, 0], [1, 1, 2]]);
    let c = M::from_rows([[1, 0, 2], [0, 3, 0], [4, 0, 1]]);
    // (a*b)*c == a*(b*c).
    assert_eq!((a.clone() * &b) * &c, a.clone() * &(b.clone() * &c));
    // a*(b+c) == a*b + a*c.
    assert_eq!(
      a.clone() * &(b.clone() + &c),
      (a.clone() * &b) + &(a.clone() * &c)
    );
  }

  #[test]
  fn display_is_a_grid() {
    let m = M::from_rows([[1, 2, 3], [4, 5, 6], [7, 8, 9]]);
    assert_eq!(format!("{m}"), "1 2 3\n4 5 6\n7 8 9");
  }

  // Compile-time confirmation that `Matrix` satisfies the `Ring` alias (which,
  // via `AddGroup`, requires the hand-written `Neg`).
  #[test]
  fn matrix_is_a_ring() {
    fn assert_ring<T: Ring>() {}
    assert_ring::<Matrix<i32, StandardBasis<3>>>();
    assert_ring::<Matrix<i32, StandardBasis<1>>>();
  }

  #[test]
  #[should_panic]
  fn from_rows_wrong_shape_panics() {
    let _ = M::from_rows([[1, 2], [3, 4]]);
  }

  #[test]
  fn index_mut_sets_entries() {
    let mut m = M::mul_id();
    m[(0, 2)] = 7;
    assert_eq!(m[(0, 2)], 7);
    assert_eq!(m, M::from_rows([[1, 0, 7], [0, 1, 0], [0, 0, 1]]));
  }

  #[test]
  #[should_panic]
  fn index_out_of_range_panics() {
    let _ = M::add_id()[(3, 0)];
  }
}
