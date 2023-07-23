///! Data
pub use std::clone::Clone;

use serde::de::DeserializeOwned;
use serde::Serialize;

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
