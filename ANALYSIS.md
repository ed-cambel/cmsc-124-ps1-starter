1. Pick 3 of the 10 categories. For each, pick a language that gives it to you for free and say what that language pays for it. "Python has dictionaries" isn't an answer. What does Python's dictionary cost in memory or in speed compared to what you built, and where would you notice?

2. You wrote the tag check in dt_value_as_int by hand. Some languages don't let you. They make the tagged union a language construct, so the compiler writes the check for you, refuses to compile a read that skips it, and refuses to compile a set of cases that misses one. Rust's enum and match work this way, and so do ML's datatypes and Swift's enumerations with associated values. What does the C version let you do that a compiler enforcing the check wouldn't, and is any of it worth wanting?

Answer: In C, we are allowed to do type punning, which reads any union field regardless of current tag. We are also allowed to alter the payload memory without updating its tag so even if it leads to invalid dynamic types, the compiler still runs the code. By extension, it can also omit tag cases in conditional statements without alerting the programmer of an error. For general use, these features would not be worth wanting as it can lead to vulnerabilities in the system.

3. Your dt_map keeps insertion order separately from the hash buckets, which is memory spent on something no lookup uses. Argue the other side: describe a design that drops it, say what breaks, and say whether you'd ship it.

Answer: If we drop the insertion order, we can do so by eliminating order, capacity, and its reallocations. Doing so will end up with dt_map_key_at unable to return the keys in the intended insertion order. The key iteration order would end up becoming arbitrary through hash distribution and linked list traversal order. If this is the case, I'd rather not ship it as dt_map_key_at requires a certain iteration order for a stable output.

4. Compare access after release with an allocation that remains unreleased at the driver's final check. What damage can each cause in a long-running server? How does that answer change for a command-line tool that exits in a second?
