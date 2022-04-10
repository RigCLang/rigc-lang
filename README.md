# RigC programming language

A prototype of **RigC** programming language - the better and simpler C++ language.

<small>I know that this probably will never happen. Anyway...</small>


## Examples

### 👋 Hello, World!

```rust
import std.io print;

func main {
	print("Hello, world!\n");
}
```

### Variables

```rust
func main {
	var x = 1; // deduced Int32
	var y = 2.f; // deduced Float32

	var z = func(x, y);
	var w = MyClass(); // w is instance of MyClass
}
```

### Control flow

#### if and else

```rust
func main {
	var x = 1;
	if (x < 10)
		print("x < 10\n");
	else
		print("x >= 10\n");

	if (x < 10 or x > 20)
		print("x < 10 or x > 20\n");
}
```

#### Loops

```rust
func main {
	var i = 0;
	while (i < 10) {
		print("i = {}\n", i);
		i = i + 1;
	}
}
```

### Classes

```rust
class Vector2
{
	x: Float32;
	y: Float32;

	construct {
		x = 0.f;
		y = 0.f;
	}

	construct(x: Float32, y: Float32) {
		self.x = x;
		self.y = y;
	}

	length {
		ret sqrt(x*x + y*y);
	}

	normalized {
		var copy = self;
		var len = copy.length();
		copy.x /= len;
		copy.y /= len;
		ret copy;
	}

	plus(x: Float32, y: Float32) {
		ret Vector2(self.x + x, self.y + y);
	}
}

func main {
	var vec = Vector2(1.f, 2.f);
}
```

#### Extension methods

First parameter must be:
- of name `self` (keyword)
- a reference type

```rust
template <T: IndexedRange>
func indexOf(self: Ref<T>, elem: Char)
{
	var i = 0;
	while (i < self.size())
	{
		if (self[i] == elem)
			ret i;
		i += 1;
	}
	ret -1;
}

// usage:
func main {
	print("Found 'b' at index {} in text \"abc\"\n", "abc".indexOf('b'));
}
```

// TBD
