
#if GOOGLE_HOTWORD_DUMMY_LIB
int GoogleHotwordDspInit(void* hotword_memmap)
{
    return 1;
}

int GoogleHotwordDspProcess(const void* samples, int num_samples,
                            int* preamble_length_ms)
{
    return 0;
}

void GoogleHotwordDspReset(void)
{
    return;
}

#endif  // GOOGLE_HOTWORD_DUMMY_LIB