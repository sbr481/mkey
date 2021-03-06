/*
 * mkey - parental controls master key generator for certain video game consoles
 * Copyright (C) 2015-2016, Daz Jones (Dazzozo) <daz@dazzozo.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mkey.h"
#include "types.h"

#define MAIN_ARGS_REQUIRED (4)

const char* main_format_devices(void);
void main_set_data_path(mkey_ctx* ctx);

int main(int argc, const char* argv[])
{
    mkey_ctx ctx;
    mkey_init(&ctx, false, NULL);

    main_set_data_path(&ctx);

    const char* default_device = "CTR";
    mkey_set_default_device(&ctx, default_device);

    if(argc < MAIN_ARGS_REQUIRED) {

        printf("Usage: mkey <inquiry> <month> <day> [device] [-v]\n");
        printf("mkey (c) 2015-2016, SALT\n\n");

        printf("inquiry          8 or 10 digit inquiry number\n");
        printf("month            month displayed on device (system time)\n");
        printf("day              day displayed on device (system time)\n\n");

        const char* devices = main_format_devices();
        printf("device           device type: %s - %s by default\n", devices, default_device);
        printf("-v, --verbose    enable debugging output\n");

        free((void*)devices);
        exit(EXIT_FAILURE);
    }

    if(!isnumeric(argv[1]) || !isnumeric(argv[2]) || !isnumeric(argv[3])) {
        printf("Error: input values must be numbers.\n");
        exit(EXIT_FAILURE);
    }

    const char* inquiry = argv[1];
    if(strlen(inquiry) != 10 && strlen(inquiry) != 8) {
        printf("Error: inquiry number must be 8 or 10 digits.\n");
        exit(EXIT_FAILURE);
    }

    u8 month = strtoul(argv[2], 0, 10);
    u8 day = strtoul(argv[3], 0, 10);

    // Optional parameters.
    argc -= MAIN_ARGS_REQUIRED;
    argv += MAIN_ARGS_REQUIRED;

    const char* device = NULL;
    bool debug = false;

    for(int i = 0; i < argc; i++) {
        if(argv[i][0] != '-') device = argv[i];
        if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) debug = true;
    }

    if(debug) mkey_set_debug(&ctx, true);

    int result = mkey_generate(&ctx, inquiry, month, day, device);
    if(result < 0) {
        if(!debug) {
            printf("An error occurred. Your input values may be incorrect, or you may be missing a key file.\n");
            printf("Use --verbose for more details.\n");
        }

        exit(EXIT_FAILURE);
    }

    printf("Master key is %05d.\n", result);
    return 0;
}

const char* main_format_devices(void)
{
    size_t size = (mkey_num_devices() * (sizeof(*mkey_devices) + 2)) + 1;
    char* buffer = malloc(size);
    memset(buffer, 0, size);

    strcat(buffer, "{");

    for(int i = 0; i < mkey_num_devices(); i++) {
        strcat(buffer, mkey_devices[i]);
        if(i != mkey_num_devices() - 1) strcat(buffer, ", ");
    }

    strcat(buffer, "}");
    return buffer;
}

#if __linux__
#include <unistd.h>
#elif _WIN32
#include <Windows.h>
#endif

// :-(
void main_set_data_path(mkey_ctx* ctx)
{
    char path[MAX_PATH];
    const char* end = NULL;

#if __linux__
    if(readlink("/proc/self/exe", path, sizeof(path)) <= 0) return;
    end = strrchr(path, '/');
#elif _WIN32
    if(GetModuleFileName(NULL, path, sizeof(path)) == 0) return;
    end = strrchr(path, '\\');
#endif
    if(!end) return;

    int length = strlen(path) - strlen(end);
    sprintf(path, "%.*s/data", length, path);

    mkey_set_data_path(ctx, path);
}
