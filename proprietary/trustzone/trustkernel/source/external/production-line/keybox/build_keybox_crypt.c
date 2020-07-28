#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

/* It's OK we directly include array.c,
   because we want to use sizeof() to
   directly get length of Ekkb_pub and
   InputPkb. */
#include <array.c>

static const char *TAG = "build_keybox_crypt";
static const char *keybox_magic = "KEYCRYPT";

static uint32_t keybox_crypt_version = 0x1;

int main(int argc, char *argv[])
{
    int fd, r;

    uint32_t ekkb_pub_size, input_pkb_size;

    if (argc < 2) {
        fprintf(stderr, "%s: Invalid arguments. USAGE: %s <output>\n",
            TAG, argv[0]);
        return EINVAL;
    }

    if ((fd = open(argv[1], O_CREAT | O_TRUNC | O_WRONLY,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0) {
        fprintf(stderr, "%s: failed to open %s with %d\n",
            TAG, argv[1], errno);
        return errno;
    }

    fprintf(stdout, "%s: output path %s\n", TAG, argv[1]);

    ekkb_pub_size = sizeof(Ekkb_pub);
    input_pkb_size = sizeof(InputPkb);

    fprintf(stdout, "%s: ekkb_pub_size: %u\n", TAG, ekkb_pub_size);
    fprintf(stdout, "%s: input_pkb_size: %u\n", TAG, input_pkb_size);

    r = write(fd, keybox_magic, strlen(keybox_magic));
    if (r != (int) strlen(keybox_magic)) {
        fprintf(stderr, "%s: failed to write magic with %zd\n", TAG, r);
        r = EIO;
        goto exit;
    }

	r = write(fd, &keybox_crypt_version, sizeof(keybox_crypt_version));
    if (r != (int) sizeof(keybox_crypt_version)) {
        fprintf(stderr, "%s: failed to write version with %zd\n", TAG, r);
        r = EIO;
        goto exit;
    }

    r = write(fd, &ekkb_pub_size, sizeof(ekkb_pub_size));
    if (r != (ssize_t) sizeof(ekkb_pub_size)) {
        fprintf(stderr, "%s: failed to write ekkb_pub_size with %zd\n", TAG, r);
        r = EIO;
        goto exit;
    }

    r = write(fd, &input_pkb_size, sizeof(input_pkb_size));
    if (r != (int) sizeof(input_pkb_size)) {
        fprintf(stderr, "%s: failed to write input_pkb_size with %zd\n", TAG, r);
        r = EIO;
        goto exit;
    }

    r = write(fd, Ekkb_pub, ekkb_pub_size);
    if (r != (int) ekkb_pub_size) {
        fprintf(stderr, "%s: failed to write Ekkb_pub with %zd\n", TAG, r);
        r = EIO;
        goto exit;
    }

    r = write(fd, InputPkb, input_pkb_size);
    if (r != (int) input_pkb_size) {
        fprintf(stderr, "%s: failed to write InputPkb with %zd\n", TAG, r);
        r = EIO;
        goto exit;
    }

    r = 0;

    fprintf(stdout, "%s: DONE\n", TAG);

exit:
    close(fd);
    return r;
}
