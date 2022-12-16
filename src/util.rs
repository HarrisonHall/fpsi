use std::clone::Clone;
use std::ops::Deref;
use std::sync::{Arc, RwLock};

type ShareType<T> = Arc<RwLock<T>>;

pub struct Shareable<T>(ShareType<T>);

impl<T: 'static> Shareable<T> {
    pub fn new(s: T) -> Self {
        Shareable(Arc::new(RwLock::new(s)))
    }
}

impl<T: 'static> Deref for Shareable<T> {
    type Target = ShareType<T>;
    fn deref(&self) -> &Self::Target {
        return &self.0;
    }
}

impl<T: 'static> Clone for Shareable<T> {
    fn clone(&self) -> Self {
        return Shareable(self.0.clone());
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn make_shareable_int() {
        struct Foo {
            x: u8,
        }
        let val = Shareable::new(Foo { x: 2 });
        if let Ok(mut valy) = val.write() {
            valy.x *= 1;
        };
        if let Ok(valy) = val.read() {
            assert_eq!(valy.x, 2);
        };
    }
}
