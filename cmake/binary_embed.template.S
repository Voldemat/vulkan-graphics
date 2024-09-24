#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B)
#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)
#define VAR_PREFIX
#if defined(__APPLE__)
.section __DATA, __const
#define VAR_PREFIX _
#endif
#if defined(__linux__) && defined(__ELF__)
#if defined(__x86_64__) || defined(_M_X64)
.section .note.GNU-stack,"",@progbits
#else
.section .note.GNU-stack,"r",%progbits
#endif
.rodata
#endif
.global STRINGIZE(PPCAT(VAR_PREFIX, data_start_${INPUT_FILE_BASE}))
.global STRINGIZE(PPCAT(VAR_PREFIX, data_end_${INPUT_FILE_BASE}))
STRINGIZE(PPCAT(VAR_PREFIX, data_start_${INPUT_FILE_BASE})):
.incbin "${INPUT_FILE_NAME}"
STRINGIZE(PPCAT(VAR_PREFIX, data_end_${INPUT_FILE_BASE})):
