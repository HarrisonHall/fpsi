///! Data
pub use std::clone::Clone;

pub use serde::{Deserialize, Serialize};

/// Time tick
pub type Tick = u64;

/// Individual frame of data
pub trait Frame<'a, Source>: Serialize + Deserialize<'a> + Clone {
    /// Get tick of frame
    fn tick() -> Tick;
    /// Get source of frame
    fn source() -> Source;
}

/// System state
pub trait State<'a>: Serialize + Deserialize<'a> + Clone {
    /// Get tick of state
    fn tick() -> Tick;
}
