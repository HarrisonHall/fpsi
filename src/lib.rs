///! FPSI is a modular and nodal data aggregation system with support for
///! communication, states, and XXX.

/// Session configuration system
pub mod config;

/// Data handling system.
pub mod data;

/// Event enum
pub mod event;

// Plugin trait for creating your own plugins.
pub mod plugin;

/// Session object. This is required to start the FPSI system.
pub mod session;

/// Misc. FPSI utility
pub mod util;
