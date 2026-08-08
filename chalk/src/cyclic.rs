use crate as chalk;
use crate::chalk;
use crate::AddIdentity;
use crate::Binary;
use crate::MulIdentity;

#[derive(Copy, Clone, Debug, Default, Eq, PartialEq)]
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
    let mut z = ZMod(n);
    z.reduce();
    z
  }
}

impl<const N: usize> ZMod<N> {
  fn reduce(&mut self) {
    match const { Algorithm::choose(N, 1) } {
      Algorithm::Add(exponent, power) => {
        // `power == 2^exponent` and `2^exponent ≡ 1 (mod N)`, so folding the
        // high bits onto the low bits preserves the value modulo `N`. Repeat
        // until the value is below `power`, then correct the remainder.
        while self.0 >= power {
          self.0 = (self.0 >> exponent) + (self.0 & (power - 1))
        }
        while self.0 >= N {
          self.0 -= N;
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

#[cfg(test)]
mod tests {
  use super::*;

  fn z<const N: usize>(n: usize) -> ZMod<N> {
    ZMod::from(n)
  }

  #[test]
  fn from_reduces_into_canonical_range() {
    // `7` is a Mersenne modulus (2^3 - 1), exercising the fold-based reduction.
    assert_eq!(z::<7>(7), z::<7>(0));
    assert_eq!(z::<7>(8), z::<7>(1));
    assert_eq!(z::<7>(36), z::<7>(1));
    assert_eq!(z::<7>(100), z::<7>(2));
    // `10` is even, exercising the `%` reduction path.
    assert_eq!(z::<10>(10), z::<10>(0));
    assert_eq!(z::<10>(37), z::<10>(7));
    assert_eq!(z::<10>(123), z::<10>(3));
    // `5` divides 2^4 - 1 but is not itself Mersenne; fold then correct.
    assert_eq!(z::<5>(24), z::<5>(4));
    assert_eq!(z::<5>(36), z::<5>(1));
  }

  #[test]
  fn identities() {
    assert_eq!(ZMod::<7>::add_id(), z::<7>(0));
    assert_eq!(ZMod::<7>::mul_id(), z::<7>(1));
    assert!(ZMod::<7>::add_id().is_add_id());
    assert!(ZMod::<7>::mul_id().is_mul_id());
  }

  #[test]
  fn add_wraps_modulo_n() {
    // Mersenne modulus.
    assert_eq!(z::<7>(3) + &z::<7>(2), z::<7>(5));
    assert_eq!(z::<7>(5) + &z::<7>(4), z::<7>(2)); // 9 ≡ 2
    assert_eq!(z::<7>(6) + &z::<7>(6), z::<7>(5)); // 12 ≡ 5
    assert_eq!(z::<7>(6) + &z::<7>(1), z::<7>(0)); // 7 ≡ 0
    // Modulo path.
    assert_eq!(z::<10>(7) + &z::<10>(8), z::<10>(5)); // 15 ≡ 5
    // Adding the additive identity is a no-op.
    assert_eq!(z::<7>(4) + &ZMod::<7>::add_id(), z::<7>(4));
  }

  #[test]
  fn sub_wraps_modulo_n() {
    assert_eq!(z::<7>(5) - &z::<7>(3), z::<7>(2));
    assert_eq!(z::<7>(3) - &z::<7>(5), z::<7>(5)); // -2 ≡ 5
    assert_eq!(z::<7>(0) - &z::<7>(1), z::<7>(6)); // -1 ≡ 6
    assert_eq!(z::<10>(2) - &z::<10>(7), z::<10>(5)); // -5 ≡ 5
    // x - x == 0.
    assert_eq!(z::<7>(4) - &z::<7>(4), ZMod::<7>::add_id());
  }

  #[test]
  fn mul_wraps_modulo_n() {
    assert_eq!(z::<7>(3) * &z::<7>(3), z::<7>(2)); // 9 ≡ 2
    assert_eq!(z::<7>(6) * &z::<7>(6), z::<7>(1)); // 36 ≡ 1
    assert_eq!(z::<7>(4) * &z::<7>(5), z::<7>(6)); // 20 ≡ 6
    assert_eq!(z::<10>(6) * &z::<10>(7), z::<10>(2)); // 42 ≡ 2
    // Multiplying by the multiplicative identity is a no-op.
    assert_eq!(z::<7>(5) * &ZMod::<7>::mul_id(), z::<7>(5));
    // Multiplying by zero yields zero.
    assert_eq!(z::<7>(5) * &ZMod::<7>::add_id(), ZMod::<7>::add_id());
  }

  #[test]
  fn ring_axioms_hold_over_all_residues() {
    // Exhaustively check distributivity and commutativity for Z/7.
    for a in 0..7 {
      for b in 0..7 {
        assert_eq!(z::<7>(a) + &z::<7>(b), z::<7>(b) + &z::<7>(a));
        assert_eq!(z::<7>(a) * &z::<7>(b), z::<7>(b) * &z::<7>(a));
        for c in 0..7 {
          // a * (b + c) == a*b + a*c.
          let lhs = z::<7>(a) * &(z::<7>(b) + &z::<7>(c));
          let rhs = (z::<7>(a) * &z::<7>(b)) + &(z::<7>(a) * &z::<7>(c));
          assert_eq!(lhs, rhs);
        }
      }
    }
  }
}
