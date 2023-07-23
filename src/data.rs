///! Data
pub use std::clone::Clone;

pub use serde::de::DeserializeOwned;
pub use serde::{Deserialize, Serialize};

/// Time tick
pub type Tick = u64;

/// Source for data
pub trait Source: Serialize + DeserializeOwned + Send + Sync + Clone + 'static {}

/// Individual frame of data
pub trait Frame<So>: Serialize + DeserializeOwned + Send + Sync + Clone + 'static
where
    So: Source,
{
    /// Get source of frame
    fn source(&self) -> So;
}

/// System state
pub trait State: Serialize + DeserializeOwned + Clone + Send + Sync + 'static {}
