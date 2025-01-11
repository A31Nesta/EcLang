# Todo List

- Add a **Token** struct containing:
    - Original word (string)
    - Type (enum)
        - Class (Depends on language)
        - Attribute (Depends on language)
        - Enter Scope (`{`)
        - Exit Scope (`}`)
        - Assignment (`=`)
        - Number
        - String (can contain spaces)
        - StringMD (can contain spaces)
        - Vector (`/vec[234][ilfd]/`)
    - Vector number (for vectors only: is it vec2? 3? 4?)
    - Vector type (enum. for vectors only: is it float? int?...)
    - Line (line of code that this token is in. For info/errors)
    - Column (column in the code where this token is. For info/errors)

- Implement **Lexer**. (takes source file and tokenizes it)

- Implement **Parser**. (takes tokens and constructs the objects while checking that the grammar is correct)