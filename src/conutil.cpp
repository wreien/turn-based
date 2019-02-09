#include "conutil.h"

#ifdef WIN32
#include <iostream>
#include <Windows.h>
#endif // WIN32

bool enableVtEscapeCodes() {
#ifdef WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output == INVALID_HANDLE_VALUE) {
        std::cerr << "Couldn't retrieve output handle!\n";
        return false;
    }

    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    if (input == INVALID_HANDLE_VALUE) {
        std::cerr << "Couldn't retrieve input handle!\n";
        return false;
    }

    DWORD originalOutMode = 0;
    DWORD originalInMode = 0;
    if (!GetConsoleMode(output, &originalOutMode)) {
        std::cerr << "Couldn't retrieve output mode!\n";
        return false;
    }
    if (!GetConsoleMode(input, &originalInMode)) {
        std::cerr << "Couldn't retrieve input mode!\n";
        return 1;
    }

    DWORD requestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING
        | DISABLE_NEWLINE_AUTO_RETURN;
    DWORD requestedInModes = ENABLE_VIRTUAL_TERMINAL_INPUT;

    DWORD outMode = originalOutMode | requestedOutModes;
    DWORD inMode = originalInMode | requestedInModes;

    if (!SetConsoleMode(output, outMode)) {
        requestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        outMode = originalOutMode | requestedOutModes;
        if (!SetConsoleMode(output, outMode)) {
            std::cerr << "Unable to set VT mode to the output!\n";
            return false;
        }
    }

    if (!SetConsoleMode(input, inMode)) {
        std::cerr << "Unable to set VT mode to the input!\n";
        return false;
    }
#endif // WIN32

    return true;
}