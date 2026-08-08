use std::collections::hash_map::Entry;
use std::collections::HashMap;
use std::fmt::Display;
use std::marker::PhantomData;

use crate as chalk;
use crate::chalk;
use crate::polynomial::Indeterminants;
use crate::polynomial::Monomial;
use crate::AddIdentity;
use crate::Binary;
use crate::CommutativeRing;
use crate::MulIdentity;
use crate::Ring;

/// A sparse, multivariate polynomial over a ring `R` in the indeterminates
/// described by `Vars`. This is what one would mathematically write as
/// `R[x, y, ...]`.
///
/// Whereas [`Polynomial`] is a dense, univariate polynomial backed by a `Vec`
/// of coefficients, `SparsePolynomial` is backed by a [`HashMap`] from
/// monomials to their (non-zero) coefficients. This makes it suitable for
/// polynomials in many indeterminates and/or of high degree, where the vast
/// majority of possible terms are absent.
///
/// The indeterminates are specified as a tuple of [`Indeterminant`] markers, so
/// `SparsePolynomial<R, (X, Y)>` is `R[x, y]`.
///
/// # Invariant
/// The backing map never contains a term whose coefficient is the additive
/// identity; the zero polynomial is represented by an empty map.
#[derive(Debug, Clone, Eq, PartialEq)]
#[chalk(
  |> Ring where R: Clone,
  |> CommutativeRing where R: Clone + CommutativeRing,
  |> Domain where R: Domain,
)]
pub struct SparsePolynomial<R: Ring, Vars: Indeterminants> {
  terms: HashMap<Monomial<Vars>, R>,
  vars: PhantomData<Vars>,
}

impl<R: Ring, Vars: Indeterminants> Binary for SparsePolynomial<R, Vars> {
  type Rhs = Self;
  fn into_rhs(&self) -> &Self {
    self
  }
}

impl<R: Ring, Vars: Indeterminants> From<R> for SparsePolynomial<R, Vars> {
  fn from(r: R) -> SparsePolynomial<R, Vars> {
    let mut terms = HashMap::new();
    if !r.is_add_id() {
      terms.insert(Monomial::mul_id(), r);
    }
    SparsePolynomial { terms, vars: PhantomData }
  }
}

impl<R: Ring, Vars: Indeterminants> SparsePolynomial<R, Vars> {
  /// The polynomial equal to the `index`-th indeterminate. Panics if `index`
  /// is not less than the number of indeterminates.
  pub fn var(index: usize) -> SparsePolynomial<R, Vars> {
    assert!(index < Vars::COUNT, "indeterminate index out of range");
    let mut monomial = Monomial::<Vars>::default();
    monomial.0[index] = 1;
    let mut terms = HashMap::new();
    terms.insert(monomial, R::mul_id());
    SparsePolynomial { terms, vars: PhantomData }
  }

  /// Builds a polynomial from a sequence of `(exponents, coefficient)` pairs.
  /// Repeated monomials have their coefficients summed. Each exponent vector
  /// must have exactly one entry per indeterminate, or this panics.
  pub fn from_terms<I: IntoIterator<Item = (Vec<usize>, R)>>(
    iter: I,
  ) -> SparsePolynomial<R, Vars> {
    let mut p = SparsePolynomial::default();
    for (exponents, coef) in iter {
      assert_eq!(
        exponents.len(),
        Vars::COUNT,
        "exponent vector length must equal the number of indeterminates"
      );
      let mut monomial = Monomial::<Vars>::default();
      for (i, exponent) in exponents.into_iter().enumerate() {
        monomial.0[i] = exponent;
      }
      match p.terms.get_mut(&monomial) {
        Some(c) => *c += coef.into_rhs(),
        None => {
          p.terms.insert(monomial, coef);
        }
      }
    }
    p.clean();
    p
  }

  /// The total degree of the polynomial, i.e. the largest total degree of any
  /// of its monomials, or `None` for the zero polynomial.
  pub fn degree(&self) -> Option<usize> {
    self.terms.keys().map(Monomial::degree).max()
  }

  /// Evaluates the polynomial, substituting `values[i]` for the `i`-th
  /// indeterminate. `values` must have at least one entry per indeterminate.
  pub fn substitute<T: Ring>(&self, values: &[T]) -> T
  where
    R: Binary<Rhs = <T as Binary>::Rhs>,
  {
    let mut result = T::add_id();
    for (monomial, coef) in self.terms.iter() {
      let mut term = T::mul_id();
      for (i, exponent) in monomial.0.into_iter().enumerate() {
        for _ in 0..exponent {
          term *= values[i].into_rhs();
        }
      }
      term *= coef.into_rhs();
      result += term.into_rhs();
    }
    result
  }

