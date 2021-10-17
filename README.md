# RigC programming language

A prototype of **RigC** programming language - the better and simpler C++ language.

<small>I know that this probably will never happen. Anyway...</small>


## Example code

### 👋 Hello, World!

```rust
import std.io print;

func main {
	print("Hello, world!");
}
```

### 🔤 Strings

```rust
import std.io print;
// for toUpper()
import std.string.transform_impl;

func main {
	const	s1 = "This is a C-string equivalent (null-terminated)";
	Char[]	s2 = "This is a array of characters (not null-terminated)";
	Char[]	s3 = "Hello";

	// To access s2 size:
	print("s2 has {} characters", s2.len());

	// Replace each character with other:
	s3.transform(c => c.toUpper());

}
```

// TBD