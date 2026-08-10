use crate as chalk;
use crate::chalk;
use crate::matrix::Matrix;
use crate::matrix::NilpotentMatrix;
use crate::Binary;
use crate::Invertible;
use crate::MulIdentity;
use crate::PartialOrdBasis;
use crate::PartiallyInvertible;
use crate::Ring;

/// An upper-triangular matrix over `R` with the multiplicative identity on the
/// diagonal (a "unipotent" matrix).
///
/// Writing such a matrix as `I + N` with `N` strictly upper triangular (hence
/// [nilpotent](NilpotentMatrix)), unipotent matrices are closed under
/// multiplication and every one is invertible — its inverse is the terminating
/// series `I - N + N^2 - ...`. They therefore form a [`MulGroup`].
///
/// [`MulGroup`]: crate::MulGroup
#[chalk(
  |> MulGroup where R: Clone,
)]
pub struct UnipotentMatrix<R: Ring, B: PartialOrdBasis>(Matrix<R, B>);

// Hand-written (rather than derived) to avoid spurious `B: Clone`/`B: Eq`
// bounds; the wrapped `Matrix` already provides basis-free impls.
impl<R: Ring + Clone, B: PartialOrdBasis> Clone for UnipotentMatrix<R, B> {
  fn clone(&self) -> Self {
    UnipotentMatrix(self.0.clone())
  }
}

impl<R: Ring + std::fmt::Debug, B: PartialOrdBasis> std::fmt::Debug
  for UnipotentMatrix<R, B>
{
  fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
    f.debug_tuple("UnipotentMatrix").field(&self.0).finish()
  }
}

impl<R: Ring, B: PartialOrdBasis> PartialEq for UnipotentMatrix<R, B> {
  fn eq(&self, other: &Self) -> bool {
    self.0 == other.0
  }
}

impl<R: Ring, B: PartialOrdBasis> Eq for UnipotentMatrix<R, B> {}

impl<R: Ring, B: PartialOrdBasis> Binary for UnipotentMatrix<R, B> {
  type Rhs = Self;
  fn into_rhs(&self) -> &Self {
    self
  }
}

impl<R: Ring, B: PartialOrdBasis> UnipotentMatrix<R, B> {
  /// Builds a unipotent matrix whose above-diagonal `(row, col)` entry is
  /// `f(row, col)`. `f` is consulted only for `row < col`; the diagonal is the
  /// multiplicative identity and everything below it is the additive identity.
  pub fn from_fn<F: FnMut(&B, &B) -> R>(mut f: F) -> UnipotentMatrix<R, B> {
    UnipotentMatrix(Matrix::from_fn(|i: &B, j: &B| match i.partial_cmp(j) {
      Some(std::cmp::Ordering::Equal) => R::mul_id(),
      Some(std::cmp::Ordering::Less) => f(i, j),
      _ => R::add_id(),
    }))
  }

  /// Wraps a matrix, asserting it is unipotent (the multiplicative identity on
  /// the diagonal and the additive identity below it). Panics otherwise.
  pub fn from_matrix(m: Matrix<R, B>) -> UnipotentMatrix<R, B> {
    let n = B::DIM;
    for i in 0..n {
      for j in 0..n {
        if i == j {
          assert!(
            m[(i, j)].is_mul_id(),
            "diagonal must be the multiplicative identity"
          );
        } else if i > j {
          assert!(m[(i, j)].is_add_id(), "below-diagonal entries must be zero");
        }
      }
    }
    UnipotentMatrix(m)
  }
}

/// Read-only indexing by `(row, col)`. There is no `IndexMut`: mutating an
/// entry could violate the unipotent invariant.
impl<R: Ring, B: PartialOrdBasis> std::ops::Index<(B, B)>
  for UnipotentMatrix<R, B>
{
  type Output = R;
  fn index(&self, (i, j): (B, B)) -> &R {
    &self.0[(i.index(), j.index())]
  }
}

impl<R: Ring, B: PartialOrdBasis> MulIdentity for UnipotentMatrix<R, B> {
  fn mul_id() -> UnipotentMatrix<R, B> {
    UnipotentMatrix(Matrix::mul_id())
  }

  fn is_mul_id(&self) -> bool {
    self.0.is_mul_id()
  }
}

impl<'a, R: Ring + Clone, B: PartialOrdBasis>
  std::ops::MulAssign<&'a UnipotentMatrix<R, B>> for UnipotentMatrix<R, B>
{
  fn mul_assign(&mut self, rhs: &UnipotentMatrix<R, B>) {
    self.0 *= &rhs.0;
  }
}

