use std::vec::Vec;

use crate::data;
use crate::plugin;

use crate::util::Shareable;

struct Session {
    handler: data::Handler,
    plugins: Vec<Box<dyn plugin::Plugin>>,
    // TODO state registry
    // TODO communication
}

impl Session {}
