#include <stdio.h>

int main() {
    char buffer[20];
    int value = 42;

    // Format the string and store it in the buffer
    int len = snprintf(buffer, sizeof(buffer), "The answer is %d", value);

    if (len >= sizeof(buffer)) {
        printf("Output was truncated.\n");
    } else {
        // Print the formatted string
        printf("Formatted string: %s\n", buffer);
    }

    return 0;
}
