pub mod data;
pub mod plugin;
pub mod session;
pub mod util;

#[cfg(test)]
mod tests {
    use super::*;

    struct PseudoPlug {
        x: u8,
    }
    impl plugin::Plugin for PseudoPlug {}

    #[test]
    fn data_structures() {
        let sess = util::Shareable::new(session::Session::new());
        if let Ok(mut sess) = sess.write() {
            sess.register_plugin(PseudoPlug { x: 0 });
            sess.handler.create_source("foo");
            if let Some(foo) = sess.handler.get_source("foo") {
                if let Ok(mut foo) = foo.write() {
                    foo.track_raw(data::Frame::new());
                }
            }
        };
    }
}
