# FPSI
FPSI is a minimal rust library designed for handling data systems managed
by multiple nodes. The core program contains a system for data production,
data filtering/aggregation, and state management. The event loop requires all
possible data states/frames/sources and event handlers to be known at compile
time.

FPSI was designed abstract so I could use it for a couple of personal projects.
It is hard to know just what FPSI can be used for without an example. **When one
of my projects are sufficiently developed I will link it here.**

## Running

### Development
`cargo test`

### Examples
- `examples/rocket.rs`: `cargo run --example rocket`

## FAQ
> Future work?

Documentation, generic event handlers, more examples.
