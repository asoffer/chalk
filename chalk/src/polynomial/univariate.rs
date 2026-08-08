use std::fmt::Display;
use std::marker::PhantomData;

use crate as chalk;
use crate::chalk;
use crate::polynomial::Indeterminant;
use crate::AddIdentity;
use crate::Binary;
use crate::CommutativeRing;
use crate::MulIdentity;
use crate::Ring;

/// A univariate polynomial represented as a dense sequence of coefficients.
#[derive(Debug, Clone, Eq, PartialEq)]
#[chalk(
  |> Ring where R: Clone,
  |> CommutativeRing where R: Clone + CommutativeRing,
  |> Domain where R: Domain,
)]
pub struct UnivariatePolynomial<R: Ring, Var: Indeterminant> {
  coefficients: Vec<R>,
  var: PhantomData<Var>,
}

impl<R: Ring, Var: Indeterminant> Binary for UnivariatePolynomial<R, Var> {
  type Rhs = Self;
  fn into_rhs(&self) -> &Self {
    self
  }
}

impl<R: Ring, Var: Indeterminant> From<R> for UnivariatePolynomial<R, Var> {
  fn from(r: R) -> UnivariatePolynomial<R, Var> {
    UnivariatePolynomial::from_coefficients([r])
  }
}

impl<R: Ring, Var: Indeterminant> UnivariatePolynomial<R, Var> {
  pub fn var() -> UnivariatePolynomial<R, Var> {
    UnivariatePolynomial::from_coefficients([R::add_id(), R::mul_id()])
  }

  /// Constructs a polynomial where the coefficient of [`Var`]^n is `coefs[n]`.
  pub fn from_coefficients<V: Into<Vec<R>>>(
    coefs: V,
  ) -> UnivariatePolynomial<R, Var> {
    let mut p = UnivariatePolynomial {
      coefficients: coefs.into(),
      var: PhantomData,
    };
    p.clean();
    p
  }

  /// If the polynomial is zero, returns `None`. Otherwise returns `Some(d)`
  /// where `d` is the degree of the polynomial.
  pub fn degree(&self) -> Option<usize> {
    let n = self.coefficients.len();
    if n == 0 {
      None
    } else {
      Some(n - 1)
    }
  }

  /// Returns the element of the ring `T` constructed by evaluating the
  /// indeterminant at `t`.
  pub fn substitute<T: Ring>(&self, t: &T) -> T
  where
    R: Clone + Binary<Rhs = <T as Binary>::Rhs>,
  {
    let mut result = T::add_id();
    for coef in self.coefficients.iter().rev() {
      result *= t.into_rhs();
      result += coef.into_rhs();
    }
    result
  }

  fn clean(&mut self) {
    while self.coefficients.last().map(AddIdentity::is_add_id) == Some(true) {
      self.coefficients.pop();
    }
  }
}

impl<R: Ring + Display, Var: Indeterminant> Display
  for UnivariatePolynomial<R, Var>
{
  fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
    if self.is_add_id() {
      write!(fmt, "0")?;
    }

    let mut separator = "";
    for (i, c) in self.coefficients.iter().enumerate().rev() {
      if c.is_add_id() {
        continue;
      }
      if c.is_mul_id() {
        match i {
          0 => write!(fmt, "{separator}1")?,
          1 => write!(fmt, "{separator}{}", Var::display())?,
          _ => write!(fmt, "{separator}{}^{i}", Var::display())?,
        }
      } else {
        match i {
          0 => write!(fmt, "{separator}{c}")?,
          1 => write!(fmt, "{separator}{c} * {}", Var::display())?,
          _ => write!(fmt, "{separator}{c} * {}^{i}", Var::display())?,
        }
      }
      separator = " + ";
    }
    Ok(())
  }
}

