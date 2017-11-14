Only listing things here for now, need to clean up:

- No multi level ternary operations e.g.
```C++
auto isValid 
  = isAValid() ? true : isBValid() ? true : false
```
- Classes should be in a namespace. Use ETJump:: if more appropriate is not available. (e.g. DateTime:: for date utilities)
- Functions, class methods and variables should be camelCased
- Class names and enums should be PascalCased
