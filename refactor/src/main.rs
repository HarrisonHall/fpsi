extern crate pretty_env_logger;
#[macro_use]
extern crate log;

use std::env;

mod data;
//use crate::data;
//extern crate data;
//use crate::data;
//use crate::data::data;

fn main() {
    // Initialize logging
    env::set_var("RUST_LOG", "trace"); // Log with everything enabled
    pretty_env_logger::init(); // Initialize logger

    info!("FPSI initialized");

    let mut d = data::DataFrame::new();
    d.set_id(25);
    let new_source: String = String::from("Baz");
    d.set_source(new_source);
    info!("d has id {} source `{}`", d.get_id(), d.get_source());

    let mut d2 = d.clone();
    info!("d2 has id {} source `{}`", d2.get_id(), d2.get_source());

    let mut d3: Box<data::DataFrame> = Box::new(data::DataFrame::new());
    info!("Box has id {}", d3.get_id());
    d3.set_id(5);
}
