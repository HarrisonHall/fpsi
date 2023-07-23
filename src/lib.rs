///! FPSI is a modular and nodal data aggregation system with support for
///! communication, states, and XXX.

/// FPSI node
pub mod node;
pub use node::*;

/// Data handling system.
pub mod data;
pub use data::*;

/// Event enum
pub mod event;
pub use event::*;

/// Prelude
pub mod prelude;