impl<'a, R: Ring + Clone, Var: Indeterminant>
  std::ops::AddAssign<&'a UnivariatePolynomial<R, Var>>
  for UnivariatePolynomial<R, Var>
{
  fn add_assign(&mut self, rhs: &UnivariatePolynomial<R, Var>) {
    for (l, r) in self.coefficients.iter_mut().zip(rhs.coefficients.iter()) {
      *l += r.into_rhs();
    }
    let lhs_len = self.coefficients.len();
    if lhs_len < rhs.coefficients.len() {
      self
        .coefficients
        .extend(rhs.coefficients[lhs_len..].iter().cloned());
    }
    self.clean();
  }
}

impl<'a, R: Ring + Clone, Var: Indeterminant>
  std::ops::SubAssign<&'a UnivariatePolynomial<R, Var>>
  for UnivariatePolynomial<R, Var>
{
  fn sub_assign(&mut self, rhs: &UnivariatePolynomial<R, Var>) {
    for (l, r) in self.coefficients.iter_mut().zip(rhs.coefficients.iter()) {
      *l -= r.into_rhs();
    }
    let lhs_len = self.coefficients.len();
    if lhs_len < rhs.coefficients.len() {
      self
        .coefficients
        .extend(rhs.coefficients[lhs_len..].iter().map(|r| -r.clone()));
    }
    self.clean();
  }
}

impl<R: Ring, Var: Indeterminant> Default for UnivariatePolynomial<R, Var> {
  fn default() -> UnivariatePolynomial<R, Var> {
    UnivariatePolynomial {
      coefficients: vec![],
      var: std::marker::PhantomData,
    }
  }
}

impl<R: Ring, Var: Indeterminant> AddIdentity for UnivariatePolynomial<R, Var> {
  fn add_id() -> UnivariatePolynomial<R, Var> {
    Default::default()
  }

  fn is_add_id(&self) -> bool {
    self.degree().is_none()
  }
}

impl<R: Ring, Var: Indeterminant> MulIdentity for UnivariatePolynomial<R, Var> {
  fn mul_id() -> UnivariatePolynomial<R, Var> {
    UnivariatePolynomial {
      coefficients: vec![<R as MulIdentity>::mul_id()],
      var: std::marker::PhantomData,
    }
  }

  fn is_mul_id(&self) -> bool {
    self.degree() == Some(0)
      && <R as MulIdentity>::is_mul_id(&self.coefficients[0])
  }
}

impl<'a, R: Ring + Clone, Var: Indeterminant>
  std::ops::MulAssign<&'a UnivariatePolynomial<R, Var>>
  for UnivariatePolynomial<R, Var>
{
  fn mul_assign(&mut self, rhs: &UnivariatePolynomial<R, Var>) {
    let lhs = std::mem::take(self);
    self.coefficients.resize_with(
      lhs.coefficients.len() + rhs.coefficients.len() - 1,
      AddIdentity::add_id,
    );
    for (i, l) in lhs.coefficients.iter().enumerate() {
      for (j, r) in rhs.coefficients.iter().enumerate() {
        let mut product = l.clone();
        product *= r.into_rhs();
        self.coefficients[i + j] += product.into_rhs();
      }
    }
    self.clean();
  }
}

#[cfg(test)]
mod tests {
  use super::*;
  use crate::indeterminants;

  indeterminants! {X: "x", Y: "y"}
  // `i32[x]`.
  type P = UnivariatePolynomial<i32, X>;

  #[test]
  fn from_coefficients_cleans_trailing_zeros() {
    // Trailing additive-identity coefficients are stripped so that equal
    // polynomials have equal representations and `degree` is accurate.
    assert_eq!(P::from_coefficients([1, 2, 0, 0]).degree(), Some(1));
    assert_eq!(P::from_coefficients([0, 0, 0]).degree(), None);
    assert_eq!(P::from_coefficients([]).degree(), None);
    assert_eq!(
      P::from_coefficients([1, 2, 3, 0]),
      P::from_coefficients([1, 2, 3])
    );
    // Interior zeros are preserved.
    assert_eq!(P::from_coefficients([1, 0, 3]).degree(), Some(2));
  }

