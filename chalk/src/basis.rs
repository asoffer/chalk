/// A finite, ordered set of `DIM` distinguished, individually named elements.
///
/// A `Basis` is purely combinatorial. It fixes how many generators there are,
/// what each is called (via [`Display`](std::fmt::Display)), and gives each a
/// distinct index in `0..DIM`. It says nothing about what may be *done* with
/// them; that is supplied by whatever algebraic structure depends the basis.
///
/// Because the trait carries no algebraic structure of its own, the same basis
/// type serves both roles unchanged. Implementations are usually produced by
/// the [`basis!`](crate::basis) macro, or taken from [`StandardBasis`].
pub trait Basis: Clone + std::fmt::Display + Eq {
  /// The number of distinguished elements the basis names. Read as the rank of
  /// a free module (its dimension, when the ring is a field), as the size of an
  /// alphabet, or as whatever the consuming structure takes a generating set to
  /// mean.
  const DIM: usize;

  /// An iterator over all `DIM` elements of the basis, each yielded exactly
  /// once. Any two invocations of `elements` must return iterators that iterate
  /// over the same elements in the same order.
  fn elements() -> impl Iterator<Item = Self>;

  /// Returns a unique index for `self` within the range `0..Self::DIM`.
  /// Distinct elements have distinct indices, so this is a bijection onto
  /// `0..DIM` and consumers may use it to store one value per generator in a
  /// flat array.
  fn index(self) -> usize;
}

/// A [`Basis`] whose generators are additionally ordered.
///
/// Structures that care about the relative position of two generators, rather
/// than only about their identity, require this in place of plain [`Basis`].
/// The triangular matrix types are the motivating example: deciding whether an
/// entry sits strictly above the diagonal is a comparison of two basis
/// elements.
pub trait PartialOrdBasis: Basis + std::cmp::PartialOrd {}
impl<B: Basis + std::cmp::PartialOrd> PartialOrdBasis for B {}

/// A basis of `N` generators named `e0` through `e{N-1}`.
///
/// This is the standard basis of `R^N` when used as module coordinates, and an
/// `N`-letter alphabet when used to spell words in a
/// [`FreeMonoid`](crate::FreeMonoid).
#[derive(Clone, Copy, Debug, Default, Eq, PartialEq, PartialOrd)]
pub struct StandardBasis<const N: usize>(usize);

impl<const N: usize> std::fmt::Display for StandardBasis<N> {
  fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
    write!(fmt, "e{}", self.0)
  }
}

impl<const N: usize> Basis for StandardBasis<N> {
  const DIM: usize = N;

  fn elements() -> impl Iterator<Item = Self> {
    (0..Self::DIM).map(Self)
  }

  fn index(self) -> usize {
    self.0
  }
}

/// Declares a named [`Basis`] type with `DIM` generators, displayed as
/// `$prefix` followed by the generator's index. For example,
/// ```
/// use chalk::basis;
/// use chalk::Basis;
///
/// basis! { E: "e", DIM = 3 }
///
/// assert_eq!(E::DIM, 3);
/// assert_eq!(format!("{}", E(0)), "e0");
/// assert_eq!(format!("{}", E(2)), "e2");
/// ```
/// constructs a type `E` naming three generators, displayed as `e0`, `e1`, and
/// `e2`. The declaration says only that; how the generators are read is decided
/// by the structure they are handed to. The same `E` can index the rows and
/// columns of a three-by-three matrix, or serve as a three-letter alphabet:
/// ```
/// use chalk::basis;
/// use chalk::FreeMonoid;
/// use chalk::MulIdentity;
///
/// basis! { Letter: "x", DIM = 2 }
///
/// // Words in the free monoid on `Letter` may be longer than `DIM`; the
/// // generators are an alphabet here, not a set of coordinates.
/// let mut word = FreeMonoid::from(Letter(0));
/// word *= &FreeMonoid::from(Letter(1));
/// word *= &FreeMonoid::from(Letter(0));
/// assert_eq!(word.to_string(), "x0x1x0");
///
/// assert!(FreeMonoid::<Letter>::mul_id().is_mul_id());
/// ```
#[macro_export]
macro_rules! basis {
  ($name:ident: $prefix:literal, DIM = $dim:literal) => {
    #[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
    struct $name(usize);
    impl $crate::Basis for $name {
      const DIM: usize = $dim;

      fn elements() -> impl Iterator<Item = Self> {
        (0usize..Self::DIM).map(Self)
      }

      fn index(self) -> usize {
        self.0
      }
    }

    impl std::fmt::Display for $name {
      fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(fmt, "{}{}", $prefix, self.0)
      }
    }
  };
}

#[cfg(test)]
mod tests {
  use super::*;

  basis! { E: "e", DIM = 3 }
  basis! { V: "v", DIM = 2 }

  #[test]
  fn declared_basis_uses_its_own_prefix() {
    // Each declaration labels its generators with the prefix it was given, so
    // two bases in the same scope are distinguishable by name.
    assert_eq!(format!("{}", V(0)), "v0");
    assert_eq!(format!("{}", V(1)), "v1");
    assert_eq!(format!("{}", E(1)), "e1");
  }

  #[test]
  fn declared_basis_indices_are_a_bijection_onto_the_range() {
    let indices: Vec<usize> = E::elements().map(Basis::index).collect();
    assert_eq!(indices, vec![0, 1, 2]);
    assert_eq!(E::elements().count(), E::DIM);
    assert_eq!(V::elements().map(Basis::index).collect::<Vec<_>>(), vec![0, 1]);
  }

  #[test]
  fn declared_basis_reports_its_dimension_and_labels() {
    assert_eq!(E::DIM, 3);
    assert_eq!(format!("{}", E(0)), "e0");
    assert_eq!(format!("{}", E(1)), "e1");
    assert_eq!(format!("{}", E(2)), "e2");
  }

  #[test]
  fn standard_basis_dimension_and_default_labels() {
    assert_eq!(StandardBasis::<4>::DIM, 4);
    assert_eq!(StandardBasis::<0>::DIM, 0);
    assert_eq!(format!("{}", StandardBasis::<4>(2)), "e2");
  }
}
