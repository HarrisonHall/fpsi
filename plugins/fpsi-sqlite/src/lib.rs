use std::fmt;
use std::sync::Arc;

use fpsi::data;
use fpsi::event::Event;
use fpsi::plugin::Plugin;
pub use fpsi::session::{Sender, Session};

use diesel::prelude::*;

pub struct SQLitePlugin {}

impl SQLitePlugin {
    fn new(database_file: &str) -> Self {
        let sql_conn = SqliteConnection::establish(":memory:").unwrap();
        diesel::insert_into()
        SQLitePlugin {}
    }
}

impl Plugin for SQLitePlugin {
    fn load(&mut self) -> Result<(), String> {
        Ok(())
    }
}

impl fmt::Display for SQLitePlugin {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "<SQLitePlugin ...>")
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use fpsi::plugin::*;
    use std::sync::atomic;

    #[derive(Debug)]
    struct Killer {
        count: atomic::AtomicU8,
    }

    impl Plugin for Killer {
        fn pre_agg(
            &self,
            _agg_frames: Arc<Vec<data::Frame>>,
            event_producer: Sender<Event>,
        ) -> Result<(), String> {
            let count = self.count.fetch_add(1, atomic::Ordering::Relaxed);
            // Kill session on 3rd pre_agg loop
            if count > 3 {
                event_producer.send(Event::Die).ok();
            }
            Ok(())
        }
    }

    impl fmt::Display for Killer {
        fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
            write!(f, "<Killer>")
        }
    }

    #[test]
    fn db_test() {
        let mut session = Session::new(None);
        session.register_plugin(SQLitePlugin::new("test.foo"));
        session.register_plugin(Killer {
            count: atomic::AtomicU8::new(0),
        });
        session.run();
    }
}
