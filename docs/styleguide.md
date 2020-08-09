# Code style guide

_This documentaion is under development._

- No multi level ternary operations e.g:

    * __Bad__:
        ```c++
        auto isValid = isAValid() ? true : isBValid() ? true : false
        ```
    * __Good__:
        ```c++
        bool isValid;
        if (isAValid()) 
        {
            isValid = true;
        } 
        else if (isBValid()) 
        {
            isValid = true;
        } 
        else 
        {
            isValid = false;
        }
        ```

- Everything new (`ETJump` related) should be in a namespace. Use `ETJump::` if more appropriate is not available. (e.g. `DateTime::` for date utilities):

    ```c++
    namespace ETJump 
    {
        class Example 
        {

        };
    };
    ```

- Functions, class methods and variables should be __camelCased__:

    ```c++
    void aCamelCasedName() 
    {

    }
    ```

- `Class` names and `enums` should be __PascalCased__:

    ```c++
    namespace ETJump 
    {
        enum FileOpenModes
        {

        };
        class FileSystem
        {

        };
    };
    ```
