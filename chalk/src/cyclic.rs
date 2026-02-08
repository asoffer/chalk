use crate as chalk;
use crate::chalk;
use crate::AddIdentity;
use crate::Binary;
use crate::MulIdentity;

#[derive(Copy, Clone, Default, Eq, PartialEq)]
#[chalk(
  |> Ring
)]
pub struct ZMod<const N: usize>(usize);

impl<const N: usize> Binary for ZMod<N> {
  type Rhs = Self;
  fn into_rhs(&self) -> &Self {
    self
  }
}

impl<const N: usize> From<usize> for ZMod<N> {
  fn from(n: usize) -> ZMod<N> {
    ZMod(n)
  }
}

impl<const N: usize> ZMod<N> {
  fn reduce(&mut self) {
    match const { Algorithm::choose(N, 0) } {
      Algorithm::Add(exponent, power) => {
        if self.0 >= power {
          self.0 = (self.0 >> exponent) + (self.0 & (power - 1))
        }
      }
      Algorithm::Mod => {
        self.0 %= N;
      }
    }
  }
}

enum Algorithm {
  Add(usize, usize),
  Mod,
}

impl Algorithm {
  const fn choose(n: usize, i: usize) -> Algorithm {
    if i == 32 {
      Algorithm::Mod
    } else if (1 << i) % n == 1 {
      Algorithm::Add(i, 1 << i)
    } else {
      Algorithm::choose(n, i + 1)
    }
  }
}

impl<'a, const N: usize> std::ops::AddAssign<&'a ZMod<N>> for ZMod<N> {
  fn add_assign(&mut self, ZMod(rhs): &'a ZMod<N>) {
    self.0 += rhs;
    self.reduce()
  }
}

impl<'a, const N: usize> std::ops::SubAssign<&'a ZMod<N>> for ZMod<N> {
  fn sub_assign(&mut self, ZMod(rhs): &'a ZMod<N>) {
    self.0 += N - rhs;
    self.reduce()
  }
}

impl<'a, const N: usize> std::ops::MulAssign<&'a ZMod<N>> for ZMod<N> {
  fn mul_assign(&mut self, ZMod(rhs): &'a ZMod<N>) {
    self.0 *= rhs;
    self.reduce()
  }
}

impl<const N: usize> AddIdentity for ZMod<N> {
  fn add_id() -> ZMod<N> {
    Default::default()
  }
}

impl<const N: usize> MulIdentity for ZMod<N> {
  fn mul_id() -> ZMod<N> {
    ZMod(1)
  }
}