  /// Removes any terms whose coefficient is the additive identity, restoring
  /// the type's invariant.
  fn clean(&mut self) {
    self.terms.retain(|_, c| !c.is_add_id());
  }
}

impl<R: Ring, Vars: Indeterminants> Default for SparsePolynomial<R, Vars> {
  fn default() -> SparsePolynomial<R, Vars> {
    SparsePolynomial { terms: HashMap::new(), vars: PhantomData }
  }
}

impl<R: Ring, Vars: Indeterminants> AddIdentity for SparsePolynomial<R, Vars> {
  fn add_id() -> SparsePolynomial<R, Vars> {
    Default::default()
  }

  fn is_add_id(&self) -> bool {
    self.terms.is_empty()
  }
}

impl<R: Ring, Vars: Indeterminants> MulIdentity for SparsePolynomial<R, Vars> {
  fn mul_id() -> SparsePolynomial<R, Vars> {
    let mut terms = HashMap::new();
    terms.insert(Monomial::mul_id(), <R as MulIdentity>::mul_id());
    SparsePolynomial { terms, vars: PhantomData }
  }

  fn is_mul_id(&self) -> bool {
    let mut terms = self.terms.iter();
    match terms.next() {
      Some((monomial, coef)) => {
        terms.next().is_none()
          && monomial.degree() == 0
          && <R as MulIdentity>::is_mul_id(coef)
      }
      None => false,
    }
  }
}

impl<'a, R: Ring + Clone, Vars: Indeterminants>
  std::ops::AddAssign<&'a SparsePolynomial<R, Vars>>
  for SparsePolynomial<R, Vars>
{
  fn add_assign(&mut self, rhs: &SparsePolynomial<R, Vars>) {
    for (monomial, coef) in rhs.terms.iter() {
      match self.terms.entry(monomial.clone()) {
        Entry::Occupied(mut o) => {
          *o.get_mut() += coef.into_rhs();
          if o.get().is_add_id() {
            o.remove();
          }
        }
        Entry::Vacant(v) => {
          v.insert(coef.clone());
        }
      }
    }
  }
}

impl<'a, R: Ring + Clone, Vars: Indeterminants>
  std::ops::SubAssign<&'a SparsePolynomial<R, Vars>>
  for SparsePolynomial<R, Vars>
{
  fn sub_assign(&mut self, rhs: &SparsePolynomial<R, Vars>) {
    for (monomial, coef) in rhs.terms.iter() {
      match self.terms.entry(monomial.clone()) {
        Entry::Occupied(mut o) => {
          *o.get_mut() -= coef.into_rhs();
          if o.get().is_add_id() {
            o.remove();
          }
        }
        Entry::Vacant(v) => {
          v.insert(-coef.clone());
        }
      }
    }
  }
}

impl<'a, R: Ring + Clone, Vars: Indeterminants>
  std::ops::MulAssign<&'a SparsePolynomial<R, Vars>>
  for SparsePolynomial<R, Vars>
{
  fn mul_assign(&mut self, rhs: &SparsePolynomial<R, Vars>) {
    let lhs = std::mem::take(self);
    for (m1, c1) in lhs.terms.iter() {
      for (m2, c2) in rhs.terms.iter() {
        let mut monomial = m1.clone();
        monomial *= m2;
        let mut product = c1.clone();
        product *= c2.into_rhs();
        match self.terms.get_mut(&monomial) {
          Some(c) => *c += product.into_rhs(),
          None => {
            self.terms.insert(monomial, product);
          }
        }
      }
    }
    self.clean();
  }
}

impl<R: Ring + Display, Vars: Indeterminants> Display
  for SparsePolynomial<R, Vars>
{
  fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
    if self.is_add_id() {
      return write!(fmt, "0");
    }

    // Sort for a deterministic, human-readable ordering: highest total degree
    // first, breaking ties by exponent vector.
    let mut terms: Vec<_> = self.terms.iter().collect();
    terms.sort_by(|(a, _), (b, _)| {
      b.degree()
        .cmp(&a.degree())
        .then_with(|| <_ as Ord>::cmp(&b.0, &a.0))
    });

    let mut separator = "";
    for (monomial, coef) in terms {
      write!(fmt, "{separator}")?;
      let is_constant = monomial.degree() == 0;
      if coef.is_mul_id() && !is_constant {
        // Omit an implicit coefficient of `1` in front of a monomial.
      } else {
        write!(fmt, "{coef}")?;
        if !is_constant {
          write!(fmt, " * ")?;
        }
      }
      let mut var_separator = "";
      for (i, exponent) in monomial.0.into_iter().enumerate() {
        match exponent {
          0 => continue,
          1 => write!(fmt, "{var_separator}{}", Vars::display(i))?,
          _ => write!(fmt, "{var_separator}{}^{exponent}", Vars::display(i))?,
        }
        var_separator = " * ";
      }
      separator = " + ";
    }
    Ok(())
  }
}

