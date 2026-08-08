use crate::private;

/// Represents an formal indeterminant symbol is a polynomial [`Ring`] or formal
/// power series. It is used to represent elements that are transcendental over
/// a base [`Ring`].
pub trait Indeterminant: Default + Eq {
  /// How the indeterminant should be printed as part of the
  /// [`std::fmt::Display`] trait.
  fn display() -> &'static str;
}

/// Macro that simplifies creation of indeterminants. For example,
/// ```
/// use chalk::indeterminants;
/// use chalk::polynomial::Indeterminant;
///
/// indeterminants! {
///   X: "x",
///   Y: "y"
/// }
///
/// assert_eq!(X::display(), "x");
/// assert_eq!(Y::display(), "y");
/// ```
/// constructs two types, `X` and `Y` that represent indeterminants and are
/// displayed as `"x"` and `"y"` repsepectively.
#[macro_export]
macro_rules! indeterminants {
  ($($i:ident: $s:literal),*) => {
    $(#[derive(Clone, Debug, Default, Eq, PartialEq)]
    struct $i;
    impl $crate::polynomial::Indeterminant for $i {
      fn display() -> &'static str {
        $s
      }
    })*
  };
}

/// An ordered collection of [`Indeterminant`]s, used to parameterize polynomial
/// rings over multiple variables. This trait is unimplementable
/// outside the [`chalk`] crate, and is only implemented for tuples of types
/// already implementing [`Indeterminant`]. Tuples of size up to 12 are
/// supported.
pub trait Indeterminants: Default + Eq + private::Sealed {
  /// The number of indeterminates.
  const COUNT: usize;

  type Array: Clone
    + Copy
    + std::fmt::Debug
    + Default
    + Eq
    + std::ops::IndexMut<usize, Output = usize>
    + IntoIterator<Item = usize>
    + std::hash::Hash
    + Ord
    + PartialEq
    + PartialOrd;

  /// The display name of the indeterminate at `index`. Panics if `index` is
  /// greater than or equal to `Self::COUNT`.
  fn display(index: usize) -> &'static str;
}

macro_rules! count_idents {
  () => (0usize);
  ($head:ident $($tail:ident)*) => (1usize + count_idents!($($tail)*));
}

macro_rules! impl_indeterminants {
  ($($t:ident),+) => {
    impl<$($t: Indeterminant),+> private::Sealed for ($($t,)+) {}
    impl<$($t: Indeterminant),+> Indeterminants for ($($t,)+) {
      const COUNT: usize = count_idents!($($t)*);

      type Array = [usize; { count_idents!($($t)*) }];

      fn display(index: usize) -> &'static str {
        [$(<$t as Indeterminant>::display()),+][index]
      }
    }
  };
}

impl_indeterminants!(A);
impl_indeterminants!(A, B);
impl_indeterminants!(A, B, C);
impl_indeterminants!(A, B, C, D);
impl_indeterminants!(A, B, C, D, E);
impl_indeterminants!(A, B, C, D, E, F);
impl_indeterminants!(A, B, C, D, E, F, G);
impl_indeterminants!(A, B, C, D, E, F, G, H);
impl_indeterminants!(A, B, C, D, E, F, G, H, I);
impl_indeterminants!(A, B, C, D, E, F, G, H, I, J);
impl_indeterminants!(A, B, C, D, E, F, G, H, I, J, K);
impl_indeterminants!(A, B, C, D, E, F, G, H, I, J, K, L);

#[cfg(test)]
mod tests {
  use super::*;

  indeterminants! {X: "x", Y: "y", Z: "z"}

  #[test]
  fn indeterminant_display_reports_its_symbol() {
    assert_eq!(X::display(), "x");
    assert_eq!(Y::display(), "y");
    assert_eq!(Z::display(), "z");
  }

  #[test]
  fn count_matches_the_tuple_arity() {
    assert_eq!(<(X,) as Indeterminants>::COUNT, 1);
    assert_eq!(<(X, Y) as Indeterminants>::COUNT, 2);
    assert_eq!(<(X, Y, Z) as Indeterminants>::COUNT, 3);
  }

  #[test]
  fn display_indexes_into_the_tuple() {
    assert_eq!(<(X, Y, Z) as Indeterminants>::display(0), "x");
    assert_eq!(<(X, Y, Z) as Indeterminants>::display(1), "y");
    assert_eq!(<(X, Y, Z) as Indeterminants>::display(2), "z");
  }

  #[test]
  fn array_type_length_tracks_the_arity() {
    // The associated `Array` is a fixed-size array with one slot per variable.
    let a = <(X, Y, Z) as Indeterminants>::Array::default();
    assert_eq!(a.len(), 3);
    let a = <(X, Y) as Indeterminants>::Array::default();
    assert_eq!(a.len(), 2);
  }

  #[test]
  #[should_panic]
  fn display_out_of_range_panics() {
    let _ = <(X, Y) as Indeterminants>::display(2);
  }
}
