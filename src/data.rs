///! Data
pub use std::clone::Clone;
use std::sync::Arc;
use std::vec::Vec;

pub use serde::de::DeserializeOwned;
pub use serde::{Deserialize, Serialize};

/// Time tick
pub type Tick = u64;

pub type FrameVec<T> = Arc<Vec<Arc<T>>>;

/// Source for data
pub trait Source: Serialize + DeserializeOwned + Send + Sync + Clone + 'static {}

/// Individual frame of data
pub trait Frame<So>: Serialize + DeserializeOwned + Send + Sync + Clone + 'static
where
    So: Source,
{
    // Get tick of frame
    // fn tick(&self) -> Tick;
    /// Get source of frame
    fn source(&self) -> So;
}

/// System state
pub trait State: Serialize + DeserializeOwned + Clone + Send + Sync + 'static {
    // Get tick of state
    // fn tick(&self) -> Tick;
}