#[cfg(test)]
mod tests {
  use super::*;
  use crate::indeterminants;

  indeterminants! {X: "x", Y: "y"}
  // `i32[x, y]`.
  type P = SparsePolynomial<i32, (X, Y)>;

  fn terms<const N: usize>(ts: [(Vec<usize>, i32); N]) -> P {
    P::from_terms(ts)
  }

  #[test]
  fn fmt() {
    assert_eq!(format!("{}", P::add_id()), "0");
    assert_eq!(format!("{}", terms([])), "0");
    assert_eq!(format!("{}", terms([(vec![0, 0], 0)])), "0");
    assert_eq!(format!("{}", terms([(vec![0, 0], 3)])), "3");
    assert_eq!(format!("{}", terms([(vec![0, 0], -1)])), "-1");
    assert_eq!(format!("{}", P::mul_id()), "1");

    assert_eq!(format!("{}", P::var(0)), "x");
    assert_eq!(format!("{}", P::var(1)), "y");
    assert_eq!(format!("{}", terms([(vec![1, 0], -1)])), "-1 * x");
    assert_eq!(format!("{}", terms([(vec![0, 1], 3)])), "3 * y");
    assert_eq!(format!("{}", terms([(vec![2, 0], 1)])), "x^2");
    assert_eq!(format!("{}", terms([(vec![2, 1], 1)])), "x^2 * y");
    assert_eq!(format!("{}", terms([(vec![2, 1], -3)])), "-3 * x^2 * y");

    // Higher total degree is printed first; ties break on the exponent vector.
    assert_eq!(
      format!("{}", terms([(vec![0, 1], 1), (vec![2, 0], 3)])),
      "3 * x^2 + y"
    );
    assert_eq!(
      format!("{}", terms([(vec![1, 1], 1), (vec![2, 0], 1)])),
      "x^2 + x * y"
    );
    assert_eq!(
      format!("{}", terms([(vec![0, 0], 1), (vec![2, 1], -1)])),
      "-1 * x^2 * y + 1"
    );
  }

  #[test]
  fn zero_coefficients_are_dropped() {
    assert_eq!(terms([(vec![1, 0], 0)]), P::add_id());
    assert_eq!(terms([(vec![1, 0], 1), (vec![1, 0], -1)]), P::add_id());
    assert!(terms([(vec![1, 0], 1), (vec![1, 0], -1)]).is_add_id());
  }

  #[test]
  fn identities() {
    assert!(P::add_id().is_add_id());
    assert!(!P::add_id().is_mul_id());
    assert!(P::mul_id().is_mul_id());
    assert!(!P::mul_id().is_add_id());
    assert_eq!(P::mul_id(), terms([(vec![0, 0], 1)]));
    assert_eq!(P::from(5), terms([(vec![0, 0], 5)]));
    assert_eq!(P::from(0), P::add_id());
    // A non-constant polynomial equal to `1` is not the multiplicative id.
    assert!(!terms([(vec![1, 0], 1)]).is_mul_id());
  }

  #[test]
  fn degree() {
    assert_eq!(P::add_id().degree(), None);
    assert_eq!(P::mul_id().degree(), Some(0));
    assert_eq!(P::var(0).degree(), Some(1));
    assert_eq!(terms([(vec![2, 1], 1), (vec![0, 1], 4)]).degree(), Some(3));
  }

  #[test]
  fn add() {
    assert_eq!(P::add_id() + &P::add_id(), P::add_id());
    assert_eq!(P::var(0) + &P::add_id(), P::var(0));
    assert_eq!(P::add_id() + &P::var(1), P::var(1));

    // x + y.
    assert_eq!(
      P::var(0) + &P::var(1),
      terms([(vec![1, 0], 1), (vec![0, 1], 1)])
    );

    // (x^2 + 2y) + (3x^2 - 2y) == 4x^2, with the y terms cancelling.
    assert_eq!(
      terms([(vec![2, 0], 1), (vec![0, 1], 2)])
        + &terms([(vec![2, 0], 3), (vec![0, 1], -2)]),
      terms([(vec![2, 0], 4)])
    );
  }

