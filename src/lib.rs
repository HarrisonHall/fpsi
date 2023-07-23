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

#[cfg(test)]
mod tests {
    use super::data::*;

    enum TestSource {
        TestA,
        TestB,
    }

    #[derive(Serialize, Deserialize, Clone)]
    struct Foo {
        val: u32,
    }

    impl Foo {
        fn new(val: u32) -> Self {
            Self { val }
        }
    }

    impl<'a> Frame<'a, TestSource> for Foo {
        fn tick() -> Tick {
            0
        }

        fn source() -> TestSource {
            TestSource::TestA
        }
    }

    #[test]
    fn it_works() {
        let foo: Foo = Foo::new(32);
        assert!(foo.val == 32);
    }
}