  #[test]
  fn fmt() {
    assert_eq!(format!("{}", P::from_coefficients([])), "0");
    assert_eq!(format!("{}", P::from_coefficients([0])), "0");
    assert_eq!(format!("{}", P::from_coefficients([0, 0, 0])), "0");
    assert_eq!(format!("{}", P::from_coefficients([1])), "1");
    assert_eq!(format!("{}", P::from_coefficients([-1])), "-1");
    assert_eq!(format!("{}", P::from_coefficients([3])), "3");
    assert_eq!(format!("{}", P::from_coefficients([0, 1])), "x");
    assert_eq!(format!("{}", P::from_coefficients([0, -1])), "-1 * x");
    assert_eq!(format!("{}", P::from_coefficients([0, 3])), "3 * x");
    assert_eq!(format!("{}", P::from_coefficients([0, 0, 1])), "x^2");
    assert_eq!(format!("{}", P::from_coefficients([0, 0, -1])), "-1 * x^2");
    assert_eq!(format!("{}", P::from_coefficients([1, 0, -1])), "-1 * x^2 + 1");
    assert_eq!(
      format!("{}", P::from_coefficients([1, 0, -1, 0])),
      "-1 * x^2 + 1"
    );
    // Highest-degree term is printed first, and zero coefficients are skipped.
    assert_eq!(
      format!("{}", P::from_coefficients([1, 2, 3])),
      "3 * x^2 + 2 * x + 1"
    );
    assert_eq!(
      format!("{}", P::from_coefficients([0, 0, 0, 1])),
      "x^3"
    );

    // The display name follows the indeterminate marker.
    assert_eq!(
      format!("{}", UnivariatePolynomial::<i32, Y>::from_coefficients([0, -1])),
      "-1 * y"
    );
  }

  #[test]
  fn degree() {
    assert_eq!(P::add_id().degree(), None);
    assert_eq!(P::from(0).degree(), None);
    assert_eq!(P::mul_id().degree(), Some(0));
    assert_eq!(P::from(5).degree(), Some(0));
    assert_eq!(P::var().degree(), Some(1));
    assert_eq!(P::from_coefficients([1, 2, 3]).degree(), Some(2));
  }

  #[test]
  fn identities() {
    assert!(P::add_id().is_add_id());
    assert!(!P::add_id().is_mul_id());
    assert!(P::mul_id().is_mul_id());
    assert!(!P::mul_id().is_add_id());

    assert_eq!(P::add_id(), P::from_coefficients([]));
    assert_eq!(P::add_id(), P::from(0));
    assert_eq!(P::mul_id(), P::from_coefficients([1]));
    assert_eq!(P::mul_id(), P::from(1));

    // A non-constant polynomial equal to 1 at x = ... is still not the
    // multiplicative identity.
    assert!(!P::from_coefficients([1, 1]).is_mul_id());
    // A constant other than 1 is not the multiplicative identity.
    assert!(!P::from(2).is_mul_id());
  }

  #[test]
  fn var_and_from() {
    // `var()` is the polynomial `x`.
    assert_eq!(P::var(), P::from_coefficients([0, 1]));
    // `from` builds a degree-zero constant.
    assert_eq!(P::from(7), P::from_coefficients([7]));
  }

