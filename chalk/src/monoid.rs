use crate::AddIdentity;
use crate::AddSemigroup;
use crate::Basis;
use crate::Binary;
use crate::MulAssociative;
use crate::MulIdentity;
use crate::MulSemigroup;

trait_alias! {
  /// An "alias trait" representing monoids with the operator `+`. Equivalent to
  /// [`AddSemigroup`]` + `[`AddIdentity`]
  pub AddMonoid = AddSemigroup + AddIdentity
}

trait_alias! {
  /// An "alias trait" representing monoids with the operator `*`. Equivalent to
  /// [`MulSemigroup`]` + `[`MulIdentity`]
  pub MulMonoid = MulSemigroup + MulIdentity
}

#[derive(Clone, Eq, PartialEq)]
pub struct FreeMonoid<B: Basis>(Vec<B>);

impl<B: Basis> From<B> for FreeMonoid<B> {
  fn from(b: B) -> Self {
    Self(vec![b])
  }
}

impl<B: Basis> std::fmt::Display for FreeMonoid<B> {
  fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
    for b in &self.0 {
      write!(fmt, "{b}")?
    }
    Ok(())
  }
}

impl<B: Basis> Binary for FreeMonoid<B> {
  type Rhs = Self;
  fn into_rhs(&self) -> &Self::Rhs {
    &self
  }
}

impl<'a, B: Basis> std::ops::MulAssign<&'a FreeMonoid<B>> for FreeMonoid<B> {
  fn mul_assign(&mut self, rhs: &'a FreeMonoid<B>) {
    self.0.extend(rhs.0.iter().cloned())
  }
}

impl<B: Basis> MulAssociative for FreeMonoid<B> {}

impl<B: Basis> MulIdentity for FreeMonoid<B> {
  fn mul_id() -> Self {
    FreeMonoid(vec![])
  }

  fn is_mul_id(&self) -> bool {
    self.0.is_empty()
  }
}

impl<B: Basis> Default for FreeMonoid<B> {
  fn default() -> Self {
    Self::mul_id()
  }
}

#[cfg(test)]
mod tests {
  use super::*;
  use crate::StandardBasis;

  crate::basis! { E: "e", DIM = 3 }
  crate::basis! { F: "f", DIM = 12 }
  crate::basis! { Empty: "z", DIM = 0 }

  /// Builds a word by multiplying single-letter monoid elements onto the
  /// identity, i.e. using only the public monoid API.
  fn word<B: Basis>(letters: impl IntoIterator<Item = B>) -> FreeMonoid<B> {
    let mut m = FreeMonoid::mul_id();
    for b in letters {
      m *= &FreeMonoid::from(b);
    }
    m
  }

  /// Exponentiation written against the [`MulMonoid`] alias, so that using it
  /// below is a check that the alias is actually usable as a bound.
  fn power<M>(base: &M, n: usize) -> M
  where
    M: MulMonoid + Binary<Rhs = M>,
  {
    let mut acc = M::mul_id();
    for _ in 0..n {
      acc *= base;
    }
    acc
  }

  #[test]
  fn identity_is_the_empty_word() {
    let id = FreeMonoid::<E>::mul_id();
    assert!(id.is_mul_id());
    assert_eq!(id.to_string(), "");
    assert!(id == FreeMonoid::<E>::mul_id());
  }

  #[test]
  fn default_is_the_identity() {
    assert!(FreeMonoid::<E>::default() == FreeMonoid::<E>::mul_id());
    assert!(FreeMonoid::<E>::default().is_mul_id());
  }

  #[test]
  fn single_letters_are_not_the_identity() {
    for e in E::elements() {
      let m = FreeMonoid::from(e);
      assert!(!m.is_mul_id());
      assert!(m != FreeMonoid::mul_id());
    }
  }

  #[test]
  fn from_produces_a_one_letter_word() {
    assert_eq!(FreeMonoid::from(E(0)).to_string(), "e0");
    assert_eq!(FreeMonoid::from(E(2)).to_string(), "e2");
    // The declared prefix and a multi-digit index are both preserved.
    assert_eq!(FreeMonoid::from(F(11)).to_string(), "f11");
  }

  #[test]
  fn multiplication_concatenates() {
    let mut x = word([E(0), E(1)]);
    x *= &word([E(2), E(0)]);
    assert_eq!(x.to_string(), "e0e1e2e0");
    assert!(x == word([E(0), E(1), E(2), E(0)]));
  }

  #[test]
  fn letters_may_repeat() {
    let x = word([E(1), E(1), E(1)]);
    assert_eq!(x.to_string(), "e1e1e1");
    assert!(x != word([E(1)]));
    assert!(x != word([E(1), E(1)]));
  }

  #[test]
  fn identity_laws_hold_on_both_sides() {
    for len in 0..4 {
      let letters: Vec<E> = E::elements().collect();
      let w = word(letters.into_iter().cycle().take(len));

      let mut left = FreeMonoid::mul_id();
      left *= &w;
      assert!(left == w);

      let mut right = w.clone();
      right *= &FreeMonoid::mul_id();
      assert!(right == w);
    }
  }

  #[test]
  fn multiplication_is_associative() {
    let a = word([E(0), E(2)]);
    let b = word([E(1)]);
    let c = word([E(2), E(2), E(0)]);

    let mut left = a.clone();
    left *= &b;
    left *= &c;

    let mut right = b.clone();
    right *= &c;
    let mut right_total = a.clone();
    right_total *= &right;

    assert!(left == right_total);
    assert_eq!(left.to_string(), "e0e2e1e2e2e0");
  }

