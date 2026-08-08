use crate::chalk;
use crate::polynomial::indeterminant::Indeterminants;
use crate::Binary;
use crate::MulIdentity;

/// A monomial consisting of the [`Indeterminant`]s in `Vars`.
#[derive(Debug, Default, Eq, PartialEq)]
#[chalk(
  |> MulMonoid,
)]
pub struct Monomial<Vars: Indeterminants>(pub(crate) Vars::Array);

impl<Vars: Indeterminants> Clone for Monomial<Vars> {
  fn clone(&self) -> Self {
    Monomial(self.0.clone())
  }
}

impl<Vars: Indeterminants> std::hash::Hash for Monomial<Vars> {
  fn hash<H: std::hash::Hasher>(&self, h: &mut H) {
    self.0.hash(h)
  }
}

impl<Vars: Indeterminants> MulIdentity for Monomial<Vars> {
  fn mul_id() -> Self {
    Default::default()
  }
}

impl<Vars: Indeterminants> Binary for Monomial<Vars> {
  type Rhs = Self;
  fn into_rhs(&self) -> &Self {
    self
  }
}

impl<'a, Vars: Indeterminants> std::ops::MulAssign<&'a Monomial<Vars>>
  for Monomial<Vars>
{
  fn mul_assign(&mut self, rhs: &Monomial<Vars>) {
    for (i, exp) in rhs.0.into_iter().enumerate() {
      self.0[i] += exp;
    }
  }
}

impl<Vars: Indeterminants> Monomial<Vars> {
  pub fn degree(&self) -> usize {
    self.0.into_iter().sum()
  }
}

#[cfg(test)]
mod tests {
  use super::*;
  use crate::indeterminants;

  indeterminants! {X: "x", Y: "y"}
  type M = Monomial<(X, Y)>;

  #[test]
  fn default_is_the_multiplicative_identity() {
    assert_eq!(M::default(), M::mul_id());
    assert_eq!(M::default().degree(), 0);
    assert!(M::mul_id().is_mul_id());
  }

  #[test]
  fn degree_sums_exponents() {
    assert_eq!(Monomial::<(X, Y)>([0, 0]).degree(), 0);
    assert_eq!(Monomial::<(X, Y)>([2, 0]).degree(), 2);
    assert_eq!(Monomial::<(X, Y)>([2, 1]).degree(), 3);
    assert_eq!(Monomial::<(X, Y)>([3, 4]).degree(), 7);
  }

  #[test]
  fn is_mul_id_only_for_the_empty_monomial() {
    assert!(Monomial::<(X, Y)>([0, 0]).is_mul_id());
    assert!(!Monomial::<(X, Y)>([1, 0]).is_mul_id());
    assert!(!Monomial::<(X, Y)>([0, 1]).is_mul_id());
  }

  #[test]
  fn mul_adds_exponents_componentwise() {
    // (x^2)(y^3) == x^2 y^3.
    let mut m = Monomial::<(X, Y)>([2, 0]);
    m *= &Monomial::<(X, Y)>([0, 3]);
    assert_eq!(m, Monomial::<(X, Y)>([2, 3]));
    assert_eq!(m.degree(), 5);

    // Multiplying by the identity leaves the monomial unchanged.
    let mut m = Monomial::<(X, Y)>([1, 4]);
    m *= &M::mul_id();
    assert_eq!(m, Monomial::<(X, Y)>([1, 4]));
  }
}
