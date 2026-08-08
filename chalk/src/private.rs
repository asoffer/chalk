// `allow(dead_code)` because this may be unused depending on which features are
// implemented.
#[allow(dead_code)]
/// Used to make traits that can be refereneced but never implemented outside
/// this crate.
pub trait Sealed {}
