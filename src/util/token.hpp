#pragma once

namespace eclang::token {
    namespace type {
        /**
            The type of token. This makes it easy
            for the parser to easily determine if the
            grammar is correct.
        */
        enum Type {
            // Single character
            SCOPE_ENTER, // {
            SCOPE_EXIT, // }
            ASSIGNMENT, // =
            PARENTHESIS_OPEN, // (
            PARENTHESIS_CLOSE, // )
            COMMA, // ,
            SEMICOLON, // ;

            KEYWORD, // Identified by Hash (#) (/#[a-ZA-Z_-]+/)
            CLASS, // Dependent on the language (/[a-zA-Z_][a-zA-Z_0-9]*/)
            ATTRIBUTE, // Dependent on the language (/[a-zA-Z_][a-zA-Z_0-9]*/)

            NUMBER, // Number, can be integer or decimal, signed or unsigned. (/[0-9]+\.{,1}[0-9]*/)
            STRING, // Identified by quotes (") (/"(\\.|[^\\"])*"/)
            STRING_MD, // Identified by triple backticks (```) (/```(\\.|[^\\`])*```/)
            VECTOR // Identified by /vec[234][ilfd]/
        };
    }

    /**
        When reading a file, the file is broken down into tokens,
        which are composed of the original string (usually a word or symbol;
        in the case of Strings and Markdown strings this is the whole string)
        and its type
    */
    struct Token {

    };
}