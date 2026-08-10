use crate as chalk;
use crate::chalk;
use crate::matrix::Matrix;
use crate::AddIdentity;
use crate::Binary;
use crate::PartialOrdBasis;
use crate::Ring;

/// A strictly-upper-triangular matrix over `R`: every entry on or below the
/// diagonal is the additive identity.
///
/// Such matrices are nilpotent (`N` to the `B::DIM` power is zero) and closed
/// under addition, subtraction, and multiplication, but they contain no
/// multiplicative identity. They therefore form a [`RingWithoutIdentity`] (an
/// "rng").
///
/// [`RingWithoutIdentity`]: crate::RingWithoutIdentity
#[chalk(
  |> RingWithoutIdentity where R: Clone,
)]
pub struct NilpotentMatrix<R: Ring, B: PartialOrdBasis>(Matrix<R, B>);

// Hand-written (rather than derived) to avoid spurious `B: Clone`/`B: Eq`
// bounds; the wrapped `Matrix` already provides basis-free impls.
impl<R: Ring + Clone, B: PartialOrdBasis> Clone for NilpotentMatrix<R, B> {
  fn clone(&self) -> Self {
    NilpotentMatrix(self.0.clone())
  }
}

impl<R: Ring + std::fmt::Debug, B: PartialOrdBasis> std::fmt::Debug
  for NilpotentMatrix<R, B>
{
  fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
    f.debug_tuple("NilpotentMatrix").field(&self.0).finish()
  }
}

impl<R: Ring, B: PartialOrdBasis> PartialEq for NilpotentMatrix<R, B> {
  fn eq(&self, other: &Self) -> bool {
    self.0 == other.0
  }
}

impl<R: Ring, B: PartialOrdBasis> Eq for NilpotentMatrix<R, B> {}

impl<R: Ring, B: PartialOrdBasis> Binary for NilpotentMatrix<R, B> {
  type Rhs = Self;
  fn into_rhs(&self) -> &Self {
    self
  }
}

impl<R: Ring, B: PartialOrdBasis> NilpotentMatrix<R, B> {
  /// Builds a strictly-upper-triangular matrix whose above-diagonal `(row,
  /// col)` entry is `f(row, col)`. `f` is consulted only for `row < col`; all
  /// other entries are the additive identity.
  pub fn from_fn<F: FnMut(&B, &B) -> R>(mut f: F) -> NilpotentMatrix<R, B> {
    NilpotentMatrix(Matrix::from_fn(|i: &B, j: &B| {
      if i.partial_cmp(&j) == Some(std::cmp::Ordering::Less) {
        f(i, j)
      } else {
        R::add_id()
      }
    }))
  }

  /// Wraps a matrix, asserting it is strictly upper triangular (the additive
  /// identity on and below the diagonal). Panics otherwise.
  pub fn from_matrix(m: Matrix<R, B>) -> NilpotentMatrix<R, B> {
    let n = B::DIM;
    for i in 0..n {
      for j in 0..n {
        if i >= j {
          assert!(
            m[(i, j)].is_add_id(),
            "matrix is not strictly upper triangular"
          );
        }
      }
    }
    NilpotentMatrix(m)
  }
}

/// Read-only indexing by `(row, col)`. There is no `IndexMut`: mutating an
/// entry could violate the strictly-upper-triangular invariant.
impl<R: Ring, B: PartialOrdBasis> std::ops::Index<(usize, usize)>
  for NilpotentMatrix<R, B>
{
  type Output = R;
  fn index(&self, index: (usize, usize)) -> &R {
    &self.0[index]
  }
}

impl<R: Ring, B: PartialOrdBasis> AddIdentity for NilpotentMatrix<R, B> {
  fn add_id() -> NilpotentMatrix<R, B> {
    NilpotentMatrix(Matrix::add_id())
  }

  fn is_add_id(&self) -> bool {
    self.0.is_add_id()
  }
}

