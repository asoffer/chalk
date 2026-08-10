//! # Chalk
//!
//! Chalk is a symbolic algebra library. It provides a number of traits that
//! define requirements for algebraic structures, as well as mechanisms for
//! defining your own structures.
//!
//! Chalk also has a many optional features that provide implementations of
//! well-known algebraic objects conforming to these traits, such as
//! polynomials, cyclic groups, and permutations groups.
//!
//! ## Design
//!
//! ### The main idea
//! Chalk's approach is a little different from what one might expect, and
//! deserves some explanation. One might expect a trait like `MulGroup` for
//! which you are required to implement the binary operator, an identity
//! element, and an inversion operation. This approach however has a fatal flaw,
//! which becomes evident when other traits are added.
//!
//! We also want to support traits like `MulMonoid`, which has the binary
//! operator and identity element (but no inversion). Moreover, we would like
//! every group to automatically be a monoid. This could be done by adding a
//! blanket implementation for [`MulMonoid`] constrained on the type
//! implementing [`MulGroup`], like so:
//!
//! ```rust,compile_fail
//! # use chalk::MulMonoid;
//! # use chalk::MulGroup;
//! impl <G: MulGroup> MulMonoid for G {}
//! ```
//!
//! Unfortunately, this blanket implementation precludes any other
//! implementation of [`MulMonoid`] whatsoever, making it a non-starter.
//!
//! This particular problem can be solved by not letting users implement
//! [`MulGroup`] at all. Rather, they are only allowed to implement
//! [`MulMonoid`] and [`Invertible`], and then having a blanket implementation
//! (the only implementation) of [`MulGroup`] for any type that implements both
//! [`MulMonoid`] and [`Invertible`].
//!
//! Of course, the same relationship betweeen [`MulMonoid`] and [`MulSemigroup`]
//! will cause problems. Taking this approach to its logical conclusion, each
//! trait gets a blanket implementation whenever each of its properties are
//! satisfied. Users implement those properties directly.
//!
//! Chalk provides mechanisms to simplify both the definition of these basic
//! properties, as well as the "trait aliases" that provide nice names for
//! aggregations of properties.
//!
//! ### Safety
//!
//! Chalk provides several marker traits such as [`AddCommutative`] which
//! indicates that the algorithms and data structures may presume that `+` is
//! commutative. It is not uncommon for such marker traits to be marked as
//! `unsafe`, as the requirement that they advertise cannot be validated, and
//! the behavior is assumed. Chalk, does not mark these traits as `unsafe`, for
//! two primary reasons.
//!
//! First, practically, most implementations of these traits are inside
//! attribute macros, meaning users do not end up typing or seeing the `unsafe`
//! keyword.
//!
//! Secondly, these marker traits are not `unsafe` in any language-level sense.
//! That is, while authors may assume that an [`AddCommutative`] type has
//! commutative addition, it does not allow them to perform any behaviors that
//! Rust deems as `unsafe` without again explicitly using that keyword. This
//! does not make it a good idea to falsely advertise types with the wrong
//! markers. It just means that doing so does not allow any language-level
//! invariants to be broken invisibly.

mod private;

macro_rules! trait_alias_impl {
  ([$($tokens:tt)*] $name:ident<$bound_name:ident: $bounds:tt> = $($defn:tt)+) => {
    $($tokens)* trait $name<$bound_name: $bounds>: $($defn)+ {}
    impl<$bound_name: $bounds, T: $($defn)+> $name<$bound_name> for T {}
  };
  ([$($tokens:tt)*] $name:ident = $($defn:tt)+) => {
    $($tokens)* trait $name: $($defn)+ {}
    impl<T: $($defn)+> $name for T {}
  };
}