  #[test]
  fn add() {
    assert_eq!(
      P::from_coefficients([]) + &P::from_coefficients([]),
      P::from_coefficients([])
    );
    assert_eq!(
      P::from_coefficients([1]) + &P::from_coefficients([]),
      P::from_coefficients([1])
    );
    assert_eq!(
      P::from_coefficients([]) + &P::from_coefficients([1]),
      P::from_coefficients([1])
    );
    // Leading terms can cancel, dropping the degree.
    assert_eq!(
      P::from_coefficients([-1]) + &P::from_coefficients([1]),
      P::from_coefficients([])
    );
    assert_eq!(
      P::from_coefficients([1, 2, 3]) + &P::from_coefficients([4, 5, 6]),
      P::from_coefficients([5, 7, 9])
    );
    assert_eq!(
      P::from_coefficients([1, 2, 3]) + &P::from_coefficients([4, -2, -3]),
      P::from_coefficients([5])
    );
    // Interior cancellation leaves interior zeros but keeps the leading term.
    assert_eq!(
      P::from_coefficients([1, 2, 3, 4])
        + &P::from_coefficients([4, -2, -3, 4]),
      P::from_coefficients([5, 0, 0, 8])
    );
    // Cancelling the leading term shortens the polynomial.
    assert_eq!(
      P::from_coefficients([1, 2, 3, 4]) + &P::from_coefficients([0, 0, 0, -4]),
      P::from_coefficients([1, 2, 3])
    );
    // Addition is commutative even when operands differ in length.
    assert_eq!(
      P::from_coefficients([0, 0, 0, -4]) + &P::from_coefficients([1, 2, 3, 4]),
      P::from_coefficients([1, 2, 3])
    );
    assert_eq!(
      P::from_coefficients([1]) + &P::from_coefficients([1, 2, 3, 4]),
      P::from_coefficients([2, 2, 3, 4])
    );
    assert_eq!(
      P::from_coefficients([1, 2, 3, 4]) + &P::from_coefficients([1]),
      P::from_coefficients([2, 2, 3, 4])
    );
  }

  #[test]
  fn sub() {
    assert_eq!(
      P::from_coefficients([5, 7, 9]) - &P::from_coefficients([1, 2, 3]),
      P::from_coefficients([4, 5, 6])
    );
    // Subtracting a polynomial from itself is zero.
    assert_eq!(
      P::from_coefficients([1, 2, 3]) - &P::from_coefficients([1, 2, 3]),
      P::add_id()
    );
    // The subtrahend is longer; the tail is negated.
    assert_eq!(
      P::from_coefficients([1]) - &P::from_coefficients([0, 0, 3]),
      P::from_coefficients([1, 0, -3])
    );
    assert_eq!(
      P::add_id() - &P::from_coefficients([1, 2]),
      P::from_coefficients([-1, -2])
    );
  }

  #[test]
  fn mul() {
    // Multiplying by zero gives zero.
    assert_eq!(P::var() * &P::add_id(), P::add_id());
    // Multiplying by one is the identity.
    assert_eq!(P::var() * &P::mul_id(), P::var());
    // Scaling by a constant.
    assert_eq!(
      P::from_coefficients([1, 2, 3]) * &P::from(2),
      P::from_coefficients([2, 4, 6])
    );
    // x * x == x^2.
    assert_eq!(P::var() * &P::var(), P::from_coefficients([0, 0, 1]));
    // (x + 1)(x - 1) == x^2 - 1.
    assert_eq!(
      P::from_coefficients([1, 1]) * &P::from_coefficients([-1, 1]),
      P::from_coefficients([-1, 0, 1])
    );
    // (1 + x)^2 == 1 + 2x + x^2 (like terms must accumulate, not overwrite).
    assert_eq!(
      P::from_coefficients([1, 1]) * &P::from_coefficients([1, 1]),
      P::from_coefficients([1, 2, 1])
    );
    // (1 + x + x^2)(1 + x) == 1 + 2x + 2x^2 + x^3.
    assert_eq!(
      P::from_coefficients([1, 1, 1]) * &P::from_coefficients([1, 1]),
      P::from_coefficients([1, 2, 2, 1])
    );
  }

  #[test]
  fn substitute() {
    // p(x) = 3x^2 + 2x + 1; p(2) = 12 + 4 + 1 = 17.
    let p = P::from_coefficients([1, 2, 3]);
    assert_eq!(p.substitute(&2), 17);
    // p(0) is the constant term.
    assert_eq!(p.substitute(&0), 1);
    // The zero polynomial evaluates to 0.
    assert_eq!(P::add_id().substitute(&5), 0);
    // A constant ignores the point of evaluation.
    assert_eq!(P::from(9).substitute(&123), 9);
    // The indeterminate evaluates to the point itself.
    assert_eq!(P::var().substitute(&42), 42);
  }
}