impl<'a, R: Ring, B: PartialOrdBasis>
  std::ops::AddAssign<&'a NilpotentMatrix<R, B>> for NilpotentMatrix<R, B>
{
  fn add_assign(&mut self, rhs: &NilpotentMatrix<R, B>) {
    self.0 += &rhs.0;
  }
}

impl<'a, R: Ring, B: PartialOrdBasis>
  std::ops::SubAssign<&'a NilpotentMatrix<R, B>> for NilpotentMatrix<R, B>
{
  fn sub_assign(&mut self, rhs: &NilpotentMatrix<R, B>) {
    self.0 -= &rhs.0;
  }
}

impl<'a, R: Ring + Clone, B: PartialOrdBasis>
  std::ops::MulAssign<&'a NilpotentMatrix<R, B>> for NilpotentMatrix<R, B>
{
  fn mul_assign(&mut self, rhs: &NilpotentMatrix<R, B>) {
    self.0 *= &rhs.0;
  }
}

impl<R: Ring, B: PartialOrdBasis> std::ops::Neg for NilpotentMatrix<R, B> {
  type Output = NilpotentMatrix<R, B>;
  fn neg(self) -> NilpotentMatrix<R, B> {
    NilpotentMatrix(-self.0)
  }
}

/// The strictly-upper-triangular matrix, viewed as a general matrix.
impl<R: Ring, B: PartialOrdBasis> From<NilpotentMatrix<R, B>> for Matrix<R, B> {
  fn from(n: NilpotentMatrix<R, B>) -> Matrix<R, B> {
    n.0
  }
}

#[cfg(test)]
mod tests {
  use super::*;
  use crate::Basis;
  use crate::MulIdentity;
  use crate::RingWithoutIdentity;
  use crate::StandardBasis;

  type N = NilpotentMatrix<i32, StandardBasis<3>>;

  fn sample() -> N {
    // [[0,1,2],[0,0,3],[0,0,0]]
    N::from_fn(|i, j| (i.index() + j.index()) as i32)
  }

  #[test]
  fn from_fn_zeros_the_lower_triangle_and_diagonal() {
    let n = sample();
    for i in 0..3 {
      for j in 0..3 {
        let expected = if i < j { (i + j) as i32 } else { 0 };
        assert_eq!(n[(i, j)], expected);
      }
    }
  }

  #[test]
  fn closed_under_addition_and_multiplication() {
    let a = sample();
    let b = N::from_fn(|_, _| 1);
    // Sum and product stay strictly upper triangular (no panic on re-wrapping),
    // which we confirm by round-tripping through a general matrix.
    let sum: Matrix<i32, StandardBasis<3>> = (a.clone() + &b).into();
    let _ = N::from_matrix(sum);
    let prod: Matrix<i32, StandardBasis<3>> = (a.clone() * &b).into();
    let _ = N::from_matrix(prod);
  }

  #[test]
  fn is_nilpotent_of_index_at_most_dim() {
    let a = sample();
    // N^3 == 0 for a 3x3 strictly-upper matrix.
    let cube = a.clone() * &a * &a;
    assert!(cube.is_add_id());
    // N^2 need not be zero.
    assert!(!(a.clone() * &a).is_add_id());
  }

  #[test]
  fn embeds_into_general_matrix() {
    let m: Matrix<i32, StandardBasis<3>> = sample().into();
    assert_eq!(m, Matrix::from_rows([[0, 1, 2], [0, 0, 3], [0, 0, 0]]));
    // The identity is not a nilpotent matrix.
    assert!(!Matrix::<i32, StandardBasis<3>>::mul_id().is_add_id());
  }

  #[test]
  fn rng_is_a_ring_without_identity() {
    fn assert_rng<T: RingWithoutIdentity>() {}
    assert_rng::<NilpotentMatrix<i32, StandardBasis<3>>>();
  }

  #[test]
  #[should_panic]
  fn from_matrix_rejects_non_strictly_upper() {
    let _ = N::from_matrix(Matrix::mul_id());
  }
}