macro_rules! trait_alias {
  ($(#[doc = $text:literal])* pub $($tokens:tt)+) => {
    trait_alias_impl!{[$(#[doc = $text])* pub] $($tokens)*}
  };
  ($($tokens:tt)+) => { trait_alias_impl!{[] $($tokens)*} };
}

/// Represents types that have an additive identity. That is, the element `e`
/// for which `e + x == x + e == x`, for any `x` of the type.
pub trait AddIdentity: Eq + Sized {
  /// Returns the additive identity for the type.
  fn add_id() -> Self;

  /// Indicates wether `self` is the additive identity. The default
  /// implementation compares against `Self::add_id()` for equality. Users may
  /// customize for faster implementations.
  fn is_add_id(&self) -> bool {
    self == &Self::add_id()
  }
}

/// Represents types that have an multiplicative identity. That is, the element
/// `e` for which `e * x == x * e == x`, for any `x` of the type.
pub trait MulIdentity: Eq + Sized {
  /// Returns the multiplicative identity for the type.
  fn mul_id() -> Self;

  /// Indicates wether `self` is the multiplicative identity. The default
  /// implementation compares against `Self::mul_id()` for equality. Users may
  /// customize for faster implementations.
  fn is_mul_id(&self) -> bool {
    self == &Self::mul_id()
  }
}

/// Represents types for which some elements may have multiplicative inverses.
pub trait PartiallyInvertible: MulIdentity {
  /// Returns the multiplicative inverse of the element if it exists, and `None`
  /// otherwise.
  fn try_inverse(&self) -> Option<Self>;
}

/// Represents types that have a multiplicative inverse for all elements other
/// than the additive identity, if one exists for the type.
pub trait Invertible: PartiallyInvertible {
  /// Returns the multiplicative inverse of the element. If the type implements
  /// [`AddIdentity`], implementations must panic if it is passed in as `self`.
  fn inverse(&self) -> Self {
    self.try_inverse().unwrap()
  }

  /// Replaces `self` with its multiplicative inverse.
  fn invert(&mut self) {
    *self = self.inverse();
  }
}

pub trait Binary {
  type Rhs: ?Sized;
  fn into_rhs(&self) -> &Self::Rhs;
}

trait_alias! {
  pub Add = Binary + for<'a> std::ops::AddAssign<&'a Self::Rhs>
}

trait_alias! {
  pub Sub = Binary + for<'a> std::ops::SubAssign<&'a Self::Rhs>
}

trait_alias! {
  pub Mul = Binary + for<'a> std::ops::MulAssign<&'a Self::Rhs>
}

trait_alias! {
  pub Div = Binary + for<'a> std::ops::DivAssign<&'a Self::Rhs>
}

trait_alias! {
  pub Meet = PartialOrd + Binary + for<'a> std::ops::BitAndAssign<&'a Self::Rhs>
}

trait_alias! {
  pub Join = PartialOrd + Binary + for<'a> std::ops::BitAndAssign<&'a Self::Rhs>
}

/// Marker trait indicating that the type's implementation of addition is
/// associative.
pub trait AddAssociative: Add {}

/// Marker trait indicating that the type's implementation of multiplication is
/// associative.
pub trait MulAssociative: Mul {}

/// Marker trait indicating that the type's implementation of `meet` (`&`) is
/// associative.
pub trait MeetAssociative: Meet {}

/// Marker trait indicating that the type's implementation of `join` (`|`) is
/// associative.
pub trait JoinAssociative: Join {}

/// Marker trait indicating that the type's implementation of addition is
/// commutative.
pub trait AddCommutative: Add {}

/// Marker trait indicating that the type's implementation of multiplication is
/// commutative.
pub trait MulCommutative: Mul {}

/// Marker trait indicating that the type's implementation of `meet` (`&`) is
/// commutative.
pub trait MeetCommutative: Meet {}

/// Marker trait indicating that the type's implementation of `join` (`|`) is
/// commutative.
pub trait JoinCommutative: Join {}

/// Marker trait indicating that the type's implementation of `meet` (`&`) is
/// idempotent.
pub trait MeetIdempotent: Meet {}

/// Marker trait indicating that the type's implementation of `join` (`|`) is
/// idempotent.
pub trait JoinIdempotent: Join {}

/// Marker trait indicating that the type's implementation of multiplication
/// distributes over addition
pub trait AddMulDistributive: Add + Mul {}

/// Marker trait indicating that the type's implementation of `join` (`|`)
/// and `meet` (`&`) operations satisify the absorption law.
pub trait Absorption: Meet + Join {}

/// Marker trait indicating that the ring's multiplication can only produce
/// zero-elements if one of the operands was zero.
pub trait Domain: Ring {}

trait_alias! {
  /// An "alias trait" representing semigroups with the operator `+`. Equivalent
  /// to [`AddAssociative`]
  pub AddSemigroup = AddAssociative
}

trait_alias! {
  /// An "alias trait" representing semigroups with the operator `*`. Equivalent
  /// to [`MulAssociative`]
  pub MulSemigroup = MulAssociative
}

mod monoid;
pub use monoid::AddMonoid;
pub use monoid::FreeMonoid;
pub use monoid::MulMonoid;

trait_alias! {
  /// An "alias trait" representing groups with the operator `+`. Equivalent to
  /// [`AddMonoid`] and requiring implementations of [`std::ops::Neg`] and
  /// [`std::ops::Sub`]
  pub AddGroup = AddMonoid + std::ops::Neg<Output=Self> + Sub
}

trait_alias! {
  /// An "alias trait" representing groups with the operator `*`. Equivalent to
  /// [`MulMonoid`]` + `[`Invertible`]
  pub MulGroup = MulMonoid + Invertible
}

trait_alias! {
  /// An "alias trait" representing abelian groups with the operator `+`.
  /// Equivalent to [`AddGroup`]` + `[`AddCommutative`]
  pub AbelianAddGroup = AddGroup + AddCommutative
}

trait_alias! {
  /// An "alias trait" representing abelian groups with the operator `*`.
  /// Equivalent to [`AddGroup`]` + `[`AddCommutative`]
  pub AbelianMulGroup = MulGroup + MulCommutative
}

trait_alias! {
  /// An "alias trait" representing rings that may or may not have an identity
  /// (often called "rngs"). Equivalent to [`AbelianAddGroup`]` + `
  /// [`MulSemigroup`]` + `[`AddMulDistributive`].
  pub RingWithoutIdentity = AbelianAddGroup + MulSemigroup + AddMulDistributive
}

trait_alias! {
  /// An "alias trait" representing rings with a multiplicative identity.
  /// Equivalent to [`RingWithoutIdentity`]` + `[`MulIdentity`]
  pub Ring = RingWithoutIdentity + MulIdentity
}

trait_alias! {
  /// An "alias trait" representing commutative rings. Equivalent to
  /// [`Ring`]` + `[`MulCommutative`]
  pub CommutativeRing = Ring + MulCommutative
}

trait_alias! {
  /// An "alias trait" representing meet-semilattices. Equivalent to
  /// [`MeetCommutative`]` + `[`MeetAssociative`]` + `[`MeetIdempotent`].
  pub MeetSemilattice = MeetCommutative + MeetAssociative + MeetIdempotent
}

trait_alias! {
  /// An "alias trait" representing join-semilattices. Equivalent to
  /// [`JoinCommutative`]` + `[`JoinAssociative`]` + `[`JoinIdempotent`].
  pub JoinSemilattice = JoinCommutative + JoinAssociative + JoinIdempotent
}

trait_alias! {
  /// An "alias trait" representing lattices. Equivalent to
  /// [`MeetSemilattice`]` + `[`JoinSemilattice`]` + `[`Absorption`].
  pub Lattice = MeetSemilattice + JoinSemilattice + Absorption
}

trait_alias! {
  /// An "alias trait" representing integral domains. Equivalent to
  /// [`CommutativeRing`]` + `[`Domain`]
  pub IntegralDomain = CommutativeRing + Domain
}

trait_alias! {
  /// An "alias trait" representing division rings. Equivalent to
  /// [`Ring`]` + `[`Div`]
  pub DivisionRing = Ring + Div
}

trait_alias! {
  /// An "alias trait" representing fields. Equivalent to
  /// [`CommutativeRing`]` + `[`Div`].
  pub Field = CommutativeRing + Div
}

mod basis;
pub use basis::Basis;
pub use basis::PartialOrdBasis;
pub use basis::StandardBasis;

#[cfg(feature = "polynomial")]
pub mod polynomial;

#[cfg(feature = "cyclic")]
pub mod cyclic;

#[cfg(feature = "matrix")]
pub mod matrix;

macro_rules! implement {
  ($t:ty, [$($tr:ty,)*]) => { $(impl $tr for $t {})* };
}

macro_rules! impl_primitive {
  ($($t:ty),* $(,)?) => {
    $(
      implement!{$t, [
        AddAssociative,
        MulAssociative,
        AddCommutative,
        MulCommutative,
        AddMulDistributive,
      ]}
      impl Binary for $t  {
        type Rhs = $t ;
        fn into_rhs(&self) -> &Self::Rhs { self }
      }
      impl AddIdentity for $t {
        fn add_id() -> $t { 0 }
      }
      impl MulIdentity for $t {
        fn mul_id() -> $t { 1 }
      }
    )*
  };
}
impl_primitive! {
  i8, i16, i32, i64, i128, isize,
  u8, u16, u32, u64, u128, usize
}

impl Binary for String {
  type Rhs = str;
  fn into_rhs(&self) -> &str {
    &self
  }
}
implement! {String, [AddAssociative,]}
impl AddIdentity for String {
  fn add_id() -> Self {
    "".into()
  }
  fn is_add_id(&self) -> bool {
    self.is_empty()
  }
}

/// An attribute macro that can be applied to types in order to simplify
/// implementations of algebraic structures.
pub use chalk_proc_macro::chalk;

#[cfg(test)]
mod tests {
  use super::*;

  #[test]
  fn primitive_additive_identity() {
    assert_eq!(<i32 as AddIdentity>::add_id(), 0);
    assert_eq!(<u8 as AddIdentity>::add_id(), 0);
    assert_eq!(<i64 as AddIdentity>::add_id(), 0);
    assert_eq!(<usize as AddIdentity>::add_id(), 0);

    assert!(0i32.is_add_id());
    assert!(!1i32.is_add_id());
    assert!(!(-1i32).is_add_id());
  }

  #[test]
  fn primitive_multiplicative_identity() {
    assert_eq!(<i32 as MulIdentity>::mul_id(), 1);
    assert_eq!(<u8 as MulIdentity>::mul_id(), 1);
    assert_eq!(<i128 as MulIdentity>::mul_id(), 1);

    assert!(1i32.is_mul_id());
    assert!(!0i32.is_mul_id());
    assert!(!2i32.is_mul_id());
  }

  #[test]
  fn primitive_into_rhs_is_reflexive() {
    let x: i32 = 7;
    assert_eq!(x.into_rhs(), &7);
  }

  #[test]
  fn string_additive_identity_is_empty() {
    assert_eq!(<String as AddIdentity>::add_id(), String::new());
    assert!(String::new().is_add_id());
    assert!("".to_string().is_add_id());
    assert!(!"a".to_string().is_add_id());
  }

  #[test]
  fn string_into_rhs_borrows_as_str() {
    let s = String::from("hello");
    let r: &str = s.into_rhs();
    assert_eq!(r, "hello");
  }

  // Compile-time checks that the blanket "trait alias" implementations are
  // wired up for the primitive integer types.
  #[test]
  fn primitives_satisfy_ring_aliases() {
    fn assert_commutative_ring<T: CommutativeRing>() {}
    // The primitive integers are wired up as commutative rings via the blanket
    // trait-alias implementations. (They are not marked as `Domain`, so
    // `IntegralDomain` is intentionally not asserted here.)
    assert_commutative_ring::<i8>();
    assert_commutative_ring::<i16>();
    assert_commutative_ring::<i32>();
    assert_commutative_ring::<i64>();
    assert_commutative_ring::<i128>();
    assert_commutative_ring::<isize>();
  }

  // The unsigned primitives are not `AddGroup`s (they have no `Neg`), but
  // `impl_primitive!` must still give every listed type the marker traits and
  // both identities. `usize` is last in that list, where an earlier recursive
  // formulation of the macro dropped the markers.
  #[test]
  fn every_primitive_gets_the_marker_traits() {
    fn assert_markers<T>()
    where
      T: AddAssociative
        + MulAssociative
        + AddCommutative
        + MulCommutative
        + AddMulDistributive
        + AddIdentity
        + MulIdentity,
    {
    }
    assert_markers::<i8>();
    assert_markers::<i16>();
    assert_markers::<i32>();
    assert_markers::<i64>();
    assert_markers::<i128>();
    assert_markers::<isize>();
    assert_markers::<u8>();
    assert_markers::<u16>();
    assert_markers::<u32>();
    assert_markers::<u64>();
    assert_markers::<u128>();
    assert_markers::<usize>();
  }

  #[test]
  fn integer_arithmetic_through_operator_traits() {
    // The primitive `AddAssign`/`MulAssign` with a reference operand (used by
    // the chalk `Add`/`Mul` aliases) behaves as ordinary integer arithmetic.
    let mut x = 3i32;
    x += &4;
    assert_eq!(x, 7);
    x *= &2;
    assert_eq!(x, 14);
  }
}
