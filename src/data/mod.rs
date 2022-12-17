mod frame;
mod handler;
mod source;

pub use frame::*;
pub use handler::*;
pub use source::*;

use crate::util::Shareable;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn data_structures() {
        let mut handler = Handler::new();
        handler.create_source("foo");
        if let Some(source) = handler.get_source("foo") {
            if let Ok(mut source) = source.write() {
                source.track_raw(Frame::new());
            }
        }
    }
}
