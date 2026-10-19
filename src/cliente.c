#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"

Opcoes parse_args(int argc, char *argv[]) {
    Opcoes opts = {0}; 

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--reader") == 0 && i + 1 < argc) {
            strcpy(opts.reader, argv[++i]);
        } else if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            strcpy(opts.input, argv[++i]);
        } else if (strcmp(argv[i], "--base") == 0 && i + 1 < argc) {
            strcpy(opts.base, argv[++i]);
        } else if (strcmp(argv[i], "--result") == 0 && i + 1 < argc) {
            strcpy(opts.result, argv[++i]);
        } else if (strcmp(argv[i], "--query") == 0 && i + 1 < argc) {
            strcpy(opts.query, argv[++i]);
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            strcpy(opts.output, argv[++i]);
        } else if (strcmp(argv[i], "--top-k") == 0 && i + 1 < argc) {
            opts.top_k = atoi(argv[++i]);
        }
    }

    return opts;
}
