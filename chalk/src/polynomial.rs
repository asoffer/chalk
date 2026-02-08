use std::fmt::Display;
use std::marker::PhantomData;

use crate as chalk;
use crate::chalk;
use crate::AddIdentity;
use crate::Binary;
use crate::CommutativeRing;
use crate::MulIdentity;
use crate::Ring;

pub trait Indeterminant: Eq {
  fn display() -> &'static str;
}

#[macro_export]
macro_rules! indeterminants {
  ($($i:ident: $s:literal),*) => {
    $(#[derive(Clone, Debug, Eq, PartialEq)]
    struct $i;
    impl $crate::polynomial::Indeterminant for $i {
      fn display() -> &'static str {
        $s
      }
    })*
  };
}

#[derive(Debug, Clone, Eq, PartialEq)]
#[chalk(
  |> Ring where R: Clone,
  |> CommutativeRing where R: Clone + CommutativeRing,
  |> Domain where R: Domain,
)]
pub struct Polynomial<R: Ring, Var: Indeterminant> {
  coefficients: Vec<R>,
  var: PhantomData<Var>,
}

impl<R: Ring, Var: Indeterminant> Binary for Polynomial<R, Var> {
  type Rhs = Self;
  fn into_rhs(&self) -> &Self {
    self
  }
}

impl<R: Ring, Var: Indeterminant> From<R> for Polynomial<R, Var> {
  fn from(r: R) -> Polynomial<R, Var> {
    Polynomial {
      coefficients: vec![r.into()],
      var: PhantomData,
    }
  }
}

impl<R: Ring, Var: Indeterminant> Polynomial<R, Var> {
  pub fn var() -> Polynomial<R, Var> {
    Polynomial::from_coefficients([0, 1])
  }

  pub fn from_coefficients<V: Into<Vec<R>>>(coefs: V) -> Polynomial<R, Var> {
    let mut p = Polynomial {
      coefficients: coefs.into(),
      var: PhantomData,
    };
    p.clean();
    p
  }

  pub fn degree(&self) -> Option<usize> {
    let n = self.coefficients.len();
    if n == 0 {
      None
    } else {
      Some(n - 1)
    }
  }

  pub fn substitute<T: Ring>(&self, t: &T) -> T
  where
    R: Clone + Binary<Rhs = <T as Binary>::Rhs>,
  {
    let mut result = T::mul_id();
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

impl<R: Ring + Display, Var: Indeterminant> Display for Polynomial<R, Var> {
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
  std::ops::AddAssign<&'a Polynomial<R, Var>> for Polynomial<R, Var>
{
  fn add_assign(&mut self, rhs: &Polynomial<R, Var>) {
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
  std::ops::SubAssign<&'a Polynomial<R, Var>> for Polynomial<R, Var>
{
  fn sub_assign(&mut self, rhs: &Polynomial<R, Var>) {
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

impl<R: Ring, Var: Indeterminant> Default for Polynomial<R, Var> {
  fn default() -> Polynomial<R, Var> {
    Polynomial {
      coefficients: vec![],
      var: std::marker::PhantomData,
    }
  }
}

impl<R: Ring, Var: Indeterminant> AddIdentity for Polynomial<R, Var> {
  fn add_id() -> Polynomial<R, Var> {
    Default::default()
  }

  fn is_add_id(&self) -> bool {
    self.degree().is_none()
  }
}

impl<R: Ring, Var: Indeterminant> MulIdentity for Polynomial<R, Var> {
  fn mul_id() -> Polynomial<R, Var> {
    Polynomial {
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
  std::ops::MulAssign<&'a Polynomial<R, Var>> for Polynomial<R, Var>
{
  fn mul_assign(&mut self, rhs: &Polynomial<R, Var>) {
    let lhs = std::mem::take(self);
    self.coefficients.resize_with(
      lhs.coefficients.len() + rhs.coefficients.len() - 1,
      AddIdentity::add_id,
    );
    for (i, l) in lhs.coefficients.iter().enumerate() {
      for (j, r) in rhs.coefficients.iter().enumerate() {
        let c = &mut self.coefficients[i + j];
        *c = l.clone();
        *c *= r.into_rhs();
      }
    }
    self.clean();
  }
}

#[cfg(test)]
mod tests {
  use super::*;

  indeterminants! {X: "x", Y: "y"}
  type P = Polynomial<i32, X>;

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

    assert_eq!(
      format!("{}", Polynomial::<i32, Y>::from_coefficients([0, -1])),
      "-1 * y"
    );
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

    assert_eq!(
      P::from_coefficients([1, 2, 3, 4])
        + &P::from_coefficients([4, -2, -3, 4]),
      P::from_coefficients([5, 0, 0, 8])
    );

    assert_eq!(
      P::from_coefficients([1, 2, 3, 4]) + &P::from_coefficients([0, 0, 0, -4]),
      P::from_coefficients([1, 2, 3])
    );

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
}
