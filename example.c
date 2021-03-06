/*
 * Taken from simple_parser.c example in jansson repository.  Modified to
 * show parsing, finding a configuration value, modifying it, and writing it
 * back out, using MBS json as an example.
 *
 * Simple example of parsing and printing JSON using jansson.
 *
 * SYNOPSIS:
 * $ examples/simple_parse
 * Type some JSON > [true, false, null, 1, 0.0, -0.0, "", {"name": "barney"}]
 * JSON Array of 8 elements:
 *   JSON True
 *   JSON False
 *   JSON Null
 *   JSON Integer: "1"
 *   JSON Real: 0.000000
 *   JSON Real: -0.000000
 *   JSON String: ""
 *   JSON Object of 1 pair:
 *     JSON Key: "name"
 *     JSON String: "barney"
 *
 * Copyright (c) 2014 Robert Poor <rdpoor@gmail.com>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* forward refs */
void print_json(json_t *root);
void print_json_aux(json_t *element, int indent);
void print_json_indent(int indent);
const char *json_plural(int count);
void print_json_object(json_t *element, int indent);
void print_json_array(json_t *element, int indent);
void print_json_string(json_t *element, int indent);
void print_json_integer(json_t *element, int indent);
void print_json_real(json_t *element, int indent);
void print_json_true(json_t *element, int indent);
void print_json_false(json_t *element, int indent);
void print_json_null(json_t *element, int indent);

void print_json(json_t *root) { print_json_aux(root, 0); }

void print_json_aux(json_t *element, int indent) {
    switch (json_typeof(element)) {
        case JSON_OBJECT:
            print_json_object(element, indent);
            break;
        case JSON_ARRAY:
            print_json_array(element, indent);
            break;
        case JSON_STRING:
            print_json_string(element, indent);
            break;
        case JSON_INTEGER:
            print_json_integer(element, indent);
            break;
        case JSON_REAL:
            print_json_real(element, indent);
            break;
        case JSON_TRUE:
            print_json_true(element, indent);
            break;
        case JSON_FALSE:
            print_json_false(element, indent);
            break;
        case JSON_NULL:
            print_json_null(element, indent);
            break;
        default:
            fprintf(stderr, "unrecognized JSON type %d\n", json_typeof(element));
    }
}

void print_json_indent(int indent) {
    int i;
    for (i = 0; i < indent; i++) {
        putchar(' ');
    }
}

const char *json_plural(int count) { return count == 1 ? "" : "s"; }

void print_json_object(json_t *element, int indent) {
    size_t size;
    const char *key;
    json_t *value;

    print_json_indent(indent);
    size = json_object_size(element);

    printf("JSON Object of %ld pair%s:\n", size, json_plural(size));
    json_object_foreach(element, key, value) {
        print_json_indent(indent + 2);
        printf("JSON Key: \"%s\"\n", key);
        print_json_aux(value, indent + 2);
    }
}

void print_json_array(json_t *element, int indent) {
    size_t i;
    size_t size = json_array_size(element);
    print_json_indent(indent);

    printf("JSON Array of %ld element%s:\n", size, json_plural(size));
    for (i = 0; i < size; i++) {
        print_json_aux(json_array_get(element, i), indent + 2);
    }
}

void print_json_string(json_t *element, int indent) {
    print_json_indent(indent);
    printf("JSON String: \"%s\"\n", json_string_value(element));
}

void print_json_integer(json_t *element, int indent) {
    print_json_indent(indent);
    printf("JSON Integer: \"%" JSON_INTEGER_FORMAT "\"\n", json_integer_value(element));
}

void print_json_real(json_t *element, int indent) {
    print_json_indent(indent);
    printf("JSON Real: %f\n", json_real_value(element));
}

void print_json_true(json_t *element, int indent) {
    (void)element;
    print_json_indent(indent);
    printf("JSON True\n");
}

void print_json_false(json_t *element, int indent) {
    (void)element;
    print_json_indent(indent);
    printf("JSON False\n");
}

void print_json_null(json_t *element, int indent) {
    (void)element;
    print_json_indent(indent);
    printf("JSON Null\n");
}

/*
 * Parse text into a JSON object. If text is valid JSON, returns a
 * json_t structure, otherwise prints and error and returns null.
 */
json_t *load_json(const char *text) {
    json_t *root;
    json_error_t error;

    root = json_loads(text, 0, &error);

    if (root) {
        return root;
    } else {
        fprintf(stderr, "json error on line %d: %s\n", error.line, error.text);
        return (json_t *)0;
    }
}

/*
 * Print a prompt and return (by reference) a null-terminated line of
 * text.  Returns NULL on eof or some error.
 */
char *read_line(char *line, int max_chars) {
    printf("Type some JSON > ");
    fflush(stdout);
    return fgets(line, max_chars, stdin);
}

/* ================================================================
 * main
 */

#define MAX_CHARS 4096

int main(int argc, char *argv[]) {
    char line[MAX_CHARS];
    json_error_t err;
    const char *key;
    const char *hg_key;
    json_t *value;
    json_t *hg_value;
    int found = 0;
    char output_template[256];
    int fd;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <json_file>\n", argv[0]);
        exit(-1);
    }

    /* parse text into JSON structure */
    json_t *root = json_load_file(argv[1], JSON_REJECT_DUPLICATES, &err);
    if(!root)
    {
        fprintf(stderr, "Error loading file\n");
        exit(-1);
    }

    /* print */
    printf("DUMPING JSON CONTENTS:\n");
    printf("======================\n");
    print_json(root);

    printf("MANIPULATING FIELDS:\n");
    printf("=========================\n");
    json_object_foreach(root, key, value) {
        if(strcmp(key, "mercury") == 0)
        {
            json_object_foreach(value, hg_key, hg_value)
            {
                if(strcmp(hg_key, "protocol") == 0)
                {
                    found = 1;
                    printf("found ROOT/mercury/protocol set to \"%s\".\n",
                        json_string_value(hg_value));
                    printf("changing ROOT/mercury/protocol to \"verbs\".\n");
                    json_object_set(value, "protocol", json_string("verbs"));
                }
            }
        }
    }
    if(!found) fprintf(stderr, "WARNING: could not find protocol.\n");

    printf("ENCODING:\n");
    printf("=========================\n");

    sprintf(output_template, "/tmp/json.XXXXXX");
    fd = mkstemp(output_template);
    close(fd);
    json_dump_file(root, output_template, JSON_INDENT(4));
    printf("wrote modified json to %s\n", output_template);

    /* release the json structure */
    json_decref(root);

    return 0;
}
