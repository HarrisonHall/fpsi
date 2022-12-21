//! The communication module is used to allow fpsi nodes to communicate in a
//! graph-like network. This implemention here is naive, I will accept PRs with
//! more advanced solutions if they are still reasonably simple and lightweight.

use std::collections::HashMap;

use serde_json;

use crate::config::Config;
use crate::event::Event;

/// CommMessage enumerates all possible communication types along the fpsi
/// network.
/// The `ttl` field represents a time-to-live value which prevents message
/// cycling along the network.
#[derive(Clone)]
pub enum CommMessage {
    /// Node network initialization message. FPSI plugins should
    /// generate this manually after establishing communications
    /// with a client. After the event handler receives this message,
    /// the connection is noted and the event can be broadcasted out
    /// to all nodes not in {origin, last_from}.
    Init {
        /// The original node that sent the init message, announcing its existence.
        origin: String,
        /// The last node that sent this message.
        last_from: String,
        /// Time-to-live
        ttl: u8,
    },
    /// Plugins should create this comm event when a message needs to be sent
    /// to another node. Plugins will receive this message when the message
    /// is to be sent to another node. A `to` field containing "*" indicates
    /// that the message should be broadcasted (ignoring sending to the neighbor
    /// with the name `last_from`).
    Send {
        /// Name of (final) recipient fpsi node
        to: String,
        /// Name of next recipient in network
        next_to: String,
        /// The last node that sent this message.
        last_from: String,
        /// Time-to-live
        ttl: u8,
        /// Structured or unstructured serde::Value containing arbitrary data
        message: serde_json::Value,
    },
    /// Plugins should create this comm event when a message is received from a
    /// neighbor. Plugins will receive this message if the `to` field matches our
    /// node name, otherwise plugins will receive a `Send` message to forward to
    /// the relevant neighbor.
    Recv {
        /// Name of recipient fpsi node
        to: String,
        /// The last node that sent this message.
        last_from: String,
        /// Time-to-live
        ttl: u8,
        /// Structured or unstructured serde::Value containing arbitrary data
        message: serde_json::Value,
    },
}

/// Communication state-machine wrapper.
/// Holds information about pathing towards neighbors.
pub struct CommController {
    /// Name of our comm node
    node: String,
    /// Maps destination node names to nearest neighbor node
    neighbor_cache: HashMap<String, String>,
}

impl CommController {
    /// Create empty CommController
    pub fn new(config: &Config) -> Self {
        CommController {
            node: config.node_name.clone(),
            neighbor_cache: HashMap::new(),
        }
    }
    /// Filter Send and Recv messages
    /// Message is expectected to be a Send or Recv
    pub fn filter(&mut self, comm_message: &CommMessage) -> Event {
        match comm_message {
            CommMessage::Init {
                origin,
                last_from,
                ttl,
            } => {
                // If origin is us, forward
                if *origin == self.node {
                    return Event::Communication(CommMessage::Init {
                        origin: origin.clone(),
                        last_from: last_from.clone(),
                        ttl: ttl.clone(),
                    });
                }
                // Update neighbor cache and if ttl >= 1, forward
                self.neighbor_cache
                    .insert(origin.clone(), last_from.clone());
                if *ttl >= 1 {
                    return Event::Communication(CommMessage::Init {
                        origin: origin.clone(),
                        last_from: self.node.clone(),
                        ttl: ttl - 1,
                    });
                }
                // Else, None, we're done
                Event::None
            }
            CommMessage::Send {
                to,
                next_to: _next_to,
                last_from,
                ttl,
                message,
            } => {
                // This _shouldn't_ happen under normal circumstances, but I guess
                // I don't see a reason we _shouldn't_ allow it :shrug:
                if *to == self.node {
                    return Event::Communication(CommMessage::Recv {
                        to: to.clone(),
                        last_from: last_from.clone(),
                        ttl: 0,
                        message: message.clone(),
                    });
                }
                // If we have a possible neighbor, send it!
                return match self.neighbor_cache.get(to) {
                    Some(neighbor) => Event::Communication(CommMessage::Send {
                        to: to.clone(),
                        next_to: neighbor.clone(),
                        last_from: self.node.clone(),
                        ttl: ttl - 1,
                        message: message.clone(),
                    }),
                    None => Event::None,
                };
            }
            CommMessage::Recv {
                to,
                last_from,
                ttl,
                message,
            } => {
                // If message is to us, return message
                if *to == self.node {
                    return Event::Communication(CommMessage::Recv {
                        to: to.clone(),
                        last_from: last_from.clone(),
                        ttl: 0,
                        message: message.clone(),
                    });
                }
                // If ttl == 0, die
                if *ttl == 0 {
                    return Event::None;
                }
                // If we have a neighbor, send out, otherwise, don't
                return match self.neighbor_cache.get(to) {
                    Some(neighbor) => Event::Communication(CommMessage::Send {
                        to: to.clone(),
                        next_to: neighbor.clone(),
                        last_from: self.node.clone(),
                        ttl: ttl - 1,
                        message: message.clone(),
                    }),
                    None => Event::None,
                };
            }
        }
    }
}
