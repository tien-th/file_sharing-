#include <stdio.h>
#include <string.h>

const char *extract_filename(const char *path) {
    const char *last_slash = strrchr(path, '/');  // Find the last occurrence of '/'
    
    if (last_slash != NULL) {
        return last_slash + 1;  // Return the part after the last '/'
    } else {
        return path;  // No '/' found, return the original path
    }
}

int main() {
    // Example usage
    char filename1[] = "server5.c";
    char filename2[] = "./server5.c";
    char filename3[] = "/mnt/disk3/tien_aiot/file_sharing-/client/1234/server5.c";

    // Extract filenames
    const char *result1 = extract_filename(filename1);
    const char *result2 = extract_filename(filename2);
    const char *result3 = extract_filename(filename3);

    // Print results
    printf("Result 1: %s\n", result1);
    printf("Result 2: %s\n", result2);
    printf("Result 3: %s\n", result3);

    return 0;
}
