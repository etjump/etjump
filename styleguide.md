Only listing things here for now, need to clean up:

- No multi level ternary operations e.g.
### Bad
```C++
auto isValid 
  = isAValid() ? true : isBValid() ? true : false
```
### Good
```C++
bool isValid;
if (isAValid()) {
  isValid = true;
} else if (isBValid()) {
  isValid = true;
} else {
  isValid = false;
}
```


- Classes should be in a namespace. Use ETJump:: if more appropriate is not available. (e.g. DateTime:: for date utilities)
```C++
namespace ETJump {
  class Example {
  
  }
}
```
- Functions, class methods and variables should be camelCased
```C++
void aCamelCasedName() {

}
```
- Class names and enums should be PascalCased
