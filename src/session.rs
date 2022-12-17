use std::sync::{Arc, RwLock};
use std::vec::Vec;

use crate::data;
use crate::plugin::Plugin;

pub struct Session {
    pub handler: data::Handler,
    plugins: Vec<Box<dyn Plugin>>,
    // TODO state registry
    // TODO communication
}

impl Session {
    pub fn new() -> Self {
        Session {
            handler: data::Handler::new(),
            plugins: Vec::new(),
        }
    }
    pub fn register_plugin<T: Plugin + 'static>(&mut self, plugin: T) {
        self.plugins.push(Box::new(plugin));
    }
}