  #[test]
  fn multiplication_is_not_commutative() {
    let mut ab = FreeMonoid::from(E(0));
    ab *= &FreeMonoid::from(E(1));

    let mut ba = FreeMonoid::from(E(1));
    ba *= &FreeMonoid::from(E(0));

    assert!(ab != ba);
    assert_eq!(ab.to_string(), "e0e1");
    assert_eq!(ba.to_string(), "e1e0");
  }

  #[test]
  fn the_monoid_is_free_of_relations() {
    // Distinct letter sequences are distinct elements: nothing collapses.
    let words: Vec<FreeMonoid<E>> = [
      vec![],
      vec![E(0)],
      vec![E(1)],
      vec![E(0), E(0)],
      vec![E(0), E(1)],
      vec![E(1), E(0)],
      vec![E(1), E(1)],
      vec![E(0), E(1), E(0)],
    ]
    .into_iter()
    .map(word)
    .collect();

    for (i, x) in words.iter().enumerate() {
      for (j, y) in words.iter().enumerate() {
        assert_eq!(i == j, x == y, "words {i} and {j} compared incorrectly");
      }
    }
  }

  #[test]
  fn product_of_identities_is_the_identity() {
    let mut x = FreeMonoid::<E>::mul_id();
    x *= &FreeMonoid::mul_id();
    assert!(x.is_mul_id());
    assert_eq!(x.to_string(), "");
  }

  #[test]
  fn multiplying_by_the_identity_preserves_is_mul_id() {
    let mut x = FreeMonoid::<E>::mul_id();
    assert!(x.is_mul_id());
    x *= &FreeMonoid::from(E(0));
    assert!(!x.is_mul_id());
    x *= &FreeMonoid::mul_id();
    assert!(!x.is_mul_id());
  }

  #[test]
  fn clones_are_independent() {
    let original = word([E(0), E(1)]);
    let mut copy = original.clone();
    copy *= &FreeMonoid::from(E(2));

    assert_eq!(original.to_string(), "e0e1");
    assert_eq!(copy.to_string(), "e0e1e2");
    assert!(original != copy);
  }

  #[test]
  fn squaring_a_word_doubles_it() {
    let w = word([E(0), E(1)]);
    let mut squared = w.clone();
    squared *= &w.clone();
    assert_eq!(squared.to_string(), "e0e1e0e1");
    assert!(squared == word([E(0), E(1), E(0), E(1)]));
  }

  #[test]
  fn powers_through_the_mul_monoid_alias() {
    let w = word([E(0), E(1)]);
    assert!(power(&w, 0).is_mul_id());
    assert!(power(&w, 1) == w);
    assert_eq!(power(&w, 3).to_string(), "e0e1e0e1e0e1");
    // Powers of the identity are the identity.
    assert!(power(&FreeMonoid::<E>::mul_id(), 5).is_mul_id());
  }

  #[test]
  fn display_of_a_long_word_is_the_concatenation_of_its_letters() {
    let letters: Vec<F> = (0..12).map(F).collect();
    let w = word(letters.iter().cloned());
    let expected: String = letters.iter().map(|f| f.to_string()).collect();
    assert_eq!(w.to_string(), expected);
    assert_eq!(w.to_string(), "f0f1f2f3f4f5f6f7f8f9f10f11");
  }

  #[test]
  fn free_monoid_over_an_empty_basis_has_only_the_identity() {
    assert_eq!(Empty::elements().count(), 0);
    let w = word(Empty::elements());
    assert!(w.is_mul_id());
    assert!(w == FreeMonoid::<Empty>::mul_id());
    assert_eq!(w.to_string(), "");
  }

  #[test]
  fn free_monoid_over_the_standard_basis() {
    let w = word(StandardBasis::<3>::elements());
    assert_eq!(w.to_string(), "e0e1e2");
    let mut doubled = w.clone();
    doubled *= &w;
    assert_eq!(doubled.to_string(), "e0e1e2e0e1e2");
    assert!(doubled != w);
  }

  #[test]
  fn into_rhs_is_reflexive() {
    let w = word([E(0), E(1)]);
    let rhs: &FreeMonoid<E> = w.into_rhs();
    assert!(std::ptr::eq(rhs, &w));
  }

  #[test]
  fn folding_a_product_matches_direct_construction() {
    let letters = [E(2), E(0), E(0), E(1)];
    let fold = |mut acc: FreeMonoid<E>, b: E| {
      acc *= &FreeMonoid::from(b);
      acc
    };
    let folded = letters.iter().cloned().fold(FreeMonoid::mul_id(), fold);
    assert!(folded == word(letters.iter().cloned()));
    assert_eq!(folded.to_string(), "e2e0e0e1");
  }

  // Compile-time checks that the trait aliases in this module are wired up.
  #[test]
  fn free_monoid_satisfies_the_multiplicative_aliases() {
    fn assert_mul_monoid<T: MulMonoid>() {}
    fn assert_mul_semigroup<T: MulSemigroup>() {}
    assert_mul_monoid::<FreeMonoid<E>>();
    assert_mul_monoid::<FreeMonoid<StandardBasis<5>>>();
    assert_mul_semigroup::<FreeMonoid<E>>();
  }

  #[test]
  fn primitives_and_strings_satisfy_the_additive_aliases() {
    fn assert_add_monoid<T: AddMonoid>() {}
    assert_add_monoid::<i32>();
    assert_add_monoid::<u8>();
    // `usize` is the last type in the `impl_primitive!` list in `lib.rs`;
    // regression guard for it being skipped by the marker-trait expansion.
    assert_add_monoid::<usize>();
    assert_add_monoid::<String>();
  }
}
