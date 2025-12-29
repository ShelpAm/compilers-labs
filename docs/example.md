# Example

Example of hlvm language.

```hlvm
var x: int = 10; // ": int" is optional type annotation

var ptr: int [] * * const; // Read from left to right: a const pointer to a
                           // pointer to an array of integers

func add(a: int, b: int): int {
    return a + b;
}
```
