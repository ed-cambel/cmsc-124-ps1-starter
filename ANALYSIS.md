### Question 1: Pick 3 of the 10 categories. For each, pick a language that gives it to you for free and say what that language pays for it. "Python has dictionaries" isn't an answer. What does Python's dictionary cost in memory or in speed compared to what you built, and where would you notice?

**Int: int (Python)**

While C also has built-in integer types such as int and long, it does not have the built-in arbitrary-precision integer that Python has. IArbitrary-precision limits the size of a value by the total available memory. In C, an integer has a fixed number of bits so we have to account for any overflows such as unsigned values wrapping around or signed overflow being undefined behavior in the module because of C's nature. Guards and checks were put in place like comparing length to `SIZE_MAX / sizeof(dt_value)` before multiplying in `dt_array_new` before any operations to prevent the previously mentioned issues.

As for Python’s tradeoff, it runs slower since it relies on software algorithms rather than direct processor instructions. Memory-wise, it uses more as each integer is a separate heap object Although, it does redeem itself by not having overflow bugs and needing none of the guards. You would notice its cost when we declare large amounts of integers.

**Tuple: tuple (Python)**

Tuple is one of Python’s core built-in collection data types. Compared to the module, it does not need declaration nor allocation. Moreover, it has no limit size. However, it is not without its downsides as it uses more memory since each element is a separate object and Python’s tuple holds pointers to them. It is also generally slower on access since it has to follow a pointer to an object. You would notice the cost in a scenario where we were to create small tuples, then the memory use accumulates.

Comparing that to the module, it reads straight from the tuple itself giving it a faster access time. However, it has its own costs as the struct always reserves space for the maximum number of values. So, a tuple that has two elements takes as much memory as a full one. It also has a hard size limit: if there are more parts than allowed, it fails.

**Maps: dictionaries (Python)**

Dictionaries are another one of Python’s core collection date types. It has insertion order built into its design and has better optimization for speed and memory. This is because of the hash table structure it uses that reduces the time to search through linked lists. On the other hand, the module uses buckets and linked lists to simulate the same functionality, but it is more expensive in terms of memory because each entry needs a separate node and pointer to connect to the nodes. It also keeps a separate array to remember the order of the keys, which uses extra memory. You would notice the cost in dictionaries with a large amount of keys.

### Question 2: You wrote the tag check in dt_value_as_int by hand. Some languages don't let you. They make the tagged union a language construct, so the compiler writes the check for you, refuses to compile a read that skips it, and refuses to compile a set of cases that misses one. Rust's enum and match work this way, and so do ML's datatypes and Swift's enumerations with associated values. What does the C version let you do that a compiler enforcing the check wouldn't, and is any of it worth wanting?

In C, we are allowed to do type punning, which reads any union field regardless of current tag. We are also allowed to alter the payload memory without updating its tag so even if it leads to invalid dynamic types, the compiler still runs the code. By extension, it can also omit tag cases in conditional statements without alerting the programmer of an error. For general use, these features would not be worth wanting as it can lead to vulnerabilities in the system.

### Question 3: Your dt_map keeps insertion order separately from the hash buckets, which is memory spent on something no lookup uses. Argue the other side: describe a design that drops it, say what breaks, and say whether you'd ship it.

If we drop the insertion order, we can do so by eliminating order, capacity, and its reallocations. Doing so will end up with dt_map_key_at unable to return the keys in the intended insertion order. The key iteration order would end up becoming arbitrary through hash distribution and linked list traversal order. If this is the case, I'd rather not ship it as dt_map_key_at requires a certain iteration order for a stable output.

### Question 4: Compare access after release with an allocation that remains unreleased at the driver's final check. What damage can each cause in a long-running server? How does that answer change for a command-line tool that exits in a second?

Access after release (also called a dangling reference) is when a program tries to use memory that it longer owns after being freed. For long-running servers, this can be a problem where the memory freed is a dependecy for some areas. For example, the memeory is reused for another object. For command-line tools, the damage is not as severe compared to that of servers. However, it can still do damage in the form of craches or wrong outputs. As for unreleased allocation, it is when a program still owns a memory but never uses or frees it. In command-line tools, there is relatively small damage since it does not affect the program's run itself and nothing accumulates due to it being short lived. But for long-running servers, the unfreed memory could build up overtime, take up memory space, and eventually slow down the entire server.
