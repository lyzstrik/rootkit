#include <fcntl.h> // open
#include <unistd.h> // read, close
#include <stdio.h> // printf
#include <string.h> // strlen

int main() {
    char buffer[128];
    int fd;

    // Ouvrir un fichier en lecture
    fd = open("testfile.txt", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    printf("File opened with fd: %d\n", fd);

    // Lire des données
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("read");
        close(fd);
        return 1;
    }
    buffer[bytes_read] = '\0';
    printf("Read %ld bytes: %s\n", bytes_read, buffer);

    // Fermer le fichier
    if (close(fd) == -1) {
        perror("close");
        return 1;
    }
    printf("File closed.\n");

    return 0;
}
