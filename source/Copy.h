inline void CopyFile(const char* source, const char* destination) {
    FILE *src = fopen(source, "rb");
    if (src == NULL) {
        printf("Error opening source file %s\n", source);
        return;
    }
    FILE *dest = fopen(destination, "wb");
    if (dest == NULL) {
        printf("Error opening destination file %s\n", destination);
        fclose(src);
        return;
    }

    char buffer[1024];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dest) != bytes_read) {
            printf("Error writing to destination file\n");
            fclose(src);
            fclose(dest);
            return;
        }
    }
    
    if (ferror(src)) {
        printf("Error reading source file\n");
    }
    
    fclose(src);
    fclose(dest);

    printf("Successfully copied file.\n");
}