  #[test]
  fn sub() {
    assert_eq!(P::var(0) - &P::var(0), P::add_id());
    assert_eq!(P::add_id() - &P::var(1), terms([(vec![0, 1], -1)]));
    assert_eq!(
      terms([(vec![1, 0], 5), (vec![0, 1], 3)])
        - &terms([(vec![1, 0], 2), (vec![0, 0], 1)]),
      terms([(vec![1, 0], 3), (vec![0, 1], 3), (vec![0, 0], -1)])
    );
  }

  #[test]
  fn mul() {
    assert_eq!(P::var(0) * &P::add_id(), P::add_id());
    assert_eq!(P::var(0) * &P::mul_id(), P::var(0));

    // x * y == xy.
    assert_eq!(P::var(0) * &P::var(1), terms([(vec![1, 1], 1)]));

    // (x + y)(x - y) == x^2 - y^2.
    let x_plus_y = P::var(0) + &P::var(1);
    let x_minus_y = P::var(0) - &P::var(1);
    assert_eq!(
      x_plus_y * &x_minus_y,
      terms([(vec![2, 0], 1), (vec![0, 2], -1)])
    );

    // (x + y)^2 == x^2 + 2xy + y^2 (checks that like terms combine).
    let x_plus_y = P::var(0) + &P::var(1);
    assert_eq!(
      x_plus_y.clone() * &x_plus_y,
      terms([(vec![2, 0], 1), (vec![1, 1], 2), (vec![0, 2], 1)])
    );
  }

  #[test]
  fn substitute() {
    // x^2 * y + 1 evaluated at x = 2, y = 3 is 4 * 3 + 1 = 13.
    let p = terms([(vec![2, 1], 1), (vec![0, 0], 1)]);
    assert_eq!(p.substitute(&[2, 3]), 13);

    // The zero polynomial evaluates to 0.
    assert_eq!(P::add_id().substitute(&[7, 9]), 0);

    // A constant ignores the variable values.
    assert_eq!(P::from(5).substitute(&[100, 200]), 5);

    // Exponents are respected: x^3 at x = 2 (y unused) is 8.
    assert_eq!(terms([(vec![3, 0], 1)]).substitute(&[2, 5]), 8);

    // A sum of terms: 2x^2 + 3y at x = 2, y = 4 is 8 + 12 = 20.
    assert_eq!(
      terms([(vec![2, 0], 2), (vec![0, 1], 3)]).substitute(&[2, 4]),
      20
    );
  }

  #[test]
  fn from_terms_sums_repeated_monomials() {
    // The same monomial listed twice has its coefficients added.
    assert_eq!(
      terms([(vec![1, 0], 2), (vec![1, 0], 3)]),
      terms([(vec![1, 0], 5)])
    );
    // Mixing repeated and distinct monomials.
    assert_eq!(
      terms([(vec![1, 0], 1), (vec![0, 1], 1), (vec![1, 0], 4)]),
      terms([(vec![1, 0], 5), (vec![0, 1], 1)])
    );
  }

  #[test]
  fn mul_is_commutative_and_distributes() {
    let a = terms([(vec![1, 0], 2), (vec![0, 1], 1)]); // 2x + y
    let b = terms([(vec![1, 1], 1), (vec![0, 0], 3)]); // xy + 3
    // Commutativity: a * b == b * a.
    assert_eq!(a.clone() * &b, b.clone() * &a);
    // Distributivity: a * (b + a) == a*b + a*a.
    let lhs = a.clone() * &(b.clone() + &a);
    let rhs = (a.clone() * &b) + &(a.clone() * &a);
    assert_eq!(lhs, rhs);
  }

  #[test]
  fn mul_by_constant_scales_every_term() {
    let p = terms([(vec![2, 0], 1), (vec![0, 1], -3)]); // x^2 - 3y
    assert_eq!(
      p * &P::from(4),
      terms([(vec![2, 0], 4), (vec![0, 1], -12)])
    );
  }

  #[test]
  #[should_panic]
  fn var_out_of_range_panics() {
    // Only indeterminates 0 and 1 exist for `(X, Y)`.
    let _ = P::var(2);
  }

  #[test]
  #[should_panic]
  fn from_terms_wrong_exponent_length_panics() {
    // The exponent vector must have one entry per indeterminate.
    let _ = P::from_terms([(vec![1], 1)]);
  }
}