impl<R: Ring + Clone, B: PartialOrdBasis> PartiallyInvertible
  for UnipotentMatrix<R, B>
{
  fn try_inverse(&self) -> Option<UnipotentMatrix<R, B>> {
    // With `N = self - I` (nilpotent, so `N^DIM == 0`), the inverse is the
    // finite geometric series `sum_{k=0}^{DIM-1} (-N)^k`.
    let id = Matrix::<R, B>::mul_id();
    let neg_n = id.clone() - &self.0; // -(self - I) == -N
    let mut power = id.clone(); // (-N)^0 == I
    let mut result = id.clone();
    for _ in 1..B::DIM {
      power *= &neg_n; // (-N)^k
      result += &power;
    }
    Some(UnipotentMatrix(result))
  }
}

impl<R: Ring + Clone, B: PartialOrdBasis> Invertible for UnipotentMatrix<R, B> {}

/// The unipotent matrix, viewed as a general matrix.
impl<R: Ring, B: PartialOrdBasis> From<UnipotentMatrix<R, B>> for Matrix<R, B> {
  fn from(u: UnipotentMatrix<R, B>) -> Matrix<R, B> {
    u.0
  }
}

/// `N |-> I + N`, the bijection from strictly-upper to unipotent matrices.
impl<R: Ring + Clone, B: PartialOrdBasis> From<NilpotentMatrix<R, B>>
  for UnipotentMatrix<R, B>
{
  fn from(n: NilpotentMatrix<R, B>) -> UnipotentMatrix<R, B> {
    let n: Matrix<R, B> = n.into();
    UnipotentMatrix(Matrix::mul_id() + &n)
  }
}

/// `U |-> U - I`, the inverse bijection from unipotent to strictly-upper.
impl<R: Ring + Clone, B: PartialOrdBasis> From<UnipotentMatrix<R, B>>
  for NilpotentMatrix<R, B>
{
  fn from(u: UnipotentMatrix<R, B>) -> NilpotentMatrix<R, B> {
    NilpotentMatrix::from_matrix(u.0 - &Matrix::mul_id())
  }
}

#[cfg(test)]
mod tests {
  use super::*;
  use crate::AddIdentity;
  use crate::Basis;
  use crate::MulGroup;
  use crate::StandardBasis;

  type U = UnipotentMatrix<i32, StandardBasis<3>>;

  fn sample() -> U {
    // [[1,1,2],[0,1,3],[0,0,1]]
    U::from_fn(|i, j| (i.index() + j.index()) as i32)
  }

  #[test]
  fn from_fn_sets_diagonal_to_one_and_zeros_below() {
    let u = sample();
    for i in StandardBasis::<3>::elements() {
      for j in StandardBasis::<3>::elements() {
        let expected = if i < j {
          (i.index() + j.index()) as i32
        } else if i == j {
          1
        } else {
          0
        };
        assert_eq!(u[(i, j)], expected);
      }
    }
  }

  #[test]
  fn identity_is_the_multiplicative_identity() {
    assert!(U::mul_id().is_mul_id());
    assert_eq!(sample() * &U::mul_id(), sample());
    assert_eq!(U::mul_id() * &sample(), sample());
  }

  #[test]
  fn product_of_unipotents_is_unipotent() {
    let a = sample();
    let b = U::from_fn(|_, _| 1);
    // Round-trip through a general matrix to confirm the product is unipotent.
    let prod: Matrix<i32, StandardBasis<3>> = (a * &b).into();
    let _ = U::from_matrix(prod);
  }

  #[test]
  fn inverse_recovers_the_identity() {
    let u = sample();
    let inv = u.inverse();
    assert_eq!(u.clone() * &inv, U::mul_id());
    assert_eq!(inv * &u, U::mul_id());
    // `invert` mutates in place.
    let mut v = sample();
    let expected = v.inverse();
    v.invert();
    assert_eq!(v, expected);
  }

  #[test]
  fn conversions_round_trip_through_nilpotent() {
    use crate::matrix::NilpotentMatrix;
    let u = sample();
    // U -> N -> U recovers the original.
    let n: NilpotentMatrix<i32, StandardBasis<3>> = u.clone().into();
    let back: U = n.into();
    assert_eq!(back, u);
    // The nilpotent part is exactly `U - I`.
    let n: NilpotentMatrix<i32, StandardBasis<3>> = sample().into();
    assert_eq!(n[(0, 1)], 1);
    assert_eq!(n[(0, 0)], 0);
  }

  #[test]
  fn is_a_multiplicative_group() {
    fn assert_mul_group<T: MulGroup>() {}
    assert_mul_group::<UnipotentMatrix<i32, StandardBasis<3>>>();
  }

  #[test]
  #[should_panic]
  fn from_matrix_rejects_non_unipotent() {
    let _ = U::from_matrix(Matrix::add_id());
  }
}